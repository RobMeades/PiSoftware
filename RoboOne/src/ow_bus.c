/*
 * One Wire bus handling thread for RoboOne.
 */ 
 
#include <stdio.h>
#include <string.h>
#include <ownet.h>
#include <atod26.h>
#include <rob_system.h>
#include <one_wire.h>
#include <hw_config.h>

/*
 * MANIFEST CONSTANTS
 */

#define MAX_NUM_DEVICES 8    /* This MUST be the same as the number of elements in the gDeviceStaticConfigList[] below */
#define TOGGLE_DELAY_MS 500  /* How long to toggle a set of pins from current state to opposite and back again */

/* Enable current measurement, integrated current accummulator, charge/discharge
 * counting and shadowing of charge/discharge count to non-volatile storage */
#define DEFAULT_DS2438_CONFIG         (DS2438_IAD_IS_ENABLED | DS2438_CA_IS_ENABLED | DS2438_EE_IS_ENABLED)
#define RIO_BATTERY_MONITOR_CONFIG    DEFAULT_DS2438_CONFIG
#define O1_BATTERY_MONITOR_CONFIG     DEFAULT_DS2438_CONFIG
#define O2_BATTERY_MONITOR_CONFIG     DEFAULT_DS2438_CONFIG
#define O3_BATTERY_MONITOR_CONFIG     DEFAULT_DS2438_CONFIG

/* RSTZ is a reset line, set power on reset back to 0,
 * don't care about conditional search as we don't use it */
#define DEFAULT_DS2408_CONFIG         (~DS2408_DEVICE_HAS_POWER_ON_RESET & ~DS2408_RSTZ_IS_STROBE)
#define CHARGER_STATE_IO_CONFIG       DEFAULT_DS2408_CONFIG
#define SCHOTTKY_IO_CONFIG            DEFAULT_DS2408_CONFIG
#define RELAY_IO_CONFIG               DEFAULT_DS2408_CONFIG
#define GENERAL_PURPOSE_IO_CONFIG     DEFAULT_DS2408_CONFIG

/* All pins low to begin with apart from charger state which is allowed to float as an input */
#define CHARGER_STATE_IO_PIN_CONFIG   0xFF
#define SCHOTTKY_IO_PIN_CONFIG        0x00
#define RELAY_IO_PIN_CONFIG           0x00
#define GENERAL_PURPOSE_IO_PIN_CONFIG 0x00

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
    OW_NAME_SCHOTTKY_PIO = 5,
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
    UInt8 pinsState;
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

/* ORDER IS IMPORTANT - the OwDeviceName enum is used to index into this */
OwDevicesStaticConfig gDeviceStaticConfigList[] =
         {{OW_NAME_RIO_BATTERY_MONITOR, {{SBATTERY_FAM, 0xb5, 0x02, 0xb3, 0x01, 0x00, 0x00, 0xbc}}, {{RIO_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O1_BATTERY_MONITOR, {{SBATTERY_FAM, 0x84, 0x0d, 0xb3, 0x01, 0x00, 0x00, 0x09}}, {{O1_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O2_BATTERY_MONITOR, {{SBATTERY_FAM, 0xdd, 0x29, 0xb3, 0x01, 0x00, 0x00, 0x56}}, {{O2_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O3_BATTERY_MONITOR, {{SBATTERY_FAM, 0x82, 0x30, 0xb3, 0x01, 0x00, 0x00, 0xd3}}, {{O3_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_CHARGER_STATE_PIO, {{PIO_FAM, 0x8d, 0xf2, 0x0c, 0x00, 0x00, 0x00, 0xb4}}, {{CHARGER_STATE_IO_CONFIG, CHARGER_STATE_IO_PIN_CONFIG}}},
          {OW_NAME_SCHOTTKY_PIO, {{PIO_FAM, 0x7f, 0x6e, 0x0d, 0x00, 0x00, 0x00, 0xb1}}, {{SCHOTTKY_IO_CONFIG, SCHOTTKY_IO_PIN_CONFIG}}},
          {OW_NAME_RELAY_PIO, {{PIO_FAM, 0x5e, 0x64, 0x0d, 0x00, 0x00, 0x00, 0x8d}}, {{RELAY_IO_CONFIG, RELAY_IO_PIN_CONFIG}}},
          {OW_NAME_GENERAL_PURPOSE_PIO, {{PIO_FAM, 0x50, 0x64, 0x0d, 0x00, 0x00, 0x00, 0x9e}}, {{GENERAL_PURPOSE_IO_CONFIG, GENERAL_PURPOSE_IO_PIN_CONFIG}}}};

/* Obviously these need to be in the same order as the above */
Char *deviceNameList[] = {"RIO_BATTERY_MONITOR",
                          "O1_BATTERY_MONITOR",
                          "O2_BATTERY_MONITOR",
                          "O3_BATTERY_MONITOR",
                          "CHARGER_STATE_PIO",
                          "SCHOTTKY_PIO",
                          "RELAY_PIO",
                          "GENERAL_PURPOSE_PIO"};
SInt32 gPortNumber = -1;

/*
 * STATIC FUNCTIONS
 */

/*
 * Print out an address for debug purposes.
 * 
 * pAddress   pointer to the 8-byte address.
 * newline    if true, add a newline on the end.
 *
 * @return  none.
 */
static void printAddress (const UInt8 *pAddress, Bool newline)
{
    UInt8 i;
    
    ASSERT_PARAM (pAddress != PNULL, (unsigned long) pAddress);

    for (i = 0; i < NUM_BYTES_IN_SERIAL_NUM; i++)
    {
        printProgress ("%.2x", *pAddress);
        pAddress++;                
    }
    
    if (newline)
    {
        printProgress ("\n");
    }
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
        case SBATTERY_FAM:
            type = OW_TYPE_DS2438_BATTERY_MONITOR;
            break;
        case PIO_FAM:
            type = OW_TYPE_DS2408_PIO;
            break;
        default:
            break;
    }
    
    return type;
}    
      
/*
 * Initialise stuff:
 *
 * - Open the serial port for OneWire comms
 * 
 * pPort   pointer to a string that represents
 *         the port to use for serial comms.
 *
 * @return  true if successful.
 */
static Bool startOneWireBus (Char * pPort)
{
    Bool success = true;

    ASSERT_PARAM (pPort != PNULL, (unsigned long) pPort);

    printProgress ("Opening port %s...", pPort);
    /* Open the serial port */
    gPortNumber = owAcquireEx (pPort);
    if (gPortNumber < 0)
    {
        success = false;
        printProgress (" failed!\n");
    }
    else
    {
        printProgress (" done.\n");        
    }
        
    return success;
}

/*
 * Shut stuff down:
 *
 * - Release the serial port
 * 
 * @return  none.
 */
static void stopOneWireBus (void)
{
    printProgress ("Closing port.\n");
    owRelease (gPortNumber);
}

/*
 * Find the devices on the OneWire bus and print
 * them out for information.
 *
 * @return  the number of devices found (can be greater
 *          than MAX_NUM_DEVICES).
 */
static UInt8 findAllDevices ()
{
    UInt8 numDevicesFound = 0;
    UInt8 numDevicesToPrint = MAX_NUM_DEVICES;
    UInt8 *pAddresses;
    UInt8 i;
    UInt8 *pPos;
    
    /* Grab work space for enough addresses */
    pAddresses = malloc (NUM_BYTES_IN_SERIAL_NUM * MAX_NUM_DEVICES);

    if (pAddresses != PNULL)
    {
        /* Find all the devices */
        numDevicesFound = owFindAllDevices (gPortNumber, pAddresses, MAX_NUM_DEVICES);
        
        /* The owFindAllDevices can return more than we ask for, so cap it */
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
        pPos = pAddresses;
        for (i = 0; i < numDevicesToPrint; i++)
        {
            printAddress (pPos, true);
            pPos += NUM_BYTES_IN_SERIAL_NUM;
        }        
    }
    
    /* Free the workspace */
    free (pAddresses);
    
    return numDevicesFound;
}

/*
 * Find and setup all the devices we expect to exist on  
 * the OneWire bus, using the configuration data in
 * gDeviceStaticConfigList[].
 *
 * @return  true if successful, otherwise false.
 */
static Bool setupDevices (void)
{
    Bool  success = true;
    Bool  found[MAX_NUM_DEVICES];
    UInt8 *pAddress;
    UInt8 pinsState;
    UInt8 i;
    
    printProgress ("Setting up OneWire devices...\n");
    for (i = 0; (i < MAX_NUM_DEVICES); i++)
    {
        pAddress = &gDeviceStaticConfigList[i].address.value[0];
        
        /* Try to select the device */
        owSerialNum (gPortNumber, pAddress, false);
        found[i] = owAccess (gPortNumber);
        
        /* If it was found, set it up */
        if (found[i])
        {
            printProgress ("Found %d [%s]: ", i + 1, deviceNameList[i]);
            printAddress (pAddress, false);
            printProgress (", setting it up...");
            switch (getDeviceType (pAddress))
            {
                case OW_TYPE_DS2438_BATTERY_MONITOR:
                    /* Write the config register, leaving the threshold alone */
                    success = writeNVConfigThresholdDS2438 (gPortNumber, pAddress, &gDeviceStaticConfigList[i].specifics.ds2438.config, PNULL);
                    break;
                case OW_TYPE_DS2408_PIO:
                    /* Disable test mode, just in case, then write the control register and
                     * the pin configuration, using an intermediate variable for the latter
                     * as the write function also reads the result back and I'd rather avoid
                     * my global data structure being modified */
                    success = disableTestModeDS2408 (gPortNumber, pAddress);
                    if (success)
                    {
                        success = writeControlRegisterDS2408 (gPortNumber, pAddress, gDeviceStaticConfigList[i].specifics.ds2408.config);
                        if (success)
                        {
                            pinsState = gDeviceStaticConfigList[i].specifics.ds2408.pinsState;                     
                            success = channelAccessWriteDS2408 (gPortNumber, pAddress, &pinsState);
                        }
                    }
                    break;
                default:
                    ASSERT_ALWAYS_PARAM (pAddress[0]);
                    success = false;
                    break;
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
    for (i = 0; i < MAX_NUM_DEVICES; i++)
    {
        if (!found[i])
        {
            printProgress ("Couldn't find %s, address: ", deviceNameList[i]);
            printAddress (&gDeviceStaticConfigList[i].address.value[0], true);
            success = false;
        }
    }

    return success;
}

/*
 * Toggle a pin or pins from their current state to the
 * reverse and back again. 
 * 
 * deviceName  the PIO device that the pins belong to.
 * pinsMask    the pins to be toggled (a bit set to 1 is
 *             to be toggled a bit set to 0 is left alone). 
 *
 * @return  true if successful, otherwise false.
 */
static Bool togglePins (OwDeviceName deviceName, UInt8 pinsMask)
{
    Bool  success = true;
    UInt8 pinsState;
    UInt8 i;
    
    /* Read the last state of the pins */
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], &pinsState);
    
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
        success = channelAccessWriteDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], &pinsState);
        msDelay (TOGGLE_DELAY_MS);
    }
    
    return success;
}

/*
 * Set a pin or pins to on (i.e. 5 Volts) or off (i.e. ground). 
 * 
 * deviceName  the PIO device that the pins belong to.
 * pinMask     the pins to be set to 5 Volts or ground. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setPins (OwDeviceName deviceName, UInt8 pinMask, Bool setPinsTo5Volts)
{
    Bool  success = true;
    UInt8 pinsState;
    
    /* Read the last state of the pins */
    success = readPIOLogicStateDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], &pinsState);
    
    /* Set or reset the ones masked in */
    if (setPinsTo5Volts)
    {
        pinsState |= pinMask;
    }
    else
    {
        pinsState &=~ pinMask;
    }
    success = channelAccessWriteDS2408 (gPortNumber, &gDeviceStaticConfigList[deviceName].address.value[0], &pinsState);
    
    return success;
}

/*
 * Switch the Orangutan power relay from it's current 
 * state to the reverse and back again.
 *
 * @return  true if successful, otherwise false.
 */
static Bool toggleOPwr (void)
{
    return togglePins (OW_NAME_SCHOTTKY_PIO, SCHOTTKY_O_PWR_TOGGLE);
}

/*
 * Switch the Orangutan reset relay from it's current 
 * state to the reverse and back again.
 *
 * @return  true if successful, otherwise false.
 */
static Bool toggleORst (void)
{
    return togglePins (OW_NAME_SCHOTTKY_PIO, SCHOTTKY_O_RESET_TOGGLE);
}

/*
 * Switch the Raspberry Pi reset relay from it's current 
 * state to the reverse and back again.
 *
 * @return  true if successful, otherwise false.
 */
static Bool togglePiRst (void)
{
    return togglePins (OW_NAME_SCHOTTKY_PIO, SCHOTTKY_PI_RESET_TOGGLE);
}

/*
 * Switch the relay that allows 12 Volts to be supplied
 * to the RIO to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwr12VOn (void)
{
    return setPins (OW_NAME_SCHOTTKY_PIO, SCHOTTKY_RIO_PWR_12V_ON, true);
}

/*
 * Switch the relay that allows 12 Volts to be
 * supplied to the RIO to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwr12VOff (void)
{
    return setPins (OW_NAME_SCHOTTKY_PIO, SCHOTTKY_RIO_PWR_12V_ON, false);
}

/*
 * Switch the relay that allows on-board battery
 * power to be supplied to the RIO to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwrBattOn (void)
{
    return setPins (OW_NAME_SCHOTTKY_PIO, SCHOTTKY_RIO_PWR_BATT_OFF, false);
}

/*
 * Switch the relay that allows on-board battery
 * power to be supplied to the RIO to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwrBattOff (void)
{
    return setPins (OW_NAME_SCHOTTKY_PIO, SCHOTTKY_RIO_PWR_BATT_OFF, true);
}

/*
 * Switch the relay that allows 12 Volts to be
 * supplied to the Orangutan to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwr12VOn (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O_PWR_12V_ON, true);
}

/*
 * Switch the relay that allows 12 Volts to be
 * supplied to the Orangutan to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwr12VOff (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O_PWR_12V_ON, false);
}

/*
 * Switch the relay that allows on-board battery
 * power to be supplied to the Orangutan to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwrBattOn (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O_PWR_BATT_OFF, false);
}

/*
 * Switch the relay that allows battery power to be
 * supplied to the Orangutan to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwrBattOff (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O_PWR_BATT_OFF, true);
}

/*
 * Switch the RIO battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioBatteryChargerOn (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_RIO_CHARGER_ON, true);
}

/*
 * Switch the RIO battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioBatteryChargerOff (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_RIO_CHARGER_ON, false);
}

/*
 * Switch the O1 battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO1BatteryChargerOn (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O1_CHARGER_ON, true);
}

/*
 * Switch the O1 battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO1BatteryChargerOff (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O1_CHARGER_ON, false);
}

/*
 * Switch the O2 battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO2BatteryChargerOn (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O2_CHARGER_ON, true);
}

/*
 * Switch the O2 battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO2BatteryChargerOff (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O2_CHARGER_ON, false);
}

/*
 * Switch the O3 battery charger to ON.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO3BatteryChargerOn (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O3_CHARGER_ON, true);
}

/*
 * Switch the O3 battery charger to OFF.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO3BatteryChargerOff (void)
{
    return setPins (OW_NAME_RELAY_PIO, RELAY_O3_CHARGER_ON, false);
}

/*
 * Read the current being drawn from the Rio/Pi/5V battery.
 *
 * pCurrent  a pointer to somewhere to put the current reading.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool readRioBattCurrent (SInt16 *pCurrent)
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
static Bool readO1BattCurrent (SInt16 *pCurrent)
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
static Bool readO2BattCurrent (SInt16 *pCurrent)
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
static Bool readO3BattCurrent (SInt16 *pCurrent)
{
    return readCurrentDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], pCurrent);
}

/*
 * Calibrate the Rio/Pi/5V battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool performCalRioBattMonitor (void)
{
    printProgress ("WARNING: calibrating RIO battery, make sure no RIO/PI current is flowing!\n");
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_RIO_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * Calibrate the O1 battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool performCalO1BattMonitor (void)
{
    printProgress ("WARNING: calibrating O1 battery, make sure no current is flowing!\n");
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O1_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * Calibrate the O2 battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool performCalO2BattMonitor (void)
{
    printProgress ("WARNING: calibrating O2 battery, make sure no current is flowing!\n");
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O2_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * Calibrate the O3 battery monitor.
 * This shold ONLY be called when the there is no
 * current being drawn from the battery.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool performCalO3BattMonitor (void)
{
    printProgress ("WARNING: calibrating O3 battery, make sure no current is flowing!\n");
    return performCalDS2438 (gPortNumber, &gDeviceStaticConfigList[OW_NAME_O3_BATTERY_MONITOR].address.value[0], PNULL);
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool  success = true;
    
    /* Setup what's necessary for OneWire bus stuff */
    success = startOneWireBus (ONEWIRE_PORT);
    
    /* Find and setup the devices on the OneWire bus */
    if (success)
    {
        success = setupDevices();
        
        if (success)
        {            
            SInt16 rioCurrent;
            SInt16 o1Current;
            SInt16 o2Current;
            SInt16 o3Current;            
            
            while (!key_abort() && success)
            {
                success = readRioBattCurrent (&rioCurrent);
                if (success)
                {
                    success = readO1BattCurrent (&o1Current);
                    if (success)
                    {
                        success = readO2BattCurrent (&o2Current);
                        if (success)
                        {
                            success = readO3BattCurrent (&o3Current);
                        }
                    }
                }
                if (success)
                {
                    printf ("Currents: RIO %4d O1 %4d, O2 %4d, O3 %4d (-ve means discharge)\n", rioCurrent, o1Current, o2Current, o3Current);                
                }
            }
        }
        else
        {
            /* If the setup fails, print out what devices we can find */
            findAllDevices();
        }
    }
    
    if (success)
    {
        printProgress ("Done.\n");
    }
    else
    {
        printProgress ("Failed!\n");
    }
    
    /* Shut things down gracefully */
    stopOneWireBus();
    
    return success;
}