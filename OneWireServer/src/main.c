/*
 * main.c
 * Entry point for one_wire_server.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <rob_system.h>
#include <one_wire.h>
#include <messaging_server.h>
#include <one_wire_server.h>

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point - start the OneWire messaging server,
 * listening on a socket.
 */
int main (int argc, char **argv)
{
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;
    UInt16 oneWireServerPort;

    setDebugPrintsOn();
    setProgressPrintsOn();

    if (argc == 2)
    {
        oneWireServerPort = atoi (argv[1]);
        printProgress ("OneWireServer listening on port %d.\n", oneWireServerPort);

        returnCode = runMessagingServer (oneWireServerPort);
        
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("OneWireServer exiting normally.\n");            
        }
        else
        {
            printProgress ("OneWireServer exiting with returnCode %d.\n", returnCode);                        
        }            
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5234\n", argv[0], argv[0]);
    }
    
    return returnCode;
}