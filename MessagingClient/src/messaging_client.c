/*
 * messaging_client.c
 * Generic messaging client, talks to generic messaging server.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <messaging_client.h>

/*
 * EXTERN
 */

extern int errno;

/*
 * STATIC FUNCTIONS
 */

/*
 * Send a message to the server.  This
 * function establishes a connection on the
 * given port and sends the message provided.
 * It waits for a response message if
 * pReceivedMsg is not PNULL.  Then it closes
 * the connection.
 * 
 * serverPort      the port number to use.
 * pIpAddressToUse pointer to a null terminated
 *                 string representing the IP address
 *                 to use.  May be PNULL, in which
 *                 case 127.0.0.1 is used.
 * pSendMsg        the message to send.
 * pReceivedMsg    the response received from
 *                 the server, may be PNULL, in
 *                 which case no response from
 *                 the server is expected.
 * 
 * @return      client return code.
 */
ClientReturnCode runMessagingClient (UInt16 serverPort, Char *pIpAddressToUse, Msg *pSendMsg, Msg *pReceivedMsg)
{
    ClientReturnCode returnCode = CLIENT_SUCCESS;
    UInt32 serverSocket;
    SockAddrIn messagingServer;
    UInt16 rawSendLength;
    Char *pIpAddress = "127.0.0.1";

    suspendDebug(); /* Switch the detail off 'cos it gets annoying */
    if (pSendMsg != PNULL)
    {
        /* Create the TCP socket */
        serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket >= 0)
        {
            printDebug ("Messaging Client %d: created socket %d.\n", serverPort, serverSocket);
            
            if (pIpAddressToUse != PNULL)
            {
               pIpAddress = pIpAddressToUse;            
            }
            
            /* Construct the server sockaddr_in structure */
            memset (&messagingServer, 0, sizeof (messagingServer)); 
            messagingServer.sin_family = AF_INET;                      /* Internet/IP */
            messagingServer.sin_addr.s_addr = inet_addr (pIpAddress); /* IP address */
            messagingServer.sin_port = htons (serverPort);             /* server port */
            
            /* Establish connection */
            if (connect (serverSocket, (SockAddr *) &messagingServer, sizeof (messagingServer)) >= 0)
            {
                printDebug ("Messaging Client %d: connected to server on socket.\n", serverPort, serverSocket);
                /* Send the message to the server */
                rawSendLength = pSendMsg->msgLength + SIZE_OF_MSG_LENGTH;
                if (send (serverSocket, pSendMsg, rawSendLength, 0) == rawSendLength)
                {
                    printDebug ("Messaging Client %d: sent %d bytes to server on socket.\n", serverPort, rawSendLength, serverSocket);
                    if (pReceivedMsg != PNULL)
                    {
                        UInt16 rawReceivedLength = 0;
                        SInt16 rawBytesReceived = 0;                        
                        MsgLength *pReceivedMsgLength;
                        
                        pReceivedMsgLength = &(pReceivedMsg->msgLength);
                        /* Set the message length to max to begin with */
                        *pReceivedMsgLength = MAX_MSG_LENGTH;
        
                        /* Receive the response back from the server */
                        while ((rawReceivedLength < *pReceivedMsgLength + SIZE_OF_MSG_LENGTH) && (rawReceivedLength < MAX_MSG_LENGTH + SIZE_OF_MSG_LENGTH) && (rawBytesReceived >= 0) && *pReceivedMsgLength > 0)
                        {
                            rawBytesReceived = recv (serverSocket, pReceivedMsg, *pReceivedMsgLength + SIZE_OF_MSG_LENGTH, 0);
                            if (rawBytesReceived >= 0)
                            {
                                printDebug ("Messaging Client %d: received %d bytes from server so far.\n", serverPort, rawBytesReceived);
                                rawReceivedLength += rawBytesReceived;
                            }
                            else
                            {
                                returnCode = CLIENT_ERR_FAILED_ON_RECV;
                                fprintf (stderr, "Failure returned by recv() (value %d), error: %s.\n", rawBytesReceived, strerror (errno));
                            }
                        }
                        
                        if (rawReceivedLength != *pReceivedMsgLength + SIZE_OF_MSG_LENGTH)
                        {
                            returnCode = CLIENT_ERR_MESSAGE_FROM_SERVER_INCOMPLETE_OR_TOO_LONG;
                            fprintf (stderr, "Message from server incomplete or too long (%d bytes received, %d bytes needed), error: %s.\n", rawReceivedLength, *pReceivedMsgLength + SIZE_OF_MSG_LENGTH, strerror (errno));
                        }
                        else
                        {
                            printDebug ("Messaging Client %d: received %d bytes from server on socket.\n", serverPort, rawReceivedLength, serverSocket);
                        }
                    }
                    else
                    {
                        printDebug ("Messaging Client %d: not waiting for a response.\n", serverPort);                        
                    }
                }
                else
                {
                    returnCode = CLIENT_ERR_COULDNT_SEND_WHOLE_MESSAGE_TO_SERVER;
                    fprintf (stderr, "Couldn't send whole %d byte message to server, error: %s.\n", rawSendLength, strerror (errno));
                }
            }
            else
            {
                returnCode = CLIENT_ERR_FAILED_TO_CONNECT_TO_SERVER;
                fprintf (stderr, "Failed to connect to server on port %d, error: %s.\n", serverPort, strerror (errno));                           
            }
            
            close (serverSocket);
            printDebug ("Messaging Client %d: closed socket.\n", serverPort, serverSocket);
        }
        else
        {
            returnCode = CLIENT_ERR_FAILED_TO_CREATE_SOCKET;
            fprintf (stderr, "Failed to create socket %ld on port %d, error: %s.\n", serverSocket, serverPort, strerror (errno));        
        }
    }
    else
    {
        returnCode = CLIENT_ERR_SEND_MESSAGE_IS_PNULL;
        fprintf (stderr, "Send message is PNULL.\n");   
    }
    resumeDebug();

    return returnCode;
}