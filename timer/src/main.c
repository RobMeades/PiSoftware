/*
 * main.c
 * Entry point for timer_server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <timer_server.h>
#include <timer_msg_auto.h>
#include <timer_client.h>

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point - start the Timer messaging server,
 * listening on a socket.
 */
int main (int argc, char **argv)
{
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;
    UInt16 timerServerPort;

    /*setDebugPrintsOnToFile ("timer.log");*/
    setProgressPrintsOn();
    /* setDebugPrintsOnToSyslog();*/ /* Don't switch any debug on in here, the load has a significant effect */

    if (argc == 2)
    {
        /* Start up the server */
        timerServerPort = atoi (argv[1]);
        printProgress ("Timer server listening on port %d.\n", timerServerPort);

        returnCode = runMessagingServer (timerServerPort);
        
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("Timer server exiting normally.\n");            
        }
        else
        {
            printProgress ("Timer server exiting with returnCode %d.\n", returnCode);                        
        }        
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5235\n", argv[0], argv[0]);
    }
    
    setDebugPrintsOff();
    
    return returnCode;
}