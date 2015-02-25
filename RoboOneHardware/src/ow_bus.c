/*
 * OneWire bus handling thread for RoboOne.
 * This file encapsulates all the knowledge of
 * what is conected to what on the OneWire bus
 * so that nothing outside here should need to
 * know the contents of hw_config.h or the bus
 * state.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <rob_system.h>
#include <one_wire.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <hardware_types.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <ow_bus.h>
#include <hw_config.h>
#include <orangutan.h>

/*
 * MANIFEST CONSTANTS
 */

#define ONEWIRE_PORT_STRING    "/dev/USBSerial"
#define MAX_NUM_DEVICES         8  /* This MUST be the same as the number of elements in the gDeviceStaticConfigList[] below */
#define MAX_NUM_BATTERY_DEVICES 4
#define TOGGLE_DELAY_US         500000L   /* How long to toggle a set of pins from current state to opposite and back again */
#define SERIAL_NUM_BUFFER_SIZE (NUM_BYTES_IN_SERIAL_NUM * 2) + 3 /* string representation of serial num with 0x in front and terminator on the end */

/* The maximum number of devices that oneWireFindAllDevices() can report */
#define MAX_DEVICES_TO_FIND         10

/* Enable current measurement, integrated current accummulator, charge/discharge
 * counting and shadowing of charge/discharge count to non-volatile storage */
#define DEFAULT_DS2438_CONFIG         (DS2438_IAD_IS_ENABLED | DS2438_CA_IS_ENABLED | DS2438_EE_IS_ENABLED)
#define RIO_BATTERY_MONITOR_CONFIG    DEFAULT_DS2438_CONFIG
#define O1_BATTERY_MONITOR_CONFIG     DEFAULT_DS2438_CONFIG
#define O2_BATTERY_MONITOR_CONFIG     DEFAULT_DS2438_CONFIG
#define O3_BATTERY_MONITOR_CONFIG     DEFAULT_DS2438_CONFIG

/* Set the threshold at which current measurements get added to the
 * accumulators in a DS2438. Use 0x40 (+/-2 LSB) as there is ~20 ma
 * of noise */
#define DEFAULT_DS2438_THRESHOLD      0x40

/* RSTZ is a reset line, set power on reset back to 0,
 * don't care about conditional search as we don't use it */
#define DEFAULT_DS2408_CONFIG         (~DS2408_DEVICE_HAS_POWER_ON_RESET & ~DS2408_RSTZ_IS_STROBE)
#define CHARGER_STATE_IO_CONFIG       DEFAULT_DS2408_CONFIG
#define DARLINGTON_IO_CONFIG          DEFAULT_DS2408_CONFIG
#define RELAY_IO_CONFIG               DEFAULT_DS2408_CONFIG
#define GENERAL_PURPOSE_IO_CONFIG     DEFAULT_DS2408_CONFIG

/* Which pins are inputs (1 for an input, 0 for an output) */
#ifdef ALL_PINS_UNDRIVEN_DS2408
#    define CHARGER_STATE_IO_PIN_INPUTS   0xFF
#    define DARLINGTON_IO_PIN_INPUTS      0xFF
#    define RELAY_IO_PIN_INPUTS           0xFF
#    define GENERAL_PURPOSE_IO_PIN_INPUTS 0xFF
#else
#    define CHARGER_STATE_IO_PIN_INPUTS   0xFF
#    define DARLINGTON_IO_PIN_INPUTS      0x00
#    define RELAY_IO_PIN_INPUTS           RELAY_12V_DETECT
#    define GENERAL_PURPOSE_IO_PIN_INPUTS 0x00  /* Default to outputs so as to drive the analogue mux */
#endif

/* Pins generally low to begin with apart from charger state which is allowed to float as an input,
 * and the Darlington pins which are disabled anyway by setting DARLINGTON_ENABLE_BAR high,
 * the 12V detect pin on the Relay PIO set as an input and RELAY_ENABLE_BAR high.
 * Note: it doesn't actually matter what input pins are set to but 1 is used for consistency. */
#ifdef ALL_PINS_UNDRIVEN_DS2408
#    define CHARGER_STATE_IO_PIN_INITIAL_STATE   0xFF
#    define DARLINGTON_IO_PIN_INITIAL_STATE      0xFF
#    define RELAY_IO_PIN_INITIAL_STATE           0xFF
#    define GENERAL_PURPOSE_IO_PIN_INITIAL_STATE 0xFF
#else
#    define CHARGER_STATE_IO_PIN_INITIAL_STATE   0xFF
#    define DARLINGTON_IO_PIN_INITIAL_STATE      (UInt8) (~(DARLINGTON_RIO_PWR_BATT_OFF | DARLINGTON_RIO_PWR_12V_ON | DARLINGTON_O_PWR_TOGGLE | DARLINGTON_O_RESET_TOGGLE) | DARLINGTON_ENABLE_BAR)
#    define RELAY_IO_PIN_INITIAL_STATE           RELAY_12V_DETECT | RELAY_ENABLE_BAR
#    define GENERAL_PURPOSE_IO_PIN_INITIAL_STATE 0x00
#endif

/* Which pin positions should have their state tracked through
 * the locally stored pinsState rather than by just reading
 * back the pins directly */
#define CHARGER_STATE_IO_SHADOW_MASK    0x00
#define DARLINGTON_IO_SHADOW_MASK       (DARLINGTON_O_PWR_TOGGLE | DARLINGTON_O_RESET_TOGGLE)
#define RELAY_IO_SHADOW_MASK            (RELAY_O_PWR_12V_ON | RELAY_O_PWR_BATT_OFF | RELAY_RIO_CHARGER_OFF | RELAY_O1_CHARGER_OFF | RELAY_O2_CHARGER_OFF | RELAY_O3_CHARGER_OFF)
#define GENERAL_PURPOSE_IO_SHADOW_MASK  0x00

/*
 * TYPES
 */

/* The names of all the one wire devices on RoboOne
 * ORDER IS IMPORTANT - this enum is used to index into gDeviceStaticConfigList[] */
typedef enum OwDeviceNameTag
{
    OW_NAME_RIO_BATTERY_MONITOR = 0,
    OW_NAME_O1_BATTERY_MONITOR = 1,
    OW_NAME_O2_BATTERY_MONITOR = 2,
    OW_NAME_O3_BATTERY_MONITOR = 3,
    OW_NAME_CHARGER_STATE_PIO = 4,
    OW_NAME_DARLINGTON_PIO = 5,
    OW_NAME_RELAY_PIO = 6,
    OW_NAME_GENERAL_PURPOSE_PIO = 7,
    OW_NUM_NAMES,
    OW_NAME_NULL
} OwDeviceName;

/* The possible OneWire device types */
typedef enum OwDeviceTypeTag
{
    OW_TYPE_DS2408_PIO,
    OW_TYPE_DS2438_BATTERY_MONITOR,
    OW_NUM_TYPES,
    OW_TYPE_UNKNOWN
} OwDeviceType;

/* Device specific information for the DS2408 PIO OneWire device */
typedef struct OwDS2408Tag
{
    UInt8 config;
    UInt8 readShadowMask; /* If a bit is set to 1 then for that pin the
                             corresponding bit position in pinsState
                             represents the current state of the pin, if set
                             a bit is set to 0 then the port can be read
                             to determine that bit's current state.  This is
                             useful if the thing connected to the IO port
                             switches at a relatively low voltage (e.g. if it
                             is 3.3V/5V compatible) so the read-back would
                             think that the device is always off even when it
                             is actually switched on. */
    UInt8 inputMask; /* If a bit is set to 1 then that pin is an input
                        pin and so will always be written to as a 1
                        to keep it that way. */
    UInt8 pinsState; /* A 1 at a bit position means the transistor floats and 
                        so is either 1 or an input, a 0 at a bit position 
                        means the transistor is switched on, so the pin is
                        dragged to ground and this is definitely an output. */ 
} OwDS2408;

/* Device specific information for the DS2438 battery monitoring OneWire device */
typedef struct OwDS2438Tag
{
    UInt8 config;
} OwDS2438;

/* Union of all the possible OneWire device specific information */
typedef union OwDeviceSpecificsTag
{
    OwDS2408 ds2408;
    OwDS2438 ds2438;
} OwDeviceSpecifics;

/* A type to hold a device address on the OneWire bus */
typedef struct OwDeviceAddressTag
{
    UInt8  value[NUM_BYTES_IN_SERIAL_NUM];
} OwDeviceAddress;

/* The devices that must exist, their names, their addresses and their configurations */
typedef struct OwDevicesStaticConfigTag
{
    OwDeviceName      name;
    OwDeviceAddress   address;
    OwDeviceSpecifics specifics;
} OwDevicesStaticConfig;

/*
 * GLOBALS (prefixed with g)
 */

/* Determine which device is which with the following procedure:
 *
 * 0. Connect the USB Serial port to the RoboOne PCB but none of the relays,
 *    reset lines, monitoring circuits, mux, etc.  Connect a battery only to
 *    the Pi/RIO.  Do not connect any 12 Volt supply.
 * 1. Run the program "tstfind", that is in the OneWire libs, on the Pi, giving it
 *    /dev/USBSerial as its target serial port.  It will list all the device
 *    addresses, in reverse byte order (i.e. with the family ID on the left).
 *    There should be eight devices, four with family ID 0x26 (FAMILY_SBATTERY) and
 *    four with family ID 0x29 (FAMILY_PIO).  Copy the addresses, reversing byte
 *    order, into the structure below, putting them in the right families but
 *    otherwise randomly assigning the addresses to the entries.
 * 2. Make sure that setDebugPrintsOn() is being called in main.c, make sure
 *    that any things that write to DS2408 pins (e.g. 'Analogue' in gWindowList[])
 *    are disabled and build this program with ALL_PINS_UNDRIVEN_DS2408 defined.
 *    Download it to the Pi and run it.  Everything should run and stay running,
 *    though your screen will likely be full of debug.  If things stop running
 *    you've forgtten to disable something that accesses the DS2408s.
 * 3. Disable setDebugPrintsOn() and build/load/run this program again.
 *    Short out each of the 0.05 ohm measurement resistors and use the 'Q'
 *    calibration function to run a calibration.
 * 4. Look at the dashboard readings for volts.  The one that is saying anything
 *    at all is the RIO battery monitor. Swap the address of that device in the
 *    table below with that of the RIO entry (making sure you change the right
 *    conditionally compiled section).  Rebuild and re-download this program to
 *    the Pi and verify that RIO now shows volts.
 * 5. Take another battery and plug it in as one of the Orangutan batteries,
 *    iterate step 4 and repeat until you've sorted all the battery monitors,
 *    making sure to re-build and re-download at each iteration.
 * 6. Plug in the charger monitoring cables and then plug in a 12 Volt supply.
 *    This should cause one or more of the charging LEDs to light up.  Check
 *    on the dashboard display to see if the corresponding charger indication
 *    comes up.  If it doesn't, swap the PIO addresses below until it does.
 *    In this way you get the OW_NAME_CHARGER_STATE_PIO entry sorted.
 *    IMPORTANT: disconnect 12 Volt power once more, it is dangerous to leave
 *    if connected when we don't know where it's going.
 * 7. Repeat 6 but this time look at the 12V present indicator on the dashboard.
 *    If this matches the real world then the OW_NAME_RELAY_PIO entry is sorted,
 *    if not swap PIO entries (aside from OW_NAME_CHARGER_STATE_PIO) until it
 *    does.
 * 8. Connect one of the General Purpose IO pins to ground.  Use the 'G' menu
 *    option to read the GPIOs state and see if it matches the real world (the
 *    one you have shorted to ground should be OFF, the rest ON).  If it doesn't
 *    swap the addresses in the OW_NAME_GENERAL_PURPOSE_PIO and
 *    OW_NAME_DARLINGTON_PIO entries below.  Retry with this setting.
 * 9. Recompile without ALL_PINS_UNDRIVEN_DS2408 and all should be sorted.
 * 
 */

/* ORDER IS IMPORTANT - the OwDeviceName enum is used to index into the
 * gDeviceStaticConfigList array and the FAMILY_SBATTERY devices must
 * come first */

static OwDevicesStaticConfig gDeviceStaticConfigList[] =
         {{OW_NAME_RIO_BATTERY_MONITOR, {{FAMILY_SBATTERY, 0xcf, 0xe0, 0xa6, 0x01, 0x00, 0x00, 0x0b}}, {{RIO_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O1_BATTERY_MONITOR, {{FAMILY_SBATTERY, 0x69, 0xe0, 0xa6, 0x01, 0x00, 0x00, 0xe5}}, {{O1_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O2_BATTERY_MONITOR, {{FAMILY_SBATTERY, 0x19, 0xe0, 0xa6, 0x01, 0x00, 0x00, 0x7d}}, {{O2_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O3_BATTERY_MONITOR, {{FAMILY_SBATTERY, 0x53, 0x20, 0xb3, 0x01, 0x00, 0x00, 0x5c}}, {{O3_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_CHARGER_STATE_PIO, {{FAMILY_PIO, 0xbc, 0xc1, 0x0e, 0x00, 0x00, 0x00, 0xa3}}, {{CHARGER_STATE_IO_CONFIG, CHARGER_STATE_IO_SHADOW_MASK, CHARGER_STATE_IO_PIN_INPUTS, CHARGER_STATE_IO_PIN_INITIAL_STATE}}},
          {OW_NAME_DARLINGTON_PIO, {{FAMILY_PIO, 0xaf, 0xc1, 0x0e, 0x00, 0x00, 0x00, 0xa1}}, {{DARLINGTON_IO_CONFIG, DARLINGTON_IO_SHADOW_MASK, DARLINGTON_IO_PIN_INPUTS, DARLINGTON_IO_PIN_INITIAL_STATE}}},
          {OW_NAME_RELAY_PIO, {{FAMILY_PIO, 0xbd, 0xc1, 0x0e, 0x00, 0x00, 0x00, 0x94}}, {{RELAY_IO_CONFIG, RELAY_IO_SHADOW_MASK, RELAY_IO_PIN_INPUTS, RELAY_IO_PIN_INITIAL_STATE}}},
          {OW_NAME_GENERAL_PURPOSE_PIO, {{FAMILY_PIO, 0x02, 0x06, 0x0d, 0x00, 0x00, 0x00, 0x4c}}, {{GENERAL_PURPOSE_IO_CONFIG, GENERAL_PURPOSE_IO_SHADOW_MASK, GENERAL_PURPOSE_IO_PIN_INPUTS, GENERAL_PURPOSE_IO_PIN_INITIAL_STATE}}}};

/* Obviously these need to be in the same order as the above */
static Char *deviceNameList[] = {"RIO_BATTERY_MONITOR",
                          "O1_BATTERY_MONITOR",
                          "O2_BATTERY_MONITOR",
                          "O3_BATTERY_MONITOR",
                          "CHARGER_STATE_PIO",
                          "DARLINGTON_PIO",
                          "RELAY_PIO",
                          "GENERAL_PURPOSE_PIO"};

/* This is the serial port number for the OneWire driver chip */
static SInt32 gPortNumber = -1;

/*
 * FUNCTION PROTOTYPES
 */
static void debugPrintPinsState (void);

/*
 * STATIC FUNCTIONS
 */

/*
 * Print out the address of a OneWire device.
 * 
 * pAddress   pointer to the 8-byte address.
 * pString    pointer to SERIAL_NUM_BUFFER_SIZE
 *            of storage to put the null-terminated
 *            string in. 
 *
 * @return    pointer to pString.
 */
static Char * printAddress (const UInt8 *pAddress, Char *pString)
{
    UInt8 i;
    UInt8 x;
    
    ASSERT_PARAM (pAddress != PNULL, (unsigned long) pAddress);
    ASSERT_PARAM (pString != PNULL, (unsigned long) pString);

    sprintf (pString, "0x");
    x = 2;
    for (i = 0; i < NUM_BYTES_IN_SERIAL_NUM; i++)
    {
        sprintf (pString + x, "%.2x", *pAddress);
        pAddress++;
        x +=2;
    }
    
    *(pString + x - 1) = 0; /* Put the terminator on */
    
    return pString;
}

/*
 * Determine the OneWire device type from the address.
 * 
 * pAddress  pointer to at least the first byte
 *           of an 8-byte address.
 *
 * @return  the type of the device.
 */
static OwDeviceType getDeviceType (const UInt8 *pAddress)
{
    OwDeviceType type = OW_TYPE_UNKNOWN;
    
    ASSERT_PARAM (pAddress != PNULL, (unsigned long) pAddress);
    
    switch (*pAddress)
    {
        case FAMILY_SBATTERY:
            type = OW_TYPE_DS2438_BATTERY_MONITOR;
            break;
        case FAMILY_PIO:
            type = OW_TYPE_DS2408_PIO;
            break;
        default:
            break;
    }
    
    return type;
}    

/*
 * Read a set of pins. 
 * 
 * deviceName  the PIO device that the pins belong to.
 * pPinsState  the returned state of the pins.
 *
 * @return  true if successful, otherwise false.
 */
static Bool readPins (OwDeviceName deviceName, UInt8 *pPinsState)
{
    Bool  success;
    
    /* Read the last state of the pins */
    printDebug ("Reading from %s.\n", deviceNameList[deviceName]);
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], pPinsState);
    if (!success)
    {
        printDebug ("Read failed.\n");
    }

    debugPrintPinsState();
        
    return success;
}

/*
 * Look for rising edges on a set of pins. 
 * 
 * deviceName  the PIO device that the pins belong to.
 * pPinsState  the returned state of the pins.
 *
 * @return  true if successful, otherwise false.
 */
static Bool readAndResetRisingEdgePins (OwDeviceName deviceName, UInt8 *pPinsState)
{
    Bool  success;
    
    /* Read the activity state of the pins and then reset it for the next time */
    printDebug ("Reading latch register from %s.\n", deviceNameList[deviceName]);
    success = readPIOActivityLatchStateRegisterDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], pPinsState);

    if (success)
    {
        success = resetActivityLatchesDS2408  (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0]);
        if (!success)
        {
            printDebug ("Latch reset failed.\n");
        }
    }
    else
    {
        printDebug ("Read failed.\n");        
    }
    
    debugPrintPinsState();

    return success;
}

/*
 * Given a pinsState, mask into it the pins that 
 * are shadowed locally. 
 * 
 * deviceName  the PIO device that the pins belong to.
 * pinsState   the state of the pins according to the
 *             device itself. 
 *
 * @return  the new pinsState.
 */
static UInt8 accountForShadow (OwDeviceName deviceName, UInt8 pinsState)
{
    UInt8 mask;
    UInt8 i;
    
    mask = 1;
    for (i = 0; i < 8; i++)
    {            
        if (gDeviceStaticConfigList[deviceName].specifics.ds2408.readShadowMask & mask)
        {
            if (gDeviceStaticConfigList[deviceName].specifics.ds2408.pinsState & mask)
            {
                pinsState |= mask;
            }
            else
            {
                pinsState &= ~mask;                    
            }
        }
        mask <<= 1;
    }
    
    return pinsState;
}

/*
 * Read a set of pins and use the shadow state
 * if that is configured for this device. 
 * 
 * deviceName  the PIO device that the pins belong to.
 * pPinsState  the returned state of the pins.
 *
 * @return  true if successful, otherwise false.
 */
static Bool readPinsWithShadow (OwDeviceName deviceName, UInt8 *pPinsState)
{
    Bool success;
    
    /* Read the last state of the pins */
    printDebug ("Reading from %s.\n", deviceNameList[deviceName]);
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], pPinsState);
    if (success)
    {
        /* Now check against the shadow mask and for those pins use pinsState instead of the read-back state */
        *pPinsState = accountForShadow (deviceName, *pPinsState);
    }
    else
    {
        printDebug ("Read failed.\n");        
    }

    debugPrintPinsState();
        
    return success;
}

/*
 * Set a pin or pins to on (i.e. 5 Volts) or off
 * (i.e. ground) and take into account the shadow
 * state if necessary. 
 * 
 * deviceName       the PIO device that the pins belong to.
 * pinsMask         the pins to be set to 5 Volts or ground. 
 * setPinsTo5Volts  whether the masked pins are to be set to
 *                  5V (== true) or ground.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setPinsWithShadow (OwDeviceName deviceName, UInt8 pinsMask, Bool setPinsTo5Volts)
{
    Bool  success;
    UInt8 pinsState;
    UInt8 pinsStateToWrite;
    Char buffer[BINARY_STRING_BUFFER_SIZE];
    
    /* Read the last state of the pins, taking shadow state into account */
    success = readPinsWithShadow (deviceName, &pinsState);
    
    /* Set or reset the ones masked in */
    if (setPinsTo5Volts)
    {
        pinsState |= pinsMask;
    }
    else
    {
        pinsState &=~ pinsMask;
    }
    pinsState |= gDeviceStaticConfigList[deviceName].specifics.ds2408.inputMask; /* Write 1's to input pins */
    
    /* Take a copy of the new intended state 'cos channelAccessWriteDS2408 will read back the written
     * state which might not be what we want since we may be shadowing some pins */
    pinsStateToWrite = pinsState;
    printDebug ("Writing %s to %s.\n", binaryString (pinsStateToWrite, &(buffer[0])), deviceNameList[deviceName]);
    success = channelAccessWriteDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], &pinsStateToWrite);
    
    /* If it worked, setup the shadow to match the result */
    if (success)
    {
        gDeviceStaticConfigList[deviceName].specifics.ds2408.pinsState = pinsState;
    }
    else
    {
        printDebug ("Write failed.\n");            
    }

    debugPrintPinsState();
    
    return success;
}

/*
 * Toggle a pin or pins from their current state to the
 * reverse and back again, taking into account the
 * shadow state if required. 
 * 
 * deviceName  the PIO device that the pins belong to.
 * pinsMask    the pins to be toggled (a bit set to 1 is
 *             to be toggled a bit set to 0 is left alone). 
 *
 * @return  true if successful, otherwise false.
 */
static Bool togglePinsWithShadow (OwDeviceName deviceName, UInt8 pinsMask)
{
    Bool  success = true;
    UInt8 pinsState;
    UInt8 pinsStateToWrite;
    UInt8 i;
    Char buffer[BINARY_STRING_BUFFER_SIZE];

    /* Read the last state of the pins, taking shadow state into account */
    success = readPinsWithShadow (deviceName, &pinsState);
        
    debugPrintPinsState();

    /* Toggle the ones masked in */
    for (i = 0; (i < 2) && success; i++)
    {
        if (pinsState & pinsMask)
        {
            pinsState &=~ pinsMask;
        }
        else
        {
            pinsState |= pinsMask;
        }
        pinsState |= gDeviceStaticConfigList[deviceName].specifics.ds2408.inputMask; /* Write 1's to input pins */
        
        /* Take a copy of the new intended state 'cos channelAccessWriteDS2408 will read back the written
         * state which might not be what we want since we may be shadowing some pins */
        pinsStateToWrite = pinsState;
        printDebug ("Writing %s to %s.\n", binaryString (pinsStateToWrite, &(buffer[0])), deviceNameList[deviceName]);
        success = channelAccessWriteDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], &pinsStateToWrite);

        /* If it worked, setup the shadow to match the result */
        if (success)
        {
            gDeviceStaticConfigList[deviceName].specifics.ds2408.pinsState = pinsState;
        }
        else
        {
            printDebug ("Write failed.\n");            
        }

        debugPrintPinsState();
        
        usleep (TOGGLE_DELAY_US);
    }
    

    return success;
}

/*
 * Print the state of all the IO pins for debugging.
 */
static void debugPrintPinsState (void)
{
    Bool success;
    UInt8 pinsState;
    Char buffer[BINARY_STRING_BUFFER_SIZE];
    
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_CHARGER_STATE_PIO].address.value[0], &pinsState);
    if (success)
    {
        /* Now check against the shadow mask and for those pins use pinsState instead of the read-back state */
        pinsState = accountForShadow (OW_NAME_CHARGER_STATE_PIO, pinsState);
        printDebug ("  CHARGER_STATE_PIO: %s.\n", binaryString (pinsState, &(buffer[0])));
    }
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_DARLINGTON_PIO].address.value[0], &pinsState);
    if (success)
    {
        /* Now check against the shadow mask and for those pins use pinsState instead of the read-back state */
        pinsState = accountForShadow (OW_NAME_DARLINGTON_PIO, pinsState);
        printDebug ("     DARLINGTON_PIO: %s.\n", binaryString (pinsState, &(buffer[0])));
    }
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RELAY_PIO].address.value[0], &pinsState);
    if (success)
    {
        /* Now check against the shadow mask and for those pins use pinsState instead of the read-back state */
        pinsState = accountForShadow (OW_NAME_RELAY_PIO, pinsState);
        printDebug ("          RELAY_PIO: %s.\n", binaryString (pinsState, &(buffer[0])));
    }
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_GENERAL_PURPOSE_PIO].address.value[0], &pinsState);
    if (success)
    {
        /* Now check against the shadow mask and for those pins use pinsState instead of the read-back state */
        pinsState = accountForShadow (OW_NAME_GENERAL_PURPOSE_PIO, pinsState);
        printDebug ("GENERAL_PURPOSE_PIO: %s.\n", binaryString (pinsState, &(buffer[0])));
    }
}

/*
 * Helper function to determine ChargeState given a
 * reading from the charger state PIO chip and masks
 * for which pins mean green or red for a charger.
 * Chargers can be off, green or red but never
 * both green and red at the same time.
 * 
 * success     whether the state was successfully
 *             read.
 * poweredOn   whether the charger is powered on.        
 * pinsState   a reading of all the charge states
 *             from the charger state PIO. 
 * greenMask   a bit mask for the pins that mean
 *             green for a charger.
 * redMask     a bit mask for the pins that mean
 *             red for a charger.
 *
 * @return  true if successful, otherwise false.
 */
static ChargeState getChargeState (Bool success, Bool poweredOn, UInt8 pinsState, UInt8 greenMask, UInt8 redMask)
{
    ChargeState state = CHARGE_STATE_UNKNOWN;
    
    if (success)
    {
        state = CHARGE_STATE_NO_POWER;
        if (poweredOn)
        {
            state = CHARGE_STATE_OFF;
            if (pinsState & greenMask)
            {
                state = CHARGE_STATE_GREEN;
            }
            if (pinsState & redMask)
            {
                if (state == CHARGE_STATE_GREEN)
                {
                    state = CHARGE_STATE_BAD;            
                }
                else
                {
                    state = CHARGE_STATE_RED;
                }
            }
        }
    }
    
    return state;
}

/*
 * Helper function to determine the flashing
 * ChargeState.
 * 
 * existingState the current state of the charger. 
 * pinsEdgeState     a reading of all the edge transitions
 *                   of the charger state PIO. 
 * lastPinsEdgeState the previous reading of the edge
 *                   transitions so that we can check
 *                   that this is not simply a pin
 *                   switching entirely off or entirely on.
 * greenMask         a bit mask for the pins that mean
 *                   green for a charger.
 * redMask           a bit mask for the pins that mean
 *                   red for a charger.
 *
 * @return  true if successful, otherwise false.
 */
static ChargeState getChargeFlashingState (ChargeState existingState, UInt8 lastPinsEdgeState, UInt8 pinsEdgeState, UInt8 greenMask, UInt8 redMask)
{
    ChargeState state = existingState;

    if ((pinsEdgeState & greenMask) && (lastPinsEdgeState & greenMask))
    {
        state = CHARGE_STATE_FLASHING_GREEN;
    }
    if ((pinsEdgeState & redMask) && (lastPinsEdgeState & redMask))
    {
        state = CHARGE_STATE_FLASHING_RED;
    }
    
    return state;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Initialise the OneWire bus port.
 *
 * @return  true if successful.
 */
Bool startOneWireBus (void)
{
    Bool success = true;
    
    /* Open the serial port */
    printProgress ("Opening port %s...", ONEWIRE_PORT_STRING);
    gPortNumber = oneWireStartBus (ONEWIRE_PORT_STRING);
    if (!success || (gPortNumber < 0))
    {
        success = false; 
        printProgress (" failed, couldn't detect DS2480!\n");            

    }
    else
    {
        printProgress (" done.\n");        
    }
    
    return success;
}

/*
 * Shut stuff down.
 * 
 * @return  none.
 */
void stopOneWireBus (void)
{
    if (gPortNumber >=0 )
    {
        printProgress ("Closing port.\n");
        oneWireStopBus (gPortNumber);
        gPortNumber = -1;
    }
}

/*
 * Find the devices on the OneWire bus and print
 * them out for information.
 *
 * @return  the number of devices found (can be greater
 *          than MAX_NUM_DEVICES).
 */
UInt8 findAllDevices ()
{
    UInt8 numDevicesFound = 0;
    UInt8 numDevicesToPrint = MAX_NUM_DEVICES;
    UInt8 *pAddress = PNULL;
    UInt8 i;
    UInt8 *pPos;
    
    /* Grab work space for enough addresses */
    pAddress = malloc (NUM_BYTES_IN_SERIAL_NUM * MAX_DEVICES_TO_FIND);
    
    if (pAddress != PNULL)
    {
        /* Find all the devices */
        numDevicesFound = oneWireFindAllDevices (gPortNumber, pAddress, MAX_NUM_DEVICES);
        /* The oneWireFindAllDevices can return more than we ask for, so cap it */
        if (numDevicesToPrint > numDevicesFound)
        {
            numDevicesToPrint = numDevicesFound;
        }        
        
        printProgress ("%d devices found on the OneWire bus", numDevicesFound);
        if (numDevicesFound > numDevicesToPrint)
        {
            printProgress (", the first %d of them are", numDevicesToPrint);
        }
        printProgress (":\n");
        
        /* Print them out */
        pPos = pAddress;
        for (i = 0; i < numDevicesToPrint; i++)
        {
            printProgress ((Char *) pPos, true);
            pPos += NUM_BYTES_IN_SERIAL_NUM;
        }
        
        /* Free the workspace */
        free (pAddress);        
    }
    
    return numDevicesFound;
}

/*
 * Find and setup all the devices we expect to exist on  
 * the OneWire bus, using the configuration data in
 * gDeviceStaticConfigList[].
 *
 * batteriesOnly  if true, only setup the ds2438 devices,
 *                otherwise setup the lot.
 * @return        true if successful, otherwise false.
 */
Bool setupDevices (Bool batteriesOnly)
{
    Bool  success = true;
    Bool  found[MAX_NUM_DEVICES];
    UInt8 *pAddress;
    UInt8 pinsState;
    UInt8 i;
    UInt8 numDevices = MAX_NUM_DEVICES;
    Char  serialNumBuffer[SERIAL_NUM_BUFFER_SIZE];
    
    if (batteriesOnly)
    {
        printProgress ("Setting up OneWire battery devices...\n");
        numDevices = MAX_NUM_BATTERY_DEVICES;
    }
    else
    {
        printProgress ("Setting up all OneWire devices...\n");
    }
    
    for (i = 0; (i < numDevices); i++)
    {
        pAddress = &gDeviceStaticConfigList[i].address.value[0];
        
        /* Try to select the device */
        found[i] = oneWireAccessDevice (gPortNumber, pAddress);
        
        /* If it was found, set it up */
        if (found[i])
        {
            printProgress ("Found %d [%s]: %s, setting it up...", i + 1, deviceNameList[i], printAddress (pAddress, &(serialNumBuffer[0])));
            printDebug ("\n");
            switch (getDeviceType (pAddress))
            {
                case OW_TYPE_DS2438_BATTERY_MONITOR:
                {
                    UInt8 threshold = DEFAULT_DS2438_THRESHOLD;
                    UInt32 elapsedTime;

                    /* Write the config register and the threshold register */
                    printDebug ("Writing config 0x%.2x, threshold %d to DS2438.\n", gDeviceStaticConfigList[i].specifics.ds2438.config, threshold);
                    success = writeNVConfigThresholdDS2438 (gPortNumber, pAddress, &(gDeviceStaticConfigList[i].specifics.ds2438.config), &threshold);
                    if (success)
                    {
                        /* Initialise the remaining capacity and time which are otherwise volatile fields*/
                        printDebug ("Initialising time and remaining capacity in DS2438.\n");
                        success = initTimeCapacityDS2438 (gPortNumber, pAddress);                        

                        if (success)
                        {
                            /* Set time */
                            elapsedTime = getSystemTicks ();
                            printDebug ("Writing time %d to DS2438.\n", elapsedTime);
                            success = writeTimeCapacityDS2438 (gPortNumber, pAddress, &elapsedTime, PNULL, true);
                        }
                    }
                }
                break;
                case OW_TYPE_DS2408_PIO:
                {
                    ASSERT_PARAM (!batteriesOnly, batteriesOnly);
                    /* Disable test mode, just in case, then write the control register and
                     * the pin configuration, using an intermediate variable for the latter
                     * as the write function also reads the result back and I'd rather avoid
                     * my global data structure being modified */
                    printDebug ("Disabling Test Mode to DS2408.\n");
                    success = disableTestModeDS2408 (gPortNumber, pAddress);
                    debugPrintPinsState();
                    if (success)
                    {
                        printDebug ("Writing 0x%.2x to DS2408 control register.\n", gDeviceStaticConfigList[i].specifics.ds2408.config);
                        success = writeControlRegisterDS2408 (gPortNumber, pAddress, gDeviceStaticConfigList[i].specifics.ds2408.config);
                        debugPrintPinsState();
                        if (success)
                        {
                            pinsState = gDeviceStaticConfigList[i].specifics.ds2408.pinsState;
                            pinsState |= gDeviceStaticConfigList[i].specifics.ds2408.inputMask; /* Write 1's to input pins */
                            printDebug ("Writing 0x%.2x to DS2408 pins.\n", pinsState);
                            success = channelAccessWriteDS2408 (gPortNumber, pAddress, &pinsState);
                        }
                        debugPrintPinsState();
                    }
                }
                break;
                default:
                {
                    ASSERT_ALWAYS_PARAM (pAddress[0]);
                    success = false;
                    break;
                }
            }
            if (!success)
            {
                printProgress (" failed!\n");
            }
            else
            {
                printProgress (" done.\n");        
            }
        }
    }
    
    /* Check whether we found everything and say so if not */
    for (i = 0; i < numDevices; i++)
    {
        if (!found[i])
        {
            printProgress ("Couldn't find %s, address: %s.\n", deviceNameList[i], printAddress (&gDeviceStaticConfigList[i].address.value[0], &(serialNumBuffer[0])));
            success = false;
        }
    }

    return success;
}

/*
 * Read 12V from mains pin.
 *
 * pMains12VIsPresent  somewhere to store the Bool result,
 *                     true if 12V from mains is present,
 *                     otherwise false.
 *
 * @return  true if successful, otherwise false.
 */
Bool readMains12VPin (Bool *pMains12VIsPresent)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPins (OW_NAME_RELAY_PIO, &pinsState);
    if (success)
    {
        *pMains12VIsPresent = false;
        if (pinsState & RELAY_12V_DETECT)
        {
            *pMains12VIsPresent = true;
        }
    }

    return success; 
}

/*
 * Read the charger state pins.
 *
 * pPinsState  somewhere to store the state of
 *             the charger pins.
 *
 * @return  true if successful, otherwise false.
 */
Bool readChargerStatePins (UInt8 *pPinsState)
{
    return readPins (OW_NAME_CHARGER_STATE_PIO, pPinsState);
}

/*
 * Read the state of the chargers.  This function
 * must be called at intervals (no less than one
 * second apart) to detect the flashing charge state.
 *
 * pState               a pointer to an array of
 *                      size NUM_CHARGERS to store
 *                      the state.
 * pFlashDetectPossible a pointer to place to store
 *                      whether flash detection was
 *                      possible or not.  Will be 
 *                      true if flash detection was
 *                      possible (i.e. the time
 *                      since the last call was
 *                      right) otherwise false.
 *
 * @return  true if successful, otherwise false.
 */
Bool readChargerState (ChargeState *pState, Bool *pFlashDetectPossible)
{
    Bool success = true;
    static UInt32 ticksAtLastRead = 0;
    static UInt8 lastPinsEdgeState;
    UInt8 pinsState;
    UInt8 pinsEdgeState;
    UInt32 ticksNow;
    Bool chargerPowered;
    Bool relaysPowered;
    
    ASSERT_PARAM (pState != PNULL, (unsigned long) pState);
    ASSERT_PARAM (pFlashDetectPossible != PNULL, (unsigned long) pFlashDetectPossible);
    
    /* Setup the statics and reset the edge latch if this is the first call */
    if (ticksAtLastRead == 0)
    {
        success = readAndResetRisingEdgePins (OW_NAME_CHARGER_STATE_PIO, &lastPinsEdgeState); 
        ticksAtLastRead = getSystemTicks();
    }
    
    if (success)
    {
        /* Get the time */
        ticksNow = getSystemTicks();
        
        /* Read the current state */
        success = readPins (OW_NAME_CHARGER_STATE_PIO, &pinsState);
        if (success)
        {
            *pFlashDetectPossible = false;
            
            /* Determine the states of charge from the reading and
             * the charger state itself (powered on or off)
             * Note: the wiring on RoboOne is such that unless
             * the relays are powered and the relay for that charger
             * deliberately switched off, the charger is powered ON */
            success = readExternalRelaysEnabled (&relaysPowered);
            if (success)
            {
                success =  readRioBatteryCharger (&chargerPowered);
                *(pState + CHARGER_RIO) = getChargeState (success, !(relaysPowered && !chargerPowered), pinsState, CHARGER_RIO_GREEN, CHARGER_RIO_RED);
                success =  readO1BatteryCharger (&chargerPowered);
                *(pState + CHARGER_O1) = getChargeState (success, !(relaysPowered && !chargerPowered), pinsState, CHARGER_O1_GREEN, CHARGER_O1_RED);
                success =  readO2BatteryCharger (&chargerPowered);
                *(pState + CHARGER_O2) = getChargeState (success, !(relaysPowered && !chargerPowered), pinsState, CHARGER_O2_GREEN, CHARGER_O2_RED);
                success =  readO3BatteryCharger (&chargerPowered);
                *(pState + CHARGER_O3) = getChargeState (success, !(relaysPowered && !chargerPowered), pinsState, CHARGER_O3_GREEN, CHARGER_O3_RED);
                
                /* Setup the flashing results if it's been long enough since the last reading to tell */
                if ((ticksAtLastRead - ticksNow) > 1)
                {
                    success = readAndResetRisingEdgePins (OW_NAME_CHARGER_STATE_PIO, &pinsEdgeState); 
                    if (success)
                    {
                        *pFlashDetectPossible = true;
                        *(pState + CHARGER_RIO) = getChargeFlashingState (*(pState + CHARGER_RIO), lastPinsEdgeState, pinsEdgeState, CHARGER_RIO_GREEN, CHARGER_RIO_RED);
                        *(pState + CHARGER_O1) = getChargeFlashingState (*(pState + CHARGER_O1), lastPinsEdgeState, pinsEdgeState, CHARGER_O1_GREEN, CHARGER_O1_RED);
                        *(pState + CHARGER_O2) = getChargeFlashingState (*(pState + CHARGER_O2), lastPinsEdgeState, pinsEdgeState, CHARGER_O2_GREEN, CHARGER_O2_RED);
                        *(pState + CHARGER_O3) = getChargeFlashingState (*(pState + CHARGER_O3), lastPinsEdgeState, pinsEdgeState, CHARGER_O3_GREEN, CHARGER_O3_RED);
                        lastPinsEdgeState = pinsEdgeState;
                        ticksAtLastRead = ticksNow;
                    }
                }
            }
        }
    }

    return success;
}

/*
 * Switch the Orangutan power relay from it's current 
 * state to the reverse and back again.
 * This function also speculatively opens the Orangutan
 * in case this is a power on.
 *
 * @return  true if successful, otherwise false.
 */
Bool toggleOPwr (void)
{
    Bool success;
    
    success = togglePinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_O_PWR_TOGGLE);
    
    openOrangutan();

    return success;
}

/*
 * Read the state of Orangutan power relay.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readOPwr (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_DARLINGTON_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = false;
        if (pinsState & DARLINGTON_O_PWR_TOGGLE)
        {
            *pIsOn = true;
        }        
    }
    
    return success;
}

/*
 * Switch the Orangutan reset relay from it's current 
 * state to the reverse and back again.
 * This function also er-opens the Orangutan port.
 *
 * @return  true if successful, otherwise false.
 */
Bool toggleORst (void)
{
    Bool success;
    
    success = togglePinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_O_RESET_TOGGLE);

    openOrangutan();
    
    return success;
}

/*
 * Read the state of Orangutan reset relay.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readORst (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_DARLINGTON_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = false;
        if (pinsState & DARLINGTON_O_RESET_TOGGLE)
        {
            *pIsOn = true;
        }        
    }
    
    return success;
}

/*
 * Switch the Raspberry Pi reset relay from it's current 
 * state to the reverse and back again.
 *
 * @return  true if successful, otherwise false.
 */
Bool togglePiRst (void)
{
    Bool success = true;
    
    /* TODO: do this via signalling to the Orangutan */
    
    return success;
}

/*
 * Switch the relay that allows 12 Volts to be supplied
 * to the RIO to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setRioPwr12VOn (void)
{
    return setPinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_RIO_PWR_12V_ON, true);
}

/*
 * Switch the relay that allows 12 Volts to be
 * supplied to the RIO to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setRioPwr12VOff (void)
{
    return setPinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_RIO_PWR_12V_ON, false);
}

/*
 * Read the state of 12V power to the Rio.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readRioPwr12V (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_DARLINGTON_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = false;
        if (pinsState & DARLINGTON_RIO_PWR_12V_ON)
        {
            *pIsOn = true;
        }        
    }
    
    return success;
}

/*
 * Switch the relay that allows on-board battery
 * power to be supplied to the RIO to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setRioPwrBattOn (void)
{
    return setPinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_RIO_PWR_BATT_OFF, false);
}

/*
 * Switch the relay that allows on-board battery
 * power to be supplied to the RIO to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setRioPwrBattOff (void)
{
    return setPinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_RIO_PWR_BATT_OFF, true);
}

/*
 * Read the state of battery power to the Rio.
 * Note that what is returned is the state of
 * battery power, not the relay (since the relay
 * switches ON to switch the battery power OFF).
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readRioPwrBatt (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_DARLINGTON_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = true;
        if (pinsState & DARLINGTON_RIO_PWR_BATT_OFF)
        {
            *pIsOn = false;
        }        
    }
    
    return success;
}

/*
 * Switch the relay that allows 12 Volts to be
 * supplied to the Orangutan to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setOPwr12VOn (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O_PWR_12V_ON, true);
}

/*
 * Switch the relay that allows 12 Volts to be
 * supplied to the Orangutan to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setOPwr12VOff (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O_PWR_12V_ON, false);
}

/*
 * Read the state of 12V power to the Orangutan.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readOPwr12V (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_RELAY_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = false;
        if (pinsState & RELAY_O_PWR_12V_ON)
        {
           *pIsOn = true;
        }        
    }
    
    return success;
}

/*
 * Switch the relay that allows on-board battery
 * power to be supplied to the Orangutan to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setOPwrBattOn (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O_PWR_BATT_OFF, false);
}

/*
 * Switch the relay that allows battery power to be
 * supplied to the Orangutan to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setOPwrBattOff (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O_PWR_BATT_OFF, true);
}

/*
 * Read the state of battery power to the Orangutan.
 * Note that what is returned is the state of
 * battery power, not the relay (since the relay
 * switches ON to switch the battery power OFF).
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readOPwrBatt (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_RELAY_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = true;
        if (pinsState & RELAY_O_PWR_BATT_OFF)
        {
            *pIsOn = false;
        }        
    }
    
    return success;
}

/*
 * Switch the RIO battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setRioBatteryChargerOn (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_RIO_CHARGER_OFF, false);
}

/*
 * Switch the RIO battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setRioBatteryChargerOff (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_RIO_CHARGER_OFF, true);
}

/*
 * Read whether the charger is connected
 * to the Rio battery.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readRioBatteryCharger (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_RELAY_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = true;
        if (pinsState & RELAY_RIO_CHARGER_OFF)
        {
            *pIsOn = false;
        }        
    }
    
    return success;
}

/*
 * Switch the O1 battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setO1BatteryChargerOn (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O1_CHARGER_OFF, false);
}

/*
 * Switch the O1 battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setO1BatteryChargerOff (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O1_CHARGER_OFF, true);
}

/*
 * Read whether the charger is connected
 * to Orangutan battery number 1.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readO1BatteryCharger (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_RELAY_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = true;
        if (pinsState & RELAY_O1_CHARGER_OFF)
        {
            *pIsOn = false;
        }        
    }
    
    return success;
}

/*
 * Switch the O2 battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setO2BatteryChargerOn (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O2_CHARGER_OFF, false);
}

/*
 * Switch the O2 battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setO2BatteryChargerOff (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O2_CHARGER_OFF, true);
}

/*
 * Read whether the charger is connected
 * to Orangutan battery number 2.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readO2BatteryCharger (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_RELAY_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = true;
        if (pinsState & RELAY_O2_CHARGER_OFF)
        {
            *pIsOn = false;
        }        
    }
    
    return success;
}

/*
 * Switch the O3 battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setO3BatteryChargerOn (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O3_CHARGER_OFF, false);
}

/*
 * Switch the O3 battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setO3BatteryChargerOff (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_O3_CHARGER_OFF, true);
}

/*
 * Read whether the charger is connected
 * to Orangutan battery number 3.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readO3BatteryCharger (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    success = readPinsWithShadow (OW_NAME_RELAY_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = true;
        if (pinsState & RELAY_O3_CHARGER_OFF)
        {
            *pIsOn = false;
        }        
    }
    
    return success;
}

/*
 * Switch all battery chargers ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setAllBatteryChargersOn (void)
{
    Bool success;
    
    success = setRioBatteryChargerOn();
    if (success)
    {
        success = setO1BatteryChargerOn();
        if (success)
        {
            success = setO2BatteryChargerOn();
            if (success)
            {
                success = setO3BatteryChargerOn();
            }
        }
    }
    
    return success;
}

/*
 * Switch all battery chargers OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setAllBatteryChargersOff (void)
{
    Bool success;
    
    success = setRioBatteryChargerOff();
    if (success)
    {
        success = setO1BatteryChargerOff();
        if (success)
        {
            success = setO2BatteryChargerOff();
            if (success)
            {
                success = setO3BatteryChargerOff();
            }
        }
    }
    
    return success;
}

/*
 * Switch all Orangutan battery chargers ON.
 *
 * @return  true if successful, otherwise false.
 */
Bool setAllOChargersOn (void)
{
    Bool success;
    
    success = setO1BatteryChargerOn();
    if (success)
    {
        success = setO2BatteryChargerOn();
        if (success)
        {
            success = setO3BatteryChargerOn();
        }
    }
    
    return success;
}

/*
 * Switch all Orangutan battery chargers OFF.
 *
 * @return  true if successful, otherwise false.
 */
Bool setAllOChargersOff (void)
{
    Bool success;
    
    success = setO1BatteryChargerOff();
    if (success)
    {
        success = setO2BatteryChargerOff();
        if (success)
        {
            success = setO3BatteryChargerOff();
        }
    }
    
    return success;
}

/*
 * Disable the power to all on-PCB relays,
 * the ones that control the Pi/RIO power
 * source and toggling of power and reset
 * to the Orangutan.  Note that this doesn't
 * change their logical state, i.e. when power
 * is enabled to them they will return to
 * their previous logic state.
 * 
 * @return  true if successful, otherwise false.
 */
Bool disableOnPCBRelays (void)
{
    return setPinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_ENABLE_BAR, true);
}

/*
 * Enable the power to all on-PCB relays,
 * the ones that control the Pi/RIO power
 * source and toggling of power and reset
 * to the Orangutan, returning them to
 * their previous logic state.
 * 
 * @return  true if successful, otherwise false.
 */
Bool enableOnPCBRelays (void)
{
    return setPinsWithShadow (OW_NAME_DARLINGTON_PIO, DARLINGTON_ENABLE_BAR, false);
}

/*
 * Read the state of power to the on-PCB relays.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readOnPCBRelaysEnabled (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    /* Read the state of power to the on-PCB relays */
    success = readPinsWithShadow (OW_NAME_DARLINGTON_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = false;
        if ((pinsState & DARLINGTON_ENABLE_BAR) == 0)
        {
            *pIsOn = true;
        }        
    }
    
    return success;
}

/*
 * Disable the power to all external relays,
 * the ones that control the chargers and
 * whether the Hindbrain is running from
 * 12V or battery power. Note that this doesn't
 * change their logical state, i.e. when power
 * is enabled to them they will return to
 * their previous logic state.
 * 
 * @return  true if successful, otherwise false.
 */
Bool disableExternalRelays (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_ENABLE_BAR, true);
}

/*
 * Disable the power to all external relays,
 * the ones that control the chargers and
 * whether the Hindbrain is running from
 * 12V or battery power, returning them to
 * their previous logic state.
 * 
 * @return  true if successful, otherwise false.
 */
Bool enableExternalRelays (void)
{
    return setPinsWithShadow (OW_NAME_RELAY_PIO, RELAY_ENABLE_BAR, false);
}

/*
 * Read the state of power to the external relays.
 *
 * pIsOn  a place to return the state, true
 *        for On.  May be PNULL, in which case
 *        the read is performed but no value
 *        is returned.
 *
 * @return  true if successful, otherwise false.
 */
Bool readExternalRelaysEnabled (Bool *pIsOn)
{
    Bool success;
    UInt8 pinsState;
    
    /* Read the state of power to the external relays */
    success = readPinsWithShadow (OW_NAME_RELAY_PIO, &pinsState);
    
    if (success && (pIsOn != PNULL))
    {
        *pIsOn = false;
        if ((pinsState & RELAY_ENABLE_BAR) == 0)
        {
            *pIsOn = true;
        }        
    }
    
    return success;
}

/*
 * Read the state of the General Purpose IO pins.
 * 
 * pPinsState  somewhere to store the state of
 *             the pins.
 *
 * @return  true if successful, otherwise false.
 */
Bool readGeneralPurposeIOs (UInt8 *pPinsState)
{
    return readPinsWithShadow (OW_NAME_GENERAL_PURPOSE_PIO, pPinsState);
}

/*
 * Set the input port used on the analogue mux.
 * For this to be any use jumpers J1, J2, J3 and J4
 * need to be shorted on the RoboOne PCB. 
 * 
 * input   the number of the input to be connected,
 *         from 0 to 7.
 *
 * @return  true if successful, otherwise false.
 */
Bool setAnalogueMuxInput (UInt8 input)
{
    Bool  success = true;
    UInt8 pinsState;
    UInt8 pinsStateToWrite;
    
    ASSERT_PARAM (input < 8, input);

    /* Read the last state of the pins on the general purpose
     * IO chip, which has the relevant output lines */
    success = readPinsWithShadow (OW_NAME_GENERAL_PURPOSE_PIO, &pinsState);

    /* First, disable the device while we change things */
    if (success)
    {
        pinsState |= GENERAL_PURPOSE_IO_MUX_ENABLE_BAR;
        pinsStateToWrite = pinsState;
        success = channelAccessWriteDS2408 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_GENERAL_PURPOSE_PIO].address.value[0], &pinsStateToWrite);
        if (success)
        {
            /* Setup the shadow to match the result */
            gDeviceStaticConfigList[OW_NAME_GENERAL_PURPOSE_PIO].specifics.ds2408.pinsState = pinsState;
            /* Now set the IO lines attached to pins A0 to A2 of the HEF4051B */
            pinsStateToWrite = pinsState;
            pinsStateToWrite &= ~(GENERAL_PURPOSE_IO_MUX_A0 | GENERAL_PURPOSE_IO_MUX_A1 | GENERAL_PURPOSE_IO_MUX_A2);
            pinsStateToWrite |= (input << GENERAL_PURPOSE_IO_MUX_SHIFT) & (GENERAL_PURPOSE_IO_MUX_A0 | GENERAL_PURPOSE_IO_MUX_A1 | GENERAL_PURPOSE_IO_MUX_A2); 
            success = channelAccessWriteDS2408 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_GENERAL_PURPOSE_PIO].address.value[0], &pinsStateToWrite);
            
            if (success)
            {
                /* Setup the shadow to match the result */
                gDeviceStaticConfigList[OW_NAME_GENERAL_PURPOSE_PIO].specifics.ds2408.pinsState = pinsState;

                /* Now enable the mux */
                pinsState &= ~GENERAL_PURPOSE_IO_MUX_ENABLE_BAR;
                pinsStateToWrite = pinsState;
                success = channelAccessWriteDS2408 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_GENERAL_PURPOSE_PIO].address.value[0], &pinsStateToWrite);
                if (success)
                {
                    /* Setup the shadow to match the result */
                    gDeviceStaticConfigList[OW_NAME_GENERAL_PURPOSE_PIO].specifics.ds2408.pinsState = pinsState;
                }
            }            
        }
    }
    
    return success;
}

/*
 * Read the voltage of the analogue mux (which
 * is connected to the RIO Battery Monitor A/D line).
 * 
 * pVoltage  a pointer to somewhere to put the Voltage reading.
 *
 * @return  true if successful, otherwise false.
 */
Bool readAnalogueMux (UInt16 *pVoltage)
{
    return readVadDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], pVoltage);
}

/*
 * Read the current being drawn from the Rio/Pi/5V battery.
 *
 * pCurrent  a pointer to somewhere to put the current reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readRioBattCurrent (SInt16 *pCurrent)
{
    return readCurrentDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], pCurrent);
}

/*
 * Read the current being drawn from the O1 battery.
 *
 * pCurrent  a pointer to somewhere to put the current reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO1BattCurrent (SInt16 *pCurrent)
{
    return readCurrentDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], pCurrent);
}

/*
 * Read the current being drawn from the O2 battery.
 *
 * pCurrent  a pointer to somewhere to put the current reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO2BattCurrent (SInt16 *pCurrent)
{
    return readCurrentDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], pCurrent);
}

/*
 * Read the current being drawn from the O3 battery.
 *
 * pCurrent  a pointer to somewhere to put the current reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO3BattCurrent (SInt16 *pCurrent)
{
    return readCurrentDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], pCurrent);
}

/*
 * Read the Voltage at the Rio/Pi/5V battery.
 *
 * pVoltage  a pointer to somewhere to put the Voltage reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readRioBattVoltage (UInt16 *pVoltage)
{
    return readVddDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], pVoltage);
}

/*
 * Read the Voltage at the O1 battery.
 *
 * pVoltage  a pointer to somewhere to put the Voltage reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO1BattVoltage (UInt16 *pVoltage)
{
    return readVddDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], pVoltage);
}

/*
 * Read the Voltage at the O2 battery.
 *
 * pVoltage  a pointer to somewhere to put the Voltage reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO2BattVoltage (UInt16 *pVoltage)
{
    return readVddDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], pVoltage);
}

/*
 * Read the Voltage at the O3 battery.
 *
 * pVoltage  a pointer to somewhere to put the Voltage reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO3BattVoltage (UInt16 *pVoltage)
{
    return readVddDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], pVoltage);
}

/*
 * Read the accumulated remaining capacity of the Rio/Pi/5V battery.
 *
 * pRemainingCapacity  a pointer to somewhere to put the reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readRioRemainingCapacity (UInt16 *pRemainingCapacity)
{
    return readTimeCapacityCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], PNULL, pRemainingCapacity, PNULL, true);
}

/*
 * Read the accumulated remaining capacity of the O1 battery.
 *
 * pRemainingCapacity  a pointer to somewhere to put the reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO1RemainingCapacity (UInt16 *pRemainingCapacity)
{
    return readTimeCapacityCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], PNULL, pRemainingCapacity, PNULL, true);
}

/*
 * Read the accumulated remaining capacity of the O2 battery.
 *
 * pRemainingCapacity  a pointer to somewhere to put the reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO2RemainingCapacity (UInt16 *pRemainingCapacity)
{
    return readTimeCapacityCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], PNULL, pRemainingCapacity, PNULL, true);
}

/*
 * Read the accumulated remaining capacity of the O3 battery.
 *
 * pRemainingCapacity  a pointer to somewhere to put the reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO3RemainingCapacity (UInt16 *pRemainingCapacity)
{
    return readTimeCapacityCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], PNULL, pRemainingCapacity, PNULL, true);
}

/*
 * Read the lifetime charge/discharge data of the Rio/Pi/5V battery.
 *
 * pCharge  a pointer to somewhere to put the charge reading.
 * pDischarge  a pointer to somewhere to put the discharge reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readRioBattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge)
{
    return readNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], pCharge, pDischarge);
}

/*
 * Read the lifetime charge/discharge data of the O1 battery.
 *
 * pCharge  a pointer to somewhere to put the charge reading.
 * pDischarge  a pointer to somewhere to put the discharge reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO1BattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge)
{
    return readNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], pCharge, pDischarge);
}

/*
 * Read the lifetime charge/discharge data of the O2 battery.
 *
 * pCharge  a pointer to somewhere to put the charge reading.
 * pDischarge  a pointer to somewhere to put the discharge reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO2BattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge)
{
    return readNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], pCharge, pDischarge);
}

/*
 * Read the lifetime charge/discharge data of the O3 battery.
 *
 * pCharge  a pointer to somewhere to put the charge reading.
 * pDischarge  a pointer to somewhere to put the discharge reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO3BattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge)
{
    return readNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], pCharge, pDischarge);
}

/*
 * Calibrate the Rio/Pi/5V battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
Bool performCalRioBatteryMonitor (void)
{
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * Calibrate the O1 battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
Bool performCalO1BatteryMonitor (void)
{
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * Calibrate the O2 battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
Bool performCalO2BatteryMonitor (void)
{
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * Calibrate the O3 battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
Bool performCalO3BatteryMonitor (void)
{
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * Calibrate all battery monitors.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery, or the
 * sense resistor is shorted.
 * 
 * @return  true if successful, otherwise false.
 */
Bool performCalAllBatteryMonitors (void)
{
    Bool success;
    
    success = performCalRioBatteryMonitor();
    if (success)
    {
        success = performCalO1BatteryMonitor();
        if (success)
        {
            success = performCalO2BatteryMonitor();
            if (success)
            {
                success = performCalO3BatteryMonitor();
            }
        }
    }
    
    return success;
}

/*
 * Set the remaining capacity data of the Rio/Pi/5V battery
 * and the time, then reset the charge/discharge
 * accumulators as well.
 *
 * systemTime          the system time in seconds.
 * remainingCapacity   the remaining capacity in mAhrs.
 * 
 * @return  true if successful, otherwise false.
 */
Bool swapRioBattery (UInt32 systemTime, UInt16 remainingCapacity)
{
    Bool success;
    UInt32 charge = remainingCapacity;
    UInt32 discharge = 0;

    success = writeTimeCapacityDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], &systemTime, &remainingCapacity, true);
    
    if (success)
    {
        success = writeNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], &charge, &discharge);
    }

    return success;
}

/*
 * Set the remaining capacity data of the O1 battery
 * and the time, then reset the charge/discharge
 * accumulators as well.
 *
 * systemTime          the system time in seconds.
 * remainingCapacity   the remaining capacity in mAhrs.
 * 
 * @return  true if successful, otherwise false.
 */
Bool swapO1Battery (UInt32 systemTime, UInt16 remainingCapacity)
{
    Bool success;
    UInt32 charge = remainingCapacity;
    UInt32 discharge = 0;

    success = writeTimeCapacityDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], &systemTime, &remainingCapacity, true);
    
    if (success)
    {
        success = writeNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], &charge, &discharge);
    }
    
    return success;
}

/*
 * Set the remaining capacity data of the O2 battery
 * and the time, then reset the charge/discharge
 * accumulators as well.
 *
 * systemTime          the system time in seconds.
 * remainingCapacity   the remaining capacity in mAhrs.
 * 
 * @return  true if successful, otherwise false.
 */
Bool swapO2Battery (UInt32 systemTime, UInt16 remainingCapacity)
{
    Bool success;
    UInt32 charge = remainingCapacity;
    UInt32 discharge = 0;

    success = writeTimeCapacityDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], &systemTime, &remainingCapacity, true);
    
    if (success)
    {
        success = writeNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], &charge, &discharge);
    }
    
    return success;
}

/*
 * Set the remaining capacity data of the O3 battery
 * and the time, then reset the charge/discharge
 * accumulators as well.
 *
 * systemTime          the system time in seconds.
 * remainingCapacity   the remaining capacity in mAhrs.
 * 
 * @return  true if successful, otherwise false.
 */
Bool swapO3Battery (UInt32 systemTime, UInt16 remainingCapacity)
{
    Bool success;
    UInt32 charge = remainingCapacity;
    UInt32 discharge = 0;

    success = writeTimeCapacityDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], &systemTime, &remainingCapacity, true);
    
    if (success)
    {
        success = writeNVChargeDischargeDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], &charge, &discharge);
    }
    
    return success;
}

/*
 * Read the temperature at the Rio/Pi battery.
 *
 * pTemperature  a pointer to somewhere to put the temperature reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readRioBattTemperature (double *pTemperature)
{
    Bool success;
    UInt16 voltage;
    
    success = setAnalogueMuxInput (TEMPERATURE_SENSOR_RIO_BATT);
    
    if (success)
    {
        success = readAnalogueMux (&voltage);
        
        if (success)            
        {
            if (pTemperature != NULL)
            {
                *pTemperature = MV_TO_C (voltage);
            }
        }
    }
    
    return success;
}

/*
 * Read the temperature at the O1 battery.
 *
 * pTemperature  a pointer to somewhere to put the temperature reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO1BattTemperature (double *pTemperature)
{
    Bool success;
    UInt16 voltage;
    
    success = setAnalogueMuxInput (TEMPERATURE_SENSOR_O1_BATT);
    
    if (success)
    {
        success = readAnalogueMux (&voltage);
        
        if (success)            
        {
            if (pTemperature != NULL)
            {
                *pTemperature = MV_TO_C (voltage);
            }
        }
    }
    
    return success;
}

/*
 * Read the temperature at the O2 battery.
 *
 * pTemperature  a pointer to somewhere to put the temperature reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO2BattTemperature (double *pTemperature)
{
    Bool success;
    UInt16 voltage;
    
    success = setAnalogueMuxInput (TEMPERATURE_SENSOR_O2_BATT);
    
    if (success)
    {
        success = readAnalogueMux (&voltage);
        
        if (success)            
        {
            if (pTemperature != NULL)
            {
                *pTemperature = MV_TO_C (voltage);
            }
        }
    }
    
    return success;
}

/*
 * Read the temperature at the O3 battery.
 *
 * pTemperature  a pointer to somewhere to put the temperature reading.
 * 
 * @return  true if successful, otherwise false.
 */
Bool readO3BattTemperature (double *pTemperature)
{
    Bool success;
    UInt16 voltage;
    
    success = setAnalogueMuxInput (TEMPERATURE_SENSOR_O3_BATT);
    
    if (success)
    {
        success = readAnalogueMux (&voltage);
        
        if (success)            
        {
            if (pTemperature != NULL)
            {
                *pTemperature = MV_TO_C (voltage);
            }
        }
    }
    
    return success;
}

