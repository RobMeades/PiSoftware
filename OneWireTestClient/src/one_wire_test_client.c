/*
 * onewiretestclient.c
 * For messing around
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
#include <messaging_client.h>

/*
 * MANIFEST CONSTANTS
 */
#define ONEWIRE_PORT_STRING    "/dev/USBSerial"
#define ONE_WIRE_SERVER_PORT   5000

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    ClientReturnCode returnCode = CLIENT_SUCCESS;
    Msg *pSendMsg;
    Msg *pReceivedMsg;
    
    setDebugPrintsOn();
    setProgressPrintsOn();

    pSendMsg = malloc (sizeof (Msg));
    
    if (pSendMsg != PNULL)
    {
        /* Setup a test message (which will just cause the server to exit) */
        pSendMsg->msgLength = 1;
        pSendMsg->msgType = 0;
        
        pReceivedMsg = malloc (sizeof (Msg));
        if (pReceivedMsg != PNULL)
        {
            returnCode = runMessagingClient (ONE_WIRE_SERVER_PORT, pSendMsg, pReceivedMsg);
            
            if ((returnCode == CLIENT_SUCCESS) && (pReceivedMsg->msgLength > 0))
            {
                printDebug ("Client: received message of length %d and type %d.\n", pReceivedMsg->msgLength, pReceivedMsg->msgType);    
            }

            free (pReceivedMsg);
        }
        else
        {
            returnCode = CLIENT_ERR_GENERAL_FAILURE;
            printDebug ("Failed to get memory for receiving.\n");               
        }        
        
        free (pSendMsg);
    }
    else
    {
        returnCode = CLIENT_ERR_GENERAL_FAILURE;
        printDebug ("Failed to get memory for sending.\n");   
    }

    return returnCode;
}