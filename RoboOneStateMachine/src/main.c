/*
 * main.c
 * Entry point for the RoboOne State Machine.
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
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>

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
    UInt16 stateMachineServerPort;

    setDebugPrintsOnToFile ("roboonestatemachine.log");
    setProgressPrintsOn();

    if (argc == 2)
    {
        stateMachineServerPort = atoi (argv[1]);
        printProgress ("State machine server listening on port %d.\n", stateMachineServerPort);

        returnCode = runMessagingServer (stateMachineServerPort);
            
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("State machine server exiting normally.\n");            
        }
        else
        {
            printProgress ("State machine server exiting with returnCode %d.\n", returnCode);                        
        }            
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5231\n", argv[0], argv[0]);
    }
    
    setDebugPrintsOff();
    
    return returnCode;
}