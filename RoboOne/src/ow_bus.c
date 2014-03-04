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

#define MAX_NUM_DEVICES 8 /* This MUST be the same as the number of elements in the gDevicesStaticConfig[] below */

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

/* All pins low to begin with */
#define CHARGER_STATE_IO_PIN_CONFIG   0x00
#define SCHOTTKY_IO_PIN_CONFIG        0x00
#define RELAY_IO_PIN_CONFIG           0x00
#define GENERAL_PURPOSE_IO_PIN_CONFIG 0x00

/*
 * TYPES
 */

/* The names of all the one wire devices on RoboOne */
typedef enum OwDeviceNameTag
{
    OW_NAME_NULL,
    OW_NAME_RIO_BATTERY_MONITOR,
    OW_NAME_O1_BATTERY_MONITOR,
    OW_NAME_O2_BATTERY_MONITOR,
    OW_NAME_O3_BATTERY_MONITOR,
    OW_NAME_CHARGER_STATE_IO,
    OW_NAME_SCHOTTKY_IO,
    OW_NAME_RELAY_IO,
    OW_NAME_GENERAL_PURPOSE_IO,
    OW_NUM_NAMES
} OwDeviceName;

/* The possible OneWire device types,
 * ORDER IS IMPORTANT (may be used to index into the OwDeviceSpecifics union) */
typedef enum OwDeviceTypeTag
{
    OW_TYPE_DS2408_PIO,
    OW_TYPE_DS2438_BATTERY_MONITOR,
    OW_TYPE_UNKNOWN,
    OW_NUM_TYPES
} OwDeviceType;

/* Device specific information for the DS24o8 PIO OneWire device */
typedef struct OwDS2408Tag
{
    UInt8 intendedConfig;
    UInt8 intendedPinConfig;
} OwDS2408;

/* Device specific information for the DS2438 battery monitoring OneWire device */
typedef struct OwDS2438Tag
{
    UInt8 intendedConfig;
} OwDS2438;

/* Union of all the possible OneWire device specific information,
 * ORDER IS IMPORTANT (may be indexed by the OwDeviceType enum) */
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

/* TODO: check that these are the correct way around and put in the specific values */
/* Would love this to be const but we call into library functions that don't promise constness */
OwDevicesStaticConfig gDeviceStaticConfigList[] =
         {{OW_NAME_RIO_BATTERY_MONITOR, {{SBATTERY_FAM, 0x84, 0x0d, 0xb3, 0x01, 0x00, 0x00, 0x09}}, {{RIO_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O1_BATTERY_MONITOR, {{SBATTERY_FAM, 0x82, 0x30, 0xb3, 0x01, 0x00, 0x00, 0xd3}}, {{O1_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O2_BATTERY_MONITOR, {{SBATTERY_FAM, 0xb5, 0x02, 0xb3, 0x01, 0x00, 0x00, 0xbc}}, {{O2_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_O3_BATTERY_MONITOR, {{SBATTERY_FAM, 0xdd, 0x29, 0xb3, 0x01, 0x00, 0x00, 0x56}}, {{O3_BATTERY_MONITOR_CONFIG}}},
          {OW_NAME_CHARGER_STATE_IO, {{PIO_FAM, 0x50, 0x64, 0x0d, 0x00, 0x00, 0x00, 0x9e}}, {{CHARGER_STATE_IO_CONFIG, CHARGER_STATE_IO_PIN_CONFIG}}},
          {OW_NAME_SCHOTTKY_IO, {{PIO_FAM, 0x5e, 0x64, 0x0d, 0x00, 0x00, 0x00, 0x8d}}, {{SCHOTTKY_IO_CONFIG, SCHOTTKY_IO_PIN_CONFIG}}},
          {OW_NAME_RELAY_IO, {{PIO_FAM, 0x8d, 0xf2, 0x0c, 0x00, 0x00, 0x00, 0xb4}}, {{RELAY_IO_CONFIG, RELAY_IO_PIN_CONFIG}}},
          {OW_NAME_GENERAL_PURPOSE_IO, {{PIO_FAM, 0x7f, 0x6e, 0x0d, 0x00, 0x00, 0x00, 0xb1}}, {{GENERAL_PURPOSE_IO_CONFIG, GENERAL_PURPOSE_IO_PIN_CONFIG}}}};

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
 * - Open the serial port for OneWire comms
 * - Initialise global variables
 * 
 * pPort   pointer to a string that represents
 *         the port to use for serial comms.
 *
 * @return  true if successful.
 */
static Bool initOneWireBus (Char * pPort)
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
 * gDevicesStaticConfig[].
 *
 * @return  true if successful, otherwise false.
 */
static Bool setupDevices (void)
{
    Bool  success = true;
    Bool  found[MAX_NUM_DEVICES];
    UInt8 *pAddress;
    UInt8 intendedPinConfig;
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
                    success = writeNVConfigThresholdDS2438 (gPortNumber, pAddress, &gDeviceStaticConfigList[i].specifics.ds2438.intendedConfig, PNULL);
                    break;
                case OW_TYPE_DS2408_PIO:
                    /* Write the control register and the pin configuration, using an intermediate variable for the latter
                     * as the write function also reads the result back and I'd rather avoid my global data structure being modified */
                    success = writeControlRegisterDS2408 (gPortNumber, pAddress, gDeviceStaticConfigList[i].specifics.ds2408.intendedConfig);
                    if (success)
                    {
                        intendedPinConfig = gDeviceStaticConfigList[i].specifics.ds2408.intendedPinConfig;                     
                        success = channelAccessWriteDS2408 (gPortNumber, pAddress, &intendedPinConfig);
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
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool  success = true;
    
    /* Setup what's necessary for OneWire bus stuff */
    success = initOneWireBus (ONEWIRE_PORT);
    
    /* Find and setup the devices on the OneWire bus */
    if (success)
    {
        success = setupDevices ();
        
        /* If the setup fails, print out what devices we can find */
        if (!success)
        {
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
    
    return success;
}
