/*
 * test.c
 * Stubs for messagingserver for testing.  Sending
 * in a zero length message will cause the server
 * to exit, otherwise messages will be echoed back
 * to the client.
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

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point - start the messaging server,
 * listening on a socket.
 */
int main (int argc, char **argv)
{
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;
    UInt16 serverPort;

    setDebugPrintsOn();
    setProgressPrintsOn();

    if (argc == 2)
    {
        serverPort = atoi (argv[1]);
        printProgress ("Messaging server listening on port %d.\n", serverPort);

        returnCode = runMessagingServer (serverPort);
        
        if (returnCode == SERVER_EXIT_NORMALLY)
        {
            printProgress ("Messaging server exiting normally.\n");            
        }
        else
        {
            printProgress ("Messaging server exiting with returnCode %d.\n", returnCode);                        
        }            
    }    
    else
    {
        printProgress ("Usage: %s portnumber\ne.g. %s 5000\n", argv[0], argv[0]);
    }
    
    return returnCode;
}

/*
 * Handle a whole message received from the client
 * and echo the message back as a response.
 * 
 * pReceivedMsg   a pointer to the buffer containing the
 *                incoming message.
 * pSendMsg       a pointer to a message buffer to put
 *                the response into. Not touched if return
 *                code is a failure one.
 * 
 * @return        server return code.
 */
ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg *pSendMsg)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
    
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);

    /* First check the message length and type */
    if (pReceivedMsg->msgLength > MAX_MSG_LENGTH)
    {
        returnCode = SERVER_ERR_MESSAGE_TOO_LARGE;
        printProgress ("Message from clent was too large.\n");
    }
        
    /* Now echo the received message back as a response */
    memcpy (pSendMsg, pReceivedMsg, pReceivedMsg->msgLength + SIZE_OF_MSG_LENGTH);
    
    if (pReceivedMsg->msgLength == 0)
    {
        returnCode = SERVER_EXIT_NORMALLY;   
    }
    
    return returnCode;
}
