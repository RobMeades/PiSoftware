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
    UInt8 pageBuffer[8];
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
                    UInt8 config;
                    
                    success = readNVConfigThresholdDS2438 (portNumber, &batteryDeviceArray[i][0], &config, PNULL);
                    if (success &&
                        ((config | DS2438_IAD_IS_ENABLED) == 0 ||
                         (config | DS2438_CA_IS_ENABLED) == 0 ||
                         (config | DS2438_EE_IS_ENABLED) == 0))
                    {
                        config = DS2438_IAD_IS_ENABLED | DS2438_CA_IS_ENABLED | DS2438_EE_IS_ENABLED;
                        success = writeNVConfigThresholdDS2438 (portNumber, &batteryDeviceArray[i][0], &config, PNULL);                        
                    }
                }
                
                /* Now read stuff */
                for (i = 0; (i < numBatteryDevices) && success; i++)
                {
                    printf ("Device %d\n", i);                    
                    success = readVddDS2438 (portNumber, &batteryDeviceArray[i][0], &voltage);
                    if (success)
                    {
                        printf (" Vdd was:  %d mV\n", voltage);
                        success = readVadDS2438 (portNumber, &batteryDeviceArray[i][0], &voltage);
                        if (success)
                        {
                            printf (" Vad was:  %d mV\n", voltage);
                            success = readTemperatureDS2438 (portNumber, &batteryDeviceArray[i][0], &temperature);
                            if (success)
                            {
                                printf (" Temperature was:  %2.2f C\n", temperature);
                                success = readCurrentDS2438 (portNumber, &batteryDeviceArray[i][0], &current);
                                if (success)
                                {
                                    printf (" Current was:  %d ma (-ve means discharge)\n", current);
                                    success = readBatteryDS2438 (portNumber, &batteryDeviceArray[i][0], &voltage, &current);
                                    if (success)
                                    {
                                        printf ("Battery was %2.2f V, %1.3f A, %2.3f W\n", (double) voltage/1000, (double) current/1000, ((double) (voltage *  current * -1)) / 1000000);
                                    }
                                }
                            }
                            for (x = 0; (x < 8) && success; x++)
                            {
                                success = readSPPageDS2438 (portNumber, &batteryDeviceArray[i][0], x, &pageBuffer[0]);
                                printf ("Page %d contents: %2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x\n", x, pageBuffer[0], pageBuffer[1], pageBuffer[2], pageBuffer[3], pageBuffer[4], pageBuffer[5], pageBuffer[6], pageBuffer[7]);
                            }
                        }
                    }
                    
                    /* Check other config data */
                    for (i = 0; (i < numBatteryDevices) && success; i++)
                    {
                        UInt32 elapsedTime;
                        UInt16 remainingCapacity;
                        SInt16 offsetCalibration;
                        UInt32 piOff;
                        UInt32 chargingStopped;
                        UInt32 accumulatedCharge;
                        UInt32 accumulatedDischarge;
                        
                        success = readTimeCapacityOffsetDS2438 (portNumber, &batteryDeviceArray[i][0], &elapsedTime, &remainingCapacity, &offsetCalibration);
                        if (success)
                        {
                            printf ("Other: elapsed time: %lu secs, remaining capacity: %u mA hours, cal. offset: %d\n", elapsedTime, remainingCapacity, offsetCalibration);
                            success = readTimePiOffChargingStoppedDS2438 (portNumber, &batteryDeviceArray[i][0], &piOff, &chargingStopped);
                            if (success)
                            {
                                printf ("      Pi last switched off: %lu secs, charging last stopped: %lu secs\n", piOff, chargingStopped);                                
                                success = readChargeDischargeDS2438 (portNumber, &batteryDeviceArray[i][0], &accumulatedCharge, &accumulatedDischarge);
                                if (success)
                                {
                                    printf ("      accumulated charge: %lu mA, accumulated discharge %lu mA\n", accumulatedCharge, accumulatedDischarge);                                    
                                }
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
