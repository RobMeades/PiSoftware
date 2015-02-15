/*
 * main.c
 * Entry point for battery_manager_server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <hardware_types.h>
#include <battery_manager_server.h>
#include <battery_manager_msg_auto.h>
#include <battery_manager_client.h>

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point - start the Battery Manager messaging server,
 * listening on a socket.
 */
int main (int argc, char **argv)
{
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;
    UInt16 batteryManagerServerPort;

    setDebugPrintsOnToFile ("roboonebatterymanager.log");
    setProgressPrintsOn();

    if (argc == 2)
    {
        /* Start up the server */
        batteryManagerServerPort = atoi (argv[1]);
        printProgress ("Battery Manager server listening on port %d.\n", batteryManagerServerPort);

        returnCode = runMessagingServer (batteryManagerServerPort);
        
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("Battery Manager server exiting normally.\n");            
        }
        else
        {
            printProgress ("Battery Manager server exiting with returnCode %d.\n", returnCode);                        
        }        
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5231\n", argv[0], argv[0]);
    }
    
    setDebugPrintsOff();
    
    return returnCode;
}