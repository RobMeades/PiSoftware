/*
 * Library functions for Pi IO.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 
 
#include <sys/mman.h>
#include <fcntl.h>

#include <unistd.h>

#include <rob_system.h>
#include <pi_io.h>


/*
 * MANIFEST CONSTANTS
 */

/* GPIO memory mapping */
#ifdef PI_1
#    define BCM2708_PERIPHERALS_BASE 0x20000000
#else
/* For PI 2 it's different, thanks to Gordon Henderson of wiringPi for telling me */
#    define BCM2708_PERIPHERALS_BASE 0x3F000000
#endif

#define GPIO_BASE                (BCM2708_PERIPHERALS_BASE + 0x200000)  /* GPIO controller */ 
 
#define BLOCK_SIZE (4 * 1024)
 
/*
 * TYPES
 */

/*
 * GLOBALS (prefixed with g)
 */

Bcm2835Peripheral gGpio = {GPIO_BASE, 0, 0, 0};
 
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
 * PUBLIC FUNCTIONS
 */

/*
 * Initialise the GPIO code. Once this has been
 * called the macro interface can be used. Returns
 * true if successful, otherwise false. If the IO
 * is already initialised when this is called then
 * it is closed and reinitialised.
 */
Bool openIo (void)
{
    if (gGpio.memFd >= 0)
    {
        unmapPeripheral (&gGpio);
    }
    
    return mapPeripheral (&gGpio);
}

/*
 * Close the GPIO code, tidying up
 */
void closeIo (void)
{
    unmapPeripheral (&gGpio);
}

/*
 * Configure a GPIO to be an input, returning
 * true if successful, otherwise false.
 */
Bool configGpioInput (UInt8 gpio)
{
    Bool success = true;
    
    if (gGpio.memFd <= 0)
    {
        success = mapPeripheral (&gGpio);
    }
    
    if (success)
    {
        GPIO_CONFIG_INPUT (gpio); 
    }
    
    return success;
}

/*
 * Configure a GPIO to be an output, returning
 * true if successful, otherwise false.
 */
Bool configGpioOutput (UInt8 gpio)
{
    Bool success = true;
    
    if (gGpio.memFd <= 0)
    {
        success = mapPeripheral (&gGpio);
    }
    
    if (success)
    {
        GPIO_CONFIG_OUTPUT (gpio); 
    }
    
    return success;
}

/*
 * Set a GPIO pin.
 */
Bool setGpio (UInt8 gpio)
{
    Bool success = true;
    
    if (gGpio.memFd <= 0)
    {
        success = mapPeripheral (&gGpio);
    }
    
    if (success)
    {
        GPIO_SET (gpio); 
    }
    
    return success;
}

/*
 * Clear a GPIO pin.
 */
Bool clrGpio (UInt8 gpio)
{
    Bool success = true;
    
    if (gGpio.memFd <= 0)
    {
        success = mapPeripheral (&gGpio);
    }
    
    if (success)
    {
        GPIO_CLR (gpio); 
    }
    
    return success;
}

/*
 * Read a GPIO pin.  If successful then the
 * state of the GPIO pin is returned in pState.
 */
Bool readGpio (UInt8 gpio, Bool *pState)
{
    Bool success = true;
    
    if (gGpio.memFd <= 0)
    {
        success = mapPeripheral (&gGpio);
    }
    
    if (success && (pState != NULL))
    {
        *pState = (Bool) GPIO_READ (gpio); 
    }
    
    return success;
}
