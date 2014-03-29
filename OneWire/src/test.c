/*
 * test.c
 * Test all the OneWire functions
 */ 
 
#include <stdio.h>
#include <ownet.h>
#include <atod26.h>
#include <findtype.h>
#include <rob_system.h>
#include <one_wire.h>

#define MAX_BATTERY_DEVICES 5
#define MAX_IO_DEVICES 5
#define ONEWIRE_PORT "/dev/USBSerial"

/*
 * main for testing
 */
int main (int argc, char **argv)
{
    Bool success = true;
    UInt8 i;
    UInt8 x;
    SInt32 portNumber = 0;
    UInt8 numBatteryDevices;
    UInt8 numIODevices;
    UInt8 batteryDeviceArray[MAX_BATTERY_DEVICES][NUM_BYTES_IN_SERIAL_NUM];
    UInt8 ioDeviceArray[MAX_IO_DEVICES][NUM_BYTES_IN_SERIAL_NUM];
    UInt8 pageBuffer[DS2408_MAX_BYTES_TO_READ];
    SInt16 current;
    double temperature;
    UInt16 voltage;
    SInt16 offsetCalibration;

    if (argc == 2)
    {
        portNumber = robStartOneWireBus (argv[1]);
    }
    else
    {
        portNumber = robStartOneWireBus (ONEWIRE_PORT);
    }

    if (portNumber < 0)
    {
        printf ("Failed to acquire port.\n");
        success = false;
    }
    else
    {  
        numBatteryDevices = FindDevices (portNumber, &batteryDeviceArray[0], SBATTERY_FAM, MAX_BATTERY_DEVICES);
        numIODevices = FindDevices (portNumber, &ioDeviceArray[0], PIO_FAM, MAX_IO_DEVICES);

        if (numBatteryDevices == 0)
        {
            success = false;
            ASSERT_ALWAYS_STRING ("No battery monitoring devices found.\n");
        }
        else
        {
            printf ("Found %d battery monitoring devices.\n", numBatteryDevices);
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
        
                    success = writeNVPageDS2438 (portNumber, &batteryDeviceArray[i][0], 2, &pageBuffer[0], DS4238_NUM_BYTES_IN_PAGE);
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
            /* Check that the configurations are setup correctly and if
             * not write in the values we need and flush them to NV so
             * that they will be correct next time */
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
        } /* numBatteryDevices > 0 */
        
        if (numIODevices == 0)
        {
            success = false;
            ASSERT_ALWAYS_STRING ("No IO devices found.\n");
        }
        else
        {
            printf ("Found %d IO devices.\n", numIODevices);
            for (i = 0; (i < numIODevices) && success; i++)
            {              
                UInt8 controlByte = 0;
                
                printf ("Disable test mode.\n");
                success = disableTestModeDS2408 (portNumber, &ioDeviceArray[i][0]);
                if (success)
                {
                    printf ("Reset activity latches.\n");
                    success = resetActivityLatchesDS2408 (portNumber, &ioDeviceArray[i][0]);
                    if (success)
                    {
                        controlByte &= ~(DS2408_DEVICE_HAS_POWER_ON_RESET | DS2408_RSTZ_IS_STROBE);
                        controlByte |= (DS2408_SEARCH_IS_AND | DS2408_SEARCH_IS_ACTIVITY_LATCHED);
                        printf ("Setup the control registers.\n");
                        success = writeControlRegisterDS2408 (portNumber, &ioDeviceArray[i][0], controlByte);
                        if (success)
                        {
                            printf ("Setup the conditional search channel selection mask registers.\n");
                            success = writeCSChannelSelectionMaskRegisterDS2408 (portNumber, &ioDeviceArray[i][0], 0x18);                        
                            if (success)
                            {
                                printf ("Setup the conditional search channel priority selection registers.\n");
                                success = writeCSChannelPolaritySelectionRegisterDS2408 (portNumber, &ioDeviceArray[i][0], 0x28);                        
                                if (success)
                                {
                                    UInt8 outputByte = 0x80;
                                    
                                    printf ("Set P7 of each device to be high.\n");
                                    success = channelAccessWriteDS2408 (portNumber, &ioDeviceArray[i][0], &outputByte);                        
                                }
                            }
                        }
                    }
                }
            }
        }  /* numIODevices > 0 */
                
        if (success)
        {
            do
            {
                printf("---- IO Devices ---- \n");
                for (i = 0; (i < numIODevices) && success; i++)
                {
                    printf ("Device %d, address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", i, ioDeviceArray[i][0], ioDeviceArray[i][1], ioDeviceArray[i][2], ioDeviceArray[i][3], ioDeviceArray[i][4], ioDeviceArray[i][5], ioDeviceArray[i][6], ioDeviceArray[i][7]);
                    success = readControlRegisterDS2408 (portNumber, &ioDeviceArray[i][0], &pageBuffer[0]);
                    if (success)
                    {
                        printf (" status reg: 0x%.2x", pageBuffer[0]);
                        success = readPIOLogicStateDS2408 (portNumber, &ioDeviceArray[i][0], &pageBuffer[0]);
                        if (success)
                        {
                            printf (", IO reg: 0x%.2x", pageBuffer[0]);
                            success = readPIOOutputLatchStateRegisterDS2408 (portNumber, &ioDeviceArray[i][0], &pageBuffer[0]);
                            if (success)
                            {
                                UInt8 bytesRead;
                                
                                printf (", IO O/P latch reg: 0x%.2x", pageBuffer[0]);
                                bytesRead = channelAccessReadDS2408 (portNumber, &ioDeviceArray[i][0], &pageBuffer[0], DS2408_MAX_BYTES_TO_READ);                                    
                                if (bytesRead > 0)
                                {
                                    printf (", IO %d times: ", bytesRead);
                                    for (x = 0; x < bytesRead; x++)
                                    {
                                        printf (" 0x%.2x", pageBuffer[x]);                                            
                                    }
                                    success = readCSChannelSelectionMaskRegisterDS2408 (portNumber, &ioDeviceArray[i][0], &pageBuffer[0]);
                                    if (success)
                                    {
                                        printf (", CS select reg: 0x%.2x", pageBuffer[0]);
                                        success = readCSChannelPolaritySelectionRegisterDS2408 (portNumber, &ioDeviceArray[i][0], &pageBuffer[0]);
                                        if (success)
                                        {
                                            printf (", CS polarity reg: 0x%.2x.\n", pageBuffer[0]);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    printf("\n");
                } /* IO Devices for loop */
                
                printf("---- Battery Devices ---- \n");
                for (i = 0; (i < numBatteryDevices) && success; i++)
                {
                    printf ("Device %d, address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", i, batteryDeviceArray[i][0], batteryDeviceArray[i][1], batteryDeviceArray[i][2], batteryDeviceArray[i][3], batteryDeviceArray[i][4], batteryDeviceArray[i][5], batteryDeviceArray[i][6], batteryDeviceArray[i][7]);
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
                    if (success)
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
                            printf ("Elapsed time: %lu secs, remaining capacity: %u mAHr, cal. offset: %d\n", elapsedTime, remainingCapacity, offsetCalibration);
                            success = readTimePiOffChargingStoppedDS2438 (portNumber, &batteryDeviceArray[i][0], &piOff, &chargingStopped);
                            if (success)
                            {
                                printf ("Pi last switched off: %lu secs, charging last stopped: %lu secs\n", piOff, chargingStopped);                                
                                success = readNVChargeDischargeDS2438 (portNumber, &batteryDeviceArray[i][0], &accumulatedCharge, &accumulatedDischarge);
                                if (success)
                                {
                                    printf ("Accumulated charge: %lu mAHr, accumulated discharge %lu mAHr\n", accumulatedCharge, accumulatedDischarge);
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
                } /* Battery devices for loop */
            } while (!key_abort() && success);
        }
        
        if (!success)
        {
            printf ("Either something returned false or at least one IO device and at least one Battery device were not found.\n");
        }
    
        robStopOneWireBus (portNumber);
        printf ("Port released.\n");    
    }

    return success;
}
