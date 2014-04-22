/*
 * main.c
 * Entry point for the RoboOne Task Handler.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <task_handler_types.h>
#include <task_handler_server.h>
#include <task_handler_msg_auto.h>
#include <task_handler_client.h>

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point - start the state machine messaging server,
 * listening on a socket.
 */
int main (int argc, char **argv)
{
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;
    UInt16 taskHandlerServerPort;

    setDebugPrintsOnToFile ("roboonetaskhandler.log");
    setProgressPrintsOn();

    if (argc == 2)
    {
        taskHandlerServerPort = atoi (argv[1]);
        printProgress ("Task handler server listening on port %d.\n", taskHandlerServerPort);

        returnCode = runMessagingServer (taskHandlerServerPort);
            
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("Task handler server exiting normally.\n");            
        }
        else
        {
            printProgress ("Task handler server exiting with returnCode %d.\n", returnCode);                        
        }            
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5233\n", argv[0], argv[0]);
    }
    
    setDebugPrintsOff();
    
    return returnCode;
}