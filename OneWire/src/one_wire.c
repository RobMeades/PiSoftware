/*
 * OneWire.c
 * OneWire functions for RoboOne
 */ 

#include <stdio.h>
#include <ownet.h>
#include <atod26.h>
#include <findtype.h>
#include <rob_system.h>
#include <one_wire.h>

#define MAXDEVICES 5

/*
 * main for testing
 */
int main (int argc, char **argv)
{
    Bool success = true;
    UInt8 i;
    UInt8 x;
    UInt8 portNumber = 0;
    UInt8 numBatteryDevices;
    UInt8 batteryDeviceArray[MAXDEVICES][NUM_BYTES_IN_SERIAL_NUM];
    UInt8 pageBuffer[DS4238_NUM_BYTES_IN_PAGE];
    SInt16 current;
    double temperature;
    UInt16 voltage;

    if (argc == 2)
    {
        portNumber = owAcquireEx (argv[1]);
    }
    else
    {
        portNumber = owAcquireEx (ONEWIRE_PORT);
    }

    if (portNumber < 0)
    {
        printf ("Failed to acquire port.\n");
        success = false;
    }
    else
    {  
        do
        {
            numBatteryDevices = FindDevices (portNumber, &batteryDeviceArray[0], SBATTERY_FAM, MAXDEVICES);
    
            if (numBatteryDevices == 0)
            {
                success = false;
                ASSERT_ALWAYS_STRING ("No battery monitoring devices found.");
            }
            else
            {
                /* Before we start check that the configurations are setup correctly and if
                 * not write in the values we need and flush them to NV so that they will
                 * be correct next time */
                for (i = 0; (i < numBatteryDevices) && success; i++)
                {
                    success = readNVPageDS2438 (portNumber, &batteryDeviceArray[i][0], DS2438_CONFIG_PAGE, &pageBuffer[0]);
                    if (success &&
                        ((pageBuffer[DS2438_CONFIG_REG_OFFSET] | DS2438_IAD_IS_ENABLED) == 0 ||
                         (pageBuffer[DS2438_CONFIG_REG_OFFSET] | DS2438_CA_IS_ENABLED) == 0 ||
                         (pageBuffer[DS2438_CONFIG_REG_OFFSET] | DS2438_EE_IS_ENABLED) == 0))
                    {
                        pageBuffer[DS2438_CONFIG_REG_OFFSET] = DS2438_IAD_IS_ENABLED | DS2438_CA_IS_ENABLED | DS2438_EE_IS_ENABLED;
                        success = writeNVPageDS2438 (portNumber, &batteryDeviceArray[i][0], DS2438_CONFIG_PAGE, &pageBuffer[DS2438_CONFIG_REG_OFFSET], DS2438_CONFIG_REG_SIZE);                        
                    }
                }
                /* Now read stuff */
                for (i = 0; (i < numBatteryDevices) && success; i++)
                {
                    printf("Device %d\n", i);                    
                    success = readVddDS2438 (portNumber, &batteryDeviceArray[i][0], &voltage);
                    if (success)
                    {
                        printf(" Vdd was:  %d mV\n", voltage);
                        success = readVadDS2438 (portNumber, &batteryDeviceArray[i][0], &voltage);
                        if (success)
                        {
                            printf(" Vad was:  %d mV\n", voltage);
                            success = readTemperatureDS2438 (portNumber, &batteryDeviceArray[i][0], &temperature);
                            if (success)
                            {
                                printf(" Temperature was:  %2.2f C\n", temperature);
                                success = readSPPageDS2438 (portNumber, &batteryDeviceArray[i][0], DS2438_CONFIG_PAGE, &pageBuffer[0]);
                                if (success)
                                {
                                    current = pageBuffer[DS2438_CURRENT_REG_OFFSET] + 0x100 * (pageBuffer[DS2438_CURRENT_REG_OFFSET + 1] & 0x03);
                                    /* From the DS2438 data sheet I (in Amps) = Current Register / (4096 * RSENS) */
                                    current = current * 1000 / RSENS_TIMES_4096;
                                    if (pageBuffer[DS2438_CURRENT_REG_OFFSET + 1] & 0x80)
                                    {
                                        printf(" Discharge current was:  %d ma\n", current);
                                    }
                                    else
                                    {
                                        printf(" Charge current was:  %d ma\n", current);                           
                                    }
                                }
                            }
                            for (x = 0; (x < DS4238_NUM_PAGES) && success; x++)
                            {
                                success = readSPPageDS2438 (portNumber, &batteryDeviceArray[i][0], x, &pageBuffer[0]);
                                printf ("Page %d contents: %2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x\n", x, pageBuffer[0], pageBuffer[1], pageBuffer[2], pageBuffer[3], pageBuffer[4], pageBuffer[5], pageBuffer[6], pageBuffer[7]);
                            }
                        }
                    }
                    
                    printf("\n");
                } /* for loop */
            }
        } while (!key_abort() && success);
    
        owRelease (portNumber);
        printf ("Port released.\n");    
    }

    return success;
}
