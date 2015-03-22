/*
 * main.c
 * Entry point for hardware_server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <hardware_types.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <orangutan.h>

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
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;
    UInt16 hardwareServerPort;

    setDebugPrintsOnToFile ("roboonehardware.log");
    setProgressPrintsOn();

    if (argc == 2)
    {
        /* Start up the server */
        hardwareServerPort = atoi (argv[1]);
        printProgress ("Hardware server listening on port %d.\n", hardwareServerPort);

        returnCode = runMessagingServer (hardwareServerPort);
        
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("Hardware server exiting normally.\n");            
        }
        else
        {
            printProgress ("Hardware server exiting with returnCode %d.\n", returnCode);                        
        }        
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5234\n", argv[0], argv[0]);
    }
    
    setDebugPrintsOff();
    
    return returnCode;
}