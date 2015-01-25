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
        GPIO_CONFIG_OUTPUT (GPIO_MOTOR_STEP);
        GPIO_CONFIG_OUTPUT (GPIO_MOTOR_DIRECTION);

        while(1)
        {
          GPIO_SET (GPIO_MOTOR_STEP);
          usleep (1000);
       
          GPIO_CLR (GPIO_MOTOR_STEP);
          usleep (1000);
        }
        
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