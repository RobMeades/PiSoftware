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
    SInt16 offsetCalibration;

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
        numBatteryDevices = FindDevices (portNumber, &batteryDeviceArray[0], SBATTERY_FAM, MAXDEVICES);

        if (numBatteryDevices == 0)
        {
            success = false;
            ASSERT_ALWAYS_STRING ("No battery monitoring devices found.\n");
        }
        else
        {
            for (i = 0; (i < numBatteryDevices) && success; i++)
            {
                UInt32 longOne1 = 0xFFFFFF01;
                UInt32 longOne2 = 0xFFFFFF02;
                UInt16 capacity = 1000;
                
                
                printf ("Writing time %lu and capacity %d.\n", longOne1, capacity);
                success = writeTimeCapacityDS2438 (portNumber, &batteryDeviceArray[i][0], &longOne1, &capacity);
                if (success)
                {
                    printf ("Writing something to DISC/EOC page.\n");
                    pageBuffer[0] = 0xFF;
                    pageBuffer[1] = 0xFF;
                    pageBuffer[2] = 0xFF;
                    pageBuffer[3] = 0xFF;
                    pageBuffer[4] = 0xFF;
                    pageBuffer[5] = 0xFF;
                    pageBuffer[6] = 0xFF;
                    pageBuffer[7] = 0xFF;
        
                    success = writeNVPageDS2438 (portNumber, &batteryDeviceArray[i][0], 2, &pageBuffer[0], sizeof (pageBuffer));
                    if (success)
                    {
                        printf ("Writing something to CCA and DCA registers.\n");
                        success = writeNVChargeDischargeDS2438 (portNumber, &batteryDeviceArray[i][0], &longOne1, &longOne2);
                        
                        if (success)
                        {
                            printf ("Writing something to user data pages.\n");
                            for (x = 0; (x < DS2438_NUM_USER_DATA_PAGES) && success; x++)
                            {
                                pageBuffer[0] = x + 1;
                                pageBuffer[1] = x + 2;
                                pageBuffer[2] = x + 3;
                                pageBuffer[3] = x + 4;
                                pageBuffer[4] = x + 5;
                                pageBuffer[5] = x + 6;
                                pageBuffer[6] = x + 7;
                                pageBuffer[7] = x + 8;
                                success = writeNVUserDataDS2438 (portNumber, &batteryDeviceArray[i][0], x, &pageBuffer[0], (x + 1) * 2);
                            }
                            
                            if (success)
                            {
#if 0
                                printf ("Calibrating: MAKE SURE NO CURRENT GOING THROUGH RSENS!\n");
                                success = performCalDS2438 (portNumber, &batteryDeviceArray[i][0], &offsetCalibration);
                                printf ("New offset %d.\n", offsetCalibration);
#endif                                
                            }
                        }
                    }
                }
            }
            
            do
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
                                success = readNVPageDS2438 (portNumber, &batteryDeviceArray[i][0], x, &pageBuffer[0]);
                                printf ("Page %d contents: %2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x\n", x, pageBuffer[0], pageBuffer[1], pageBuffer[2], pageBuffer[3], pageBuffer[4], pageBuffer[5], pageBuffer[6], pageBuffer[7]);
                            }
                        }
                    }
                    
                    /* Check other data */
                    for (i = 0; (i < numBatteryDevices) && success; i++)
                    {
                        UInt32 elapsedTime;
                        UInt16 remainingCapacity;
                        UInt32 piOff;
                        UInt32 chargingStopped;
                        UInt32 accumulatedCharge;
                        UInt32 accumulatedDischarge;
                        
                        success = readTimeCapacityCalDS2438 (portNumber, &batteryDeviceArray[i][0], &elapsedTime, &remainingCapacity, &offsetCalibration);
                        if (success)
                        {
                            printf ("Other: elapsed time: %lu secs, remaining capacity: %u mA hours, cal. offset: %d\n", elapsedTime, remainingCapacity, offsetCalibration);
                            success = readTimePiOffChargingStoppedDS2438 (portNumber, &batteryDeviceArray[i][0], &piOff, &chargingStopped);
                            if (success)
                            {
                                printf ("       Pi last switched off: %lu secs, charging last stopped: %lu secs\n", piOff, chargingStopped);                                
                                success = readNVChargeDischargeDS2438 (portNumber, &batteryDeviceArray[i][0], &accumulatedCharge, &accumulatedDischarge);
                                if (success)
                                {
                                    printf ("       accumulated charge: %lu mAHr, accumulated discharge %lu mAHr\n", accumulatedCharge, accumulatedDischarge);
                                    for (x = 0; (x < DS2438_NUM_USER_DATA_PAGES) && success; x++)
                                    {
                                        success = readNVUserDataDS2438 (portNumber, &batteryDeviceArray[i][0], x, &pageBuffer[0]);
                                        if (success)
                                        {
                                            printf ("UserData %d contents: %2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x\n", x, pageBuffer[0], pageBuffer[1], pageBuffer[2], pageBuffer[3], pageBuffer[4], pageBuffer[5], pageBuffer[6], pageBuffer[7]);
                                        }
                                    }
                                }
                            }                            
                        }
                    }
                    
                    printf("\n");
                } /* for loop */
            } while (!key_abort() && success);
        }
    
        owRelease (portNumber);
        printf ("Port released.\n");    
    }

    return success;
}
