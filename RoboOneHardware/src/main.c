/*
 * main.c
 * Entry point for hardware_server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <ow_bus.h>

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point - start the Hardware messaging server,
 * listening on a socket.
 */
int main (int argc, char **argv)
{
    Bool success = false;
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;
    UInt16 hardwareServerPort;

    setDebugPrintsOnToFile ("roboonehardware.txt");
    setProgressPrintsOn();

    if (argc == 2)
    {
        /* First of all, start up the OneWire bus */
        success = startOneWireBus();
          
        /* Find and setup the devices on the OneWire bus */
        if (success)
        {
            success = setupDevices();                  
            if (success)
            {
                /* Now start up the server */
                hardwareServerPort = atoi (argv[1]);
                printProgress ("HardwareServer listening on port %d.\n", hardwareServerPort);

                returnCode = runMessagingServer (hardwareServerPort);                
            }
            else
            {
                /* If the setup fails, print out what devices we can find */
                findAllDevices();
            }
            
            /* Shut the OneWire stuff down gracefully */
            stopOneWireBus ();
        }
        
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("HardwareServer exiting normally.\n");            
        }
        else
        {
            printProgress ("HardwareServer exiting with returnCode %d.\n", returnCode);                        
        }        
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5234\n", argv[0], argv[0]);
    }
    
    setDebugPrintsOff();
    
    return returnCode;
}