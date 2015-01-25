/*
 * Main for Charger.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <curses.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <rob_system.h>
#include <main.h>

/*
 * GLOBALS (prefixed with g)
 */

Bcm2835Peripheral gGpio = {GPIO_BASE};
 
/*
 * MANIFEST CONSTANTS
 */

/* GPIO handling macros */ 
#define GPIO_CONFIG_INPUT(g)  (*(gGpio.pAddress + ((g) / 10)) &= ~(7 << (((g) % 10) * 3)))
#define GPIO_CONFIG_OUTPUT(g) (*(gGpio.pAddress + ((g) / 10)) = (*(gGpio.pAddress + ((g) / 10)) & ~(7 << (((g) % 10) * 3))) |  (1 << (((g) % 10) * 3))) 
#define GPIO_CONFIG_ALT(g, a) (*(gGpio.pAddress + (((g) / 10))) |= (((a) <= 3 ? (a) + 4 : (a) == 4 ? 3 : 2) << (((g) % 10) * 3)))
#define GPIO_SET              (*(gGpio.pAddress + 7))  /* sets   bits which are 1 ignores bits which are 0 */
#define GPIO_CLR              (*(gGpio.pAddress + 10)) /* clears bits which are 1 ignores bits which are 0 */
#define GPIO_READ(g)          (*(gGpio.pAddress + 13) &= (1 << (g)))

/*
 * STATIC FUNCTIONS
 */

/*
 * Expose the physical address defined in the passed structure using mmap on /dev/mem
 * Note that this code _must_ run as root to get at /dev/mem.
 */
static Bool mapPeripheral (Bcm2835Peripheral *pPeripheral)
{
    Bool success = false;
    
    /* Open /dev/mem */
    pPeripheral->memFd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (pPeripheral->memFd >= 0)
    {
        pPeripheral->pMap = mmap (NULL,
                                  BLOCK_SIZE,
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED,
                                  pPeripheral->memFd,   /* File descriptor to physical memory virtual file '/dev/mem' */
                                  pPeripheral->addressP); /* Address in physical map that we want this memory block to expose */
                                  
        if (pPeripheral->pMap != MAP_FAILED)
        {
            pPeripheral->pAddress = (volatile unsigned int *) pPeripheral->pMap;
            success = true;
        }
    }
 
   return success;
}
 
/*
 * Tidy up
 */
static void unmapPeripheral (Bcm2835Peripheral *pPeripheral)
{
    munmap (pPeripheral->pMap, BLOCK_SIZE);
    close (pPeripheral->memFd);
}

/*
 * Initalise globals.
 */
static Bool initGlobals (void)
{
    return mapPeripheral (&gGpio);    
}

/*
 * Tidy up.
 */
static void deInitGlobals (void)
{
    unmapPeripheral (&gGpio);    
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
    
    setDebugPrintsOnToFile ("charger.log");
    setProgressPrintsOn();
    
    setDebugPrintsOff();

    success = initGlobals();
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
          GPIO_SET = 1 << GPIO_MOTOR_STEP;
          usleep (1000);
       
          GPIO_CLR = 1 << GPIO_MOTOR_STEP;
          usleep (1000);
        }
        
        deInitGlobals();
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