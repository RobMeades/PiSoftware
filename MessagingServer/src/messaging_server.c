/*
 * messagingserver.c
 * A generic server that accepts messages over a sockets interface.
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

/*
 * MANIFEST CONSTANTS
 */

/* Max connection requests */
#define MAXPENDING  5


/*
 * EXTERN
 */

extern int errno;

/*
 * STATIC FUNCTIONS
 */

/*
 * Handle the comms with the client. 
 * 
 * socket   the socket with which to communicate.
 *
 * @return  a return code.
 */
static ServerReturnCode handleSendReceive (UInt32 clientSocket)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
    Msg *pReceivedMsg;
    MsgLength *pReceivedMsgLength = PNULL;
    UInt16 rawReceivedLength = 0;
  
    pReceivedMsg = malloc (sizeof (Msg));
    
    if (pReceivedMsg != PNULL)
    {
        SInt16 rawBytesReceived;

        /* Receive a message */
        pReceivedMsgLength = &(pReceivedMsg->msgLength);
        /* Set the received message length to max to begin with */
        *pReceivedMsgLength = MAX_MSG_LENGTH;        

        do
        {
            rawBytesReceived = recv (clientSocket, pReceivedMsg, *pReceivedMsgLength + SIZE_OF_MSG_LENGTH, 0);
            if (rawBytesReceived > 0)
            {
                rawReceivedLength += rawBytesReceived;
            }
        }
        while ((rawReceivedLength != *pReceivedMsgLength + SIZE_OF_MSG_LENGTH) && (rawReceivedLength < MAX_MSG_LENGTH + SIZE_OF_MSG_LENGTH) && (rawBytesReceived >= 0) && *pReceivedMsgLength > 0);
        
        if (rawReceivedLength == *pReceivedMsgLength + SIZE_OF_MSG_LENGTH)
        {
            Msg *pSendMsg;

            /* Create space for a message to send back */
            pSendMsg = malloc (sizeof (Msg));
            
            if (pSendMsg != PNULL)
            {
                pSendMsg->msgLength = 0; /* Set the response message to zero length before calling the handler */
                
                /* Call the external function to handle the message and,
                 * optionally, create a response */
                returnCode = serverHandleMsg (pReceivedMsg, pSendMsg);
                
                /* Send the response if there is one */
                if (((returnCode == SERVER_EXIT_NORMALLY) || (returnCode == SERVER_SUCCESS_KEEP_RUNNING)) && (pSendMsg->msgLength > 0))
                {
                    UInt16 rawSendLength;
                    
                    rawSendLength = pSendMsg->msgLength + SIZE_OF_MSG_LENGTH;
                    ASSERT_PARAM (rawSendLength <= MAX_MSG_LENGTH, rawSendLength);
                    
                    if (send (clientSocket, pSendMsg, rawSendLength, 0) != rawSendLength)
                    {
                        returnCode = SERVER_ERR_FAILED_TO_SEND_RESPONSE_TO_CLIENT;
                        fprintf (stderr, "Failed to send response to client (%d bytes), error: %s.\n", rawSendLength, strerror (errno));
                    }
                }
                
                free (pSendMsg);
            }
            else
            {
                returnCode = SERVER_ERR_FAILED_TO_GET_MEMORY_FOR_RESPONSE;
                fprintf (stderr, "Failed to get memory (%d bytes) for response.\n", sizeof (Msg));                    
            }
        }
        else
        {
            returnCode = SERVER_ERR_MESSAGE_FROM_CLIENT_INCOMPLETE_OR_TOO_LONG;
            fprintf (stderr, "Message from client incomplete or too long (%d bytes received, %d bytes needed).\n", rawReceivedLength, *pReceivedMsgLength + SIZE_OF_MSG_LENGTH);        
        }
        
        free (pReceivedMsg);
    }
    else
    {
        returnCode = SERVER_ERR_FAILED_TO_GET_MEMORY_FOR_RECEPTION;
        fprintf (stderr, "Failed to get memory (%d bytes) for reception.\n", sizeof (Msg));   
    }

    return returnCode;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point.  This function creates the messaging
 * server on port 'messagingServerPort' and listens for
 * connections.  When a connection is made it waits
 * for a message on that connection, passes it to
 * the external handler 'messagingServerHandler()' for
 * treatment and then closes the connection once more.
 * 
 * serverPort  the port number to use.
 * 
 * @return     server return code.
 */
ServerReturnCode runMessagingServer (UInt16 serverPort)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
    UInt32 serverSocketOptionValue = 1;
    UInt32 serverSocket;
    UInt32 clientSocket;
    SockAddrIn messagingServer;
    SockAddrIn messagingClient;

    /* Create the TCP socket */
    suspendDebug(); /* Switch the detail off 'cos it gets annoying */
    serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket >= 0)
    {
        printDebug ("Messaging Server %d: created socket %d.\n", serverPort, serverSocket);
        /* Construct the server SockAddrIn structure */
        memset (&messagingServer, 0, sizeof (messagingServer));  
        messagingServer.sin_family = AF_INET;                     /* Internet/IP */
        messagingServer.sin_addr.s_addr = htonl (INADDR_ANY);     /* Incoming addr */
        messagingServer.sin_port = htons (serverPort);            /* server port */
        
        /* Set SO_REUSEADDR on the  socket to true (1) so that if we take
         * the server up and down there is no issue with it being in state
         * TIME_WAIT */
        if (setsockopt (serverSocket, SOL_SOCKET, SO_REUSEADDR, &serverSocketOptionValue, sizeof (serverSocketOptionValue)) >= 0)
        {
            printDebug ("Messaging Server %d: has set socket options.\n", serverPort);
            /* Bind the server socket */
            if (bind (serverSocket, (SockAddr *) &messagingServer, sizeof (messagingServer)) >= 0)
            {
                printDebug ("Messaging Server %d: bound to socket %d.\n", serverPort, serverSocket);
                /* Listen on the server socket */
                if (listen (serverSocket, MAXPENDING) >= 0)
                {
                    printDebug ("Messaging Server %d: listening on socket %d, maxpending %d.\n", serverPort, serverSocket, MAXPENDING);
                    /* Run until error or exit */
                    while (returnCode == SERVER_SUCCESS_KEEP_RUNNING)
                    {
                        socklen_t clientLength = sizeof (messagingClient);
                          
                        /* Wait for a client to connect */
                        if ((clientSocket = accept (serverSocket, (SockAddr *) &messagingClient, &clientLength)) >= 0)
                        {
                            printDebug ("Messaging Server %d: a client connected on client socket %d.\n", serverPort, clientSocket);
                            /* fprintf (stdout, "Client connected: %s\n", inet_ntoa (messagingClient.sin_addr)); */
                            
                            /* Exchange messages */
                            resumeDebug();
                            returnCode = handleSendReceive (clientSocket);
                            suspendDebug();
                            
                            /* Close the socket again */
                            close (clientSocket);                    
                            printDebug ("Messaging Server %d: closed client socket %d.\n", serverPort, clientSocket);
                        }
                        else
                        {
                            returnCode = SERVER_ERR_FAILED_TO_ACCEPT_CLIENT_CONNECTION;
                            fprintf (stderr, "Failed to accept client connection on socket %ld, port %d, error: %s.\n", serverSocket, serverPort, strerror (errno));
                        }
                    }
                }
                else
                {
                    returnCode = SERVER_ERR_FAILED_TO_LISTEN_ON_SOCKET;
                    fprintf (stderr, "Failed to listen on socket %ld, port %d, error: %s.\n", serverSocket, serverPort, strerror (errno));               
                }
            }
            else
            {
                returnCode = SERVER_ERR_FAILED_TO_BIND_SOCKET;
                fprintf (stderr, "Failed to bind to socket %ld on port %d, error: %s.\n", serverSocket, serverPort, strerror (errno));            
            }
        }
        else
        {
            returnCode = SERVER_ERR_FAILED_TO_SET_SOCKET_OPTIONS;
            fprintf (stderr, "Failed to set socket %ld options, error: %s.\n", serverSocket, strerror (errno));            
        }
    }
    else
    {
        returnCode = SERVER_ERR_FAILED_TO_CREATE_SOCKET;
        fprintf (stderr, "Failed to create socket on port %d, error: %s.\n", serverPort, strerror (errno));
    }
    resumeDebug();
    
    return returnCode;
}