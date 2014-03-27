/*
 * onewireserver.c
 * Builds my onewire.a library into a sockets server.
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

/* Max connection requests */
#define MAXPENDING  5

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * Handle a whole message received from the client. 
 * 
 * pReceivedMsg   a pointer to the buffer containing the
 *                incoming message.
 * pResponse      a pointer to a message buffer to put
 *                the response into. Not touched if success
 *                is false.
 */
static void handleMsg (OneWireMsg *pReceivedMsg, OneWireMsg *pSendMsg)
{
    OneWireReqMsgHeader msgHeader;
    
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);

    /* First check the message length and type */
    printProgress ("Received message of length %d and type %d.\n", pReceivedMsg->msgLength, pReceivedMsg->msgType);    
    ASSERT_PARAM (pReceivedMsg->msgLength < MAX_MSG_LENGTH, pReceivedMsg->msgLength);
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_ONE_WIRE_MSG, pReceivedMsg->msgType);
    
    /* Then get the header */
    memcpy (&msgHeader, (pReceivedMsg + OFFSET_TO_REQ_MSG_HEADER), sizeof (msgHeader));
    printProgress ("Header has port number 0x%x and serialNumber 0x%x%x%x%x%x%x%x%x.", msgHeader.portNumber, msgHeader.serialNumber[0], msgHeader.serialNumber[1], msgHeader.serialNumber[2], msgHeader.serialNumber[3], msgHeader.serialNumber[4], msgHeader.serialNumber[5], msgHeader.serialNumber[6], msgHeader.serialNumber[7]);
    
    /* TODO: now process the message and create the response */
    pSendMsg->msgType = 1;
    pSendMsg->msgLength = 1;
}

/*
 * Handle the comms with the client. 
 * 
 * socket   the socket with which to communicate.
 *
 * @return  true if successful, otherwise false.
 */
static Bool handleSendReceive (UInt32 socket)
{
    Bool success = true;
    OneWireMsg *pReceivedMsg;
    OneWireMsgLength *pReceivedMsgLength = PNULL;
    UInt16 receivedLength;
    OneWireMsg *pSendMsg;
  
    pReceivedMsg = malloc (sizeof (OneWireMsg));
    
    if (pReceivedMsg != PNULL)
    {
        /* Receive a message */
        pReceivedMsgLength = &(pReceivedMsg->msgLength);
        /* Set the received message length to max to begin with */
        *((UInt8 *) pReceivedMsg + OFFSET_TO_MSG_LENGTH) = MAX_MSG_LENGTH;        

        do
        {
            receivedLength = recv (socket, pReceivedMsg, sizeof (OneWireMsg), 0);
        }
        while ((receivedLength != *pReceivedMsgLength) && (receivedLength < MAX_MSG_LENGTH) && (receivedLength >= 0));
        
        if (receivedLength != *pReceivedMsgLength)
        {
            /* Act upon it */
            pSendMsg = malloc (sizeof (OneWireMsg));
            
            if (pSendMsg != PNULL)
            {
                handleMsg (pReceivedMsg, pSendMsg);
                
                /* Send the response */
                if (send (socket, pSendMsg, pSendMsg->msgLength, 0) != pSendMsg->msgLength)
                {
                    success = false;
                    printProgress ("Failed to send response to client");
                }
                
                free (pSendMsg);
            }
            else
            {
                success = false;
                printProgress ("Failed to get memory for response");                    
            }
        }
        else
        {
            success = false;
            printProgress ("Failed to receive full message from client");        
        }
        
        free (pReceivedMsg);
    }
    else
    {
        success = false;
        printProgress ("Failed to get memory for reception");   
    }

    return success;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool success = true;
    UInt32 serverSocket;
    UInt32 clientSocket;
    SockAddrIn oneWireServer;
    SockAddrIn oneWireClient;

    /* Create the TCP socket */
    serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket >= 0)
    {
        /* Construct the server SockAddrIn structure */
        memset (&oneWireServer, 0, sizeof (oneWireServer));  
        oneWireServer.sin_family = AF_INET;                     /* Internet/IP */
        oneWireServer.sin_addr.s_addr = htonl (INADDR_ANY);     /* Incoming addr */
        oneWireServer.sin_port = htons (ONE_WIRE_SERVER_PORT);  /* server port */
        
        /* Bind the server socket */
        if (bind (serverSocket, (SockAddr *) &oneWireServer, sizeof (oneWireServer)) >= 0)
        {
            /* Listen on the server socket */
            if (listen (serverSocket, MAXPENDING) >= 0)
            {
                /* Run until error */
                while (success)
                {
                    socklen_t oneWireClientLen = sizeof (oneWireClient);
                      
                    /* Wait for a client to connect */
                    if ((clientSocket = accept (serverSocket, (SockAddr *) &oneWireClient, &oneWireClientLen)) >= 0)
                    {
                        fprintf (stdout, "Client connected: %s\n", inet_ntoa (oneWireClient.sin_addr));
                        
                        /* Do something with the message */
                        success = handleSendReceive (clientSocket);
                        
                        /* Close the socket again */
                        close (clientSocket);                    
                    }
                    else
                    {
                        success = false;
                        printProgress ("Failed to accept client connection");                        
                    }
                }
            }
            else
            {
                success = false;
                printProgress ("Failed to listen on server socket");               
            }
        }
        else
        {
            success = false;
            printProgress ("Failed to bind the server socket");            
        }
    }
    else
    {
        success = false;
        printProgress ("Failed to create socket");
    }
    
    return success;
}