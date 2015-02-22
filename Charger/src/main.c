/*
 * Main for Charger.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ownet.h>
#include <atod26.h>
#include <findtype.h>
 
#include <curses.h>

#include <unistd.h>

#include <rob_system.h>
#include <one_wire.h>
#include <pi_io.h>
#include <main.h>

/*
 * MANIFEST CONSTANTS
 */

/* The serial port that the One Wire devices are attached to */
#define ONEWIRE_PORT "/dev/USBSerial"

/* The maximum number of battery monitoring devices on the One Wire bus */
#define MAX_BATTERY_DEVICES 1

/* The maximum number of IO devices on the One Wire bus */
#define MAX_IO_DEVICES 1

/* Convert a microstep to an angle in degrees, rounding down */
#define MICROSTEPS_TO_ANGLE(m) (m * 360 /1600)

/* Convert an angle to microsteps, rounding down */
#define ANGLE_TO_MICROSTEPS(a) (a * 1600 / 360)

/* The minimum angle to move (chose a multiple of 0.225) */
#define MIN_ANGLE_DEGREES 9
#define MIN_ANGLE_MICROSTEPS ANGLE_TO_MICROSTEPS (MIN_ANGLE_DEGREES)

/* The minimum number of readings to decide on a move */
#define MIN_READINGS 3

/* The maximum angle to move to, clockwise or anti-clockwise
 * (chose a multiple of 0.225) */
#define LIMIT_ANGLE_DEGREES 90
#define LIMIT_ANGLE_MICROSTEPS ANGLE_TO_MICROSTEPS (LIMIT_ANGLE_DEGREES)

/*
 * GLOBALS (prefixed with g)
 */
 
SInt32 gPortNumber = 0;
UInt8 gBatteryDeviceArray[MAX_BATTERY_DEVICES][NUM_BYTES_IN_SERIAL_NUM];
UInt8 gIoDeviceArray[MAX_IO_DEVICES][NUM_BYTES_IN_SERIAL_NUM];
SInt16 gCurrent;
double gTemperature;
UInt16 gVoltage;
SInt16 gOffsetCalibration;

/*
 * STATIC FUNCTIONS
 */


/*
 * Set the GPIOs on the PI to the correct initial state.   
 */
static void setupPiGpios(void)
{    
    GPIO_CONFIG_OUTPUT (GPIO_IR_ENABLE);
    GPIO_CONFIG_OUTPUT (GPIO_MOTOR_MICRO_STEP);
    GPIO_CONFIG_OUTPUT (GPIO_MOTOR_DIRECTION);
    GPIO_CONFIG_OUTPUT (GPIO_MOTOR_ENABLE_BAR);

    GPIO_SET (GPIO_IR_ENABLE);
    GPIO_SET (GPIO_MOTOR_ENABLE_BAR);
}

/*
 * Setup the OneWire IO device.  
 * the OneWire bus, using the configuration data in
 * gDeviceStaticConfigList[].
 *
 * pAddress  the address of the IO device.
 *
 * @return  true if successful, otherwise false.
 */
static Bool setupIoDevice(UInt8 *pAddress)
{
    Bool success;

    printProgress ("Disabling test mode...\n");
    success = disableTestModeDS2408 (gPortNumber, pAddress);

    if (success)
    {
        printProgress ("Resetting activity latches...\n");
        success = resetActivityLatchesDS2408 (gPortNumber, pAddress);
        if (success)
        {
            UInt8 controlByte = 0;

            controlByte &= ~(DS2408_DEVICE_HAS_POWER_ON_RESET | DS2408_RSTZ_IS_STROBE);
            controlByte |= (DS2408_SEARCH_IS_AND | DS2408_SEARCH_IS_ACTIVITY_LATCHED);
            
            printProgress ("Setting up the control registers...\n");
            success = writeControlRegisterDS2408 (gPortNumber, pAddress, controlByte);
            if (success)
            {
                UInt8 pinsState = OW_PIN_IR_DETECT_NORTH | OW_PIN_IR_DETECT_SOUTH | OW_PIN_IR_DETECT_EAST | OW_PIN_IR_DETECT_WEST;

                printDebug ("Writing 0x%.2x to pins.\n", pinsState);
                success = channelAccessWriteDS2408 (gPortNumber, pAddress, &pinsState);
            }
        }
    }
    
    return success;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool success = false;
    UInt8 numBatteryDevices;
    UInt8 numIoDevices;
    SInt32 angleMicrosteps = 0;
    UInt32 x;
        
    setDebugPrintsOnToFile ("charger.log");
    setProgressPrintsOn();
    
    setDebugPrintsOff();
    
    /* Open the One Wire serial port */
    if (argc == 2)
    {
        gPortNumber = oneWireStartBus (argv[1]);
    }
    else
    {
        gPortNumber = oneWireStartBus (ONEWIRE_PORT);
    }

    if (gPortNumber < 0)
    {
        ASSERT_ALWAYS_STRING ("Failed to acquire port for OneWire interface.");
    }
    else
    {  
        numBatteryDevices = FindDevices (gPortNumber, &gBatteryDeviceArray[0], FAMILY_SBATTERY, MAX_BATTERY_DEVICES);

        if (numBatteryDevices == 0)
        {
            ASSERT_ALWAYS_STRING ("OneWire battery monitoring device not found.");
        }
        else
        {            
            printProgress ("Found %d OneWire battery monitoring device(s).\n", numBatteryDevices);
            
            numIoDevices = FindDevices (gPortNumber, &gIoDeviceArray[0], FAMILY_PIO, MAX_IO_DEVICES);

            if (numIoDevices == 0)
            {
                ASSERT_ALWAYS_STRING ("OneWire IO device not found.");
            }
            else
            {
                printProgress ("Setting up OneWire IO device...\n");
                success = setupIoDevice (&gIoDeviceArray[0][0]);

                if (success)
                {
                    /* Initialise the GPIOs */
                    success = openIo();
                    
                    if (success)
                    {
                        printProgress ("Setting up PI GPIOs...\n");       
                        setupPiGpios ();
                        
                        /*
                         * The charger should be placed against a wall,
                         * oriented so that it can only be approached
                         * from the North.  The code here should then
                         * line the charging point (which is mounted at
                         * North) up with an approaching IR.
                         */
                        printProgress ("Starting main loop...\n");
                        do
                        {
                            UInt32 clockwiseVote;
                            UInt32 antiClockwiseVote;
                            UInt32 moveVote;
                            
                            clockwiseVote = 0;
                            antiClockwiseVote = 0;
                            moveVote = 0;
                            
                            /* Count over a number of readings to avoid glitches */
                            for (x = 0; (x < MIN_READINGS) && success; x++)
                            {
                                Bool isNorth;
                                Bool isEast;
                                Bool isWest;
                                UInt8 pinsState;
                                
                                success = readPIOLogicStateDS2408 (gPortNumber, &gIoDeviceArray[0][0], &pinsState);
                                if (success)
                                {
                                    isNorth = !(pinsState & OW_PIN_IR_DETECT_NORTH);
                                    isEast = !(pinsState & OW_PIN_IR_DETECT_EAST);
                                    isWest = !(pinsState & OW_PIN_IR_DETECT_WEST);
                                    
                                    if (isWest && !isEast)
                                    {
                                        /* If West LED is on, and not East, go anticlockwise */
                                        antiClockwiseVote++;
                                    }
                                    else
                                    {
                                        /* If East LED is on, and not West, go clockwise */
                                        if (isEast && !isWest)
                                        {
                                            clockwiseVote++;
                                        }
                                    }
                                    
                                    if (isNorth && ((!isWest && !isEast) || (isWest && isEast)))
                                    {
                                        /* We're lined up, do nothing */
                                    }
                                    else
                                    {
                                        moveVote++;
                                    }
                                }
                                else
                                {
                                    ASSERT_ALWAYS_STRING ("Failed to read OneWire IO device.\n");
                                }
                            }
                                                    
                            /* If there has been a decisive set of readings indicating a
                             * need to move, do something */
                            if ((moveVote == MIN_READINGS) && (clockwiseVote != antiClockwiseVote))
                            {
                                SInt8 microstepChange;
                                
                                /* Set the direction */
                                if (clockwiseVote > antiClockwiseVote)
                                {
                                    GPIO_SET (GPIO_MOTOR_DIRECTION);
                                    microstepChange = 1;
                                }
                                else
                                {
                                    GPIO_CLR (GPIO_MOTOR_DIRECTION);
                                    microstepChange = -1;
                                }
                
                                /* Move the minimum number of microsteps in the given direction */
                                for (x = 0; x < MIN_ANGLE_MICROSTEPS; x++)
                                {
                                    if ((angleMicrosteps + microstepChange < LIMIT_ANGLE_MICROSTEPS) &&
                                        (angleMicrosteps + microstepChange > -LIMIT_ANGLE_MICROSTEPS))
                                    {
                                        GPIO_CLR (GPIO_MOTOR_ENABLE_BAR);
                                        GPIO_SET (GPIO_MOTOR_MICRO_STEP);
                                        usleep (10000);                        
                                        GPIO_CLR (GPIO_MOTOR_MICRO_STEP);
                                        usleep (10000);
                                        angleMicrosteps += microstepChange;
                                        GPIO_SET (GPIO_MOTOR_ENABLE_BAR);
                                    }
                                }
                            }
                        }
                        while (!key_abort() && success);
                        printProgress ("Exiting...\n");
                        GPIO_CLR (GPIO_IR_ENABLE);
                        closeIo();
                    }
                    else
                    {
                        printProgress ("PI GPIO initialisation failure, is this program running as root?\n");
                    }
                }
                else
                {
                    ASSERT_ALWAYS_STRING ("Failed to set up OneWire IO device.");
                }
            }
        }
        
        printProgress ("Closing OneWire port...\n");
        oneWireStopBus (gPortNumber);
    }
    
    if (!success)
    {
        printProgress ("Failed!\n");
        exit (-1);
    }

    return success;
}