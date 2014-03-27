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
#include <onewire_server.h>

/*
 * MANIFEST CONSTANTS
 */

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
    Bool success = true;
    UInt32 clientSocket;
    SockAddrIn oneWireServer;
    OneWireMsg *pSendMsg;
    UInt8 sendLength;
    OneWireMsg *pReceivedMsg;
    UInt8 receivedLength = 0;
    
    pSendMsg = malloc (sizeof (OneWireMsg));
    
    if (pSendMsg != PNULL)
    {
        /* Setup a test message */
        pSendMsg->msgLength = 1;
        pSendMsg->msgType = 0;
        
        /* Create the TCP socket */
        clientSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket >= 0)
        {
            /* Construct the server sockaddr_in structure */
            memset (&oneWireServer, 0, sizeof (oneWireServer)); 
            oneWireServer.sin_family = AF_INET;                      /* Internet/IP */
            oneWireServer.sin_addr.s_addr = inet_addr ("127.0.0.1"); /* IP address */
            oneWireServer.sin_port = htons (ONE_WIRE_SERVER_PORT);   /* server port */
            
            /* Establish connection */
            if (connect (clientSocket, (SockAddr *) &oneWireServer, sizeof (oneWireServer)) >= 0)
            {
                /* Send the message to the server */
                sendLength = sizeof (OneWireMsg);
                if (send (clientSocket, pSendMsg, sendLength, 0) == sendLength)
                {
                    UInt16 bytesReceived;
                    OneWireMsgLength *pMsgLength = PNULL;
                    
                    pReceivedMsg = malloc (sizeof (OneWireMsg));
                    pMsgLength = &(pReceivedMsg->msgLength);
                    /* Set the message length to max to begin with */
                    *pMsgLength = MAX_MSG_LENGTH;
    
                    /* Receive the response back from the server */
                    while ((receivedLength < *pMsgLength) && (receivedLength < MAX_MSG_LENGTH))
                    {
                        bytesReceived = recv (clientSocket, pReceivedMsg, sizeof (OneWireMsg), 0);
                        if (bytesReceived >= 1)
                        {
                            receivedLength += bytesReceived;
                        }
                    }
                    
                    if (receivedLength == *pMsgLength)
                    {
                        printProgress ("Received message type %d, length %d", pReceivedMsg->msgType, pReceivedMsg->msgLength);
                    }
                    else
                    {
                        printProgress ("Failed to receive whole message from server");
                    }
                    
                    free (pReceivedMsg);
                }
                else
                {
                    success = false;                
                    printProgress ("Couldn't send whole message to server");
                }
            }
            else
            {
                success = false;
                printProgress ("Failed to connect with server");                           
            }
            
            close (clientSocket);
        }
        else
        {
            success = false;
            printProgress ("Failed to create socket");        
        }
        free (pSendMsg);
    }
    else
    {
        success = false;
        printProgress ("Failed to get memory for sending");   
    }

    return success;
}