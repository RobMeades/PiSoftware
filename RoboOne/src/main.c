/*
 * One Wire bus handling thread for RoboOne.
 */ 
 
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <ow_bus.h>
#include <menu.h>

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool  success = true;
    
    /* Setup what's necessary for OneWire bus stuff */
    success = startOneWireBus();
    
    /* Find and setup the devices on the OneWire bus */
    if (success)
    {
        success = setupDevices();
        
        if (success)
        {
            /* Run the interactive menu */
            runMenu();
        }
        else
        {
            /* If the setup fails, print out what devices we can find */
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
    
    /* Shut things down gracefully */
    stopOneWireBus();
    
    return success;
}