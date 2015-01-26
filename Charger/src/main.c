/*
 * Main for Charger.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <curses.h>

#include <unistd.h>

#include <rob_system.h>
#include <pi_io.h>
#include <main.h>

/*
 * GLOBALS (prefixed with g)
 */
 
/*
 * MANIFEST CONSTANTS
 */

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
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool success = false;
    SInt32 angleMicrosteps = 0;
    UInt32 x;
        
    setDebugPrintsOnToFile ("charger.log");
    setProgressPrintsOn();
    
    setDebugPrintsOff();
    
    success = openIo();
    if (success)
    {
        GPIO_CONFIG_INPUT (GPIO_IR_DETECT_NORTH);
        GPIO_CONFIG_INPUT (GPIO_IR_DETECT_SOUTH);
        GPIO_CONFIG_INPUT (GPIO_IR_DETECT_EAST);
        GPIO_CONFIG_INPUT (GPIO_IR_DETECT_WEST);
        
        GPIO_CONFIG_OUTPUT (GPIO_IR_ENABLE);
        GPIO_CONFIG_OUTPUT (GPIO_MOTOR_MICRO_STEP);
        GPIO_CONFIG_OUTPUT (GPIO_MOTOR_DIRECTION);

        GPIO_SET (GPIO_IR_ENABLE);
        
        /*
         * The charger should be placed against a wall,
         * oriented so that it can only be approached
         * from the North.  The code here should then
         * line the charging point (which is mounted at
         * North) up with an approaching IR.
         */
        while (1)
        {
            UInt32 clockwiseVote;
            UInt32 antiClockwiseVote;
            UInt32 moveVote;
            
            clockwiseVote = 0;
            antiClockwiseVote = 0;
            moveVote = 0;
            
            /* Count over a number of readings to avoid glitches */
            for (x = 0; x < MIN_READINGS; x++)
            {
                Bool isNorth = !GPIO_READ (GPIO_IR_DETECT_NORTH);
                Bool isEast = !GPIO_READ (GPIO_IR_DETECT_EAST);
                Bool isWest = !GPIO_READ (GPIO_IR_DETECT_WEST);
                
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
                        GPIO_SET (GPIO_MOTOR_MICRO_STEP);
                        usleep (10000);                        
                        GPIO_CLR (GPIO_MOTOR_MICRO_STEP);
                        usleep (10000);
                        angleMicrosteps += microstepChange;
                    }
                }
            }
        }
        
        GPIO_CLR (GPIO_IR_ENABLE);
        closeIo();
    }
    else
    {
        printProgress ("Initialisation failure, is this program running as root?\n");        
    }
    
    if (!success)
    {
        printProgress ("Failed!\n");
        exit (-1);
    }

    return success;
}