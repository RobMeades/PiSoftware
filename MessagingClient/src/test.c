/*
 * test.c
 * Stubs for messaging client testing.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <unistd.h>
#include <netinet/in.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <messaging_client.h>

/*
 * MANIFEST CONSTANTS
 */

#define SERVER_EXE "./test_server"
#define SERVER_PORT_STRING "5000"

/*
 * EXTERN
 */

extern int errno;

/*
 * STATIC FUNCTIONS
 */

/*
 * Check for differences between two messages.
 * 
 * pSendMsg       a pointer the message that was sent.
 * pReceivedMsg   a pointer to message echoed back.
 * 
 * @return        true if the check passed, otherwise
 *                false.
 */
static Bool checkReceivedMsgContents (Msg *pSendMsg, Msg *pReceivedMsg)
{
    Bool success = true;
    UInt32 i;

    if (pReceivedMsg->msgLength != pSendMsg->msgLength)
    {
        success = false;
        printDebug ("\nSend message length (%d) and received message length (%d) are different.\n", pSendMsg->msgLength, pReceivedMsg->msgLength);            
    }
    else
    {
       if (pReceivedMsg->msgType != pSendMsg->msgType)
       {
           success = false;
           printDebug ("\nSend message type (%d) and received message type (%d) are different.\n", pSendMsg->msgType, pReceivedMsg->msgType);            
       }
       else
       {
           if (pSendMsg->msgLength > SIZE_OF_MSG_TYPE) /* Only check the body if there is one */
           {
               for (i = OFFSET_TO_MSG_BODY; (i < pSendMsg->msgLength - SIZE_OF_MSG_TYPE) && success; i++)
               {
                   if (pSendMsg->msgBody[i] != pReceivedMsg->msgBody[i])
                   {
                       success = false;
                   }
               }
               
               if (!success)
               {
                   i--;
                   printDebug ("\nReceived message body differs from sent message body at byte %d, should be %d but is %d.\n", i, pSendMsg->msgBody[i], pReceivedMsg->msgBody[i]);                                       
               }
           }
       }
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
    pid_t serverPID;
    SInt32 oneWireServerPort = -1;
    
    setDebugPrintsOn();
    setProgressPrintsOn();
    
    printProgress ("This will test that the messaging client and server libraries talk.\n");
    printProgress ("Make sure that %s is present 'cos we'll be running it.\n", SERVER_EXE);

    oneWireServerPort = atoi (SERVER_PORT_STRING);
    
    /* Spawn a child that will become the One Wire server. */
    serverPID = fork();
    
    if (serverPID == 0)
    {
        /* Start OneWire server process on port oneWireServerPort */
        static char *argv[]={SERVER_EXE, SERVER_PORT_STRING, PNULL};
        
        execv (SERVER_EXE, argv);
        printDebug ("Couldn't launch %s, err: %s\n", SERVER_EXE, strerror (errno));
    }
    else
    { /* Parent process */
      /* Setup what's necessary for OneWire bus stuff */
        int serverStatus;
        Bool returnCode;
        Msg *pSendMsg;
        Msg *pReceivedMsg;
        SInt32 i; /* Deliberately signed */
        UInt32 x;

        /* Wait for the server to start */
        usleep (SERVER_START_DELAY_PI_US);
        
        /* Check that it is running (looks strange but it is the standard method apparently) */
        if (kill (serverPID, 0) == 0)
        {
            pSendMsg = malloc (sizeof (Msg));
            
            if (pSendMsg != PNULL)
            {
                /* Send all the possible message lengths,
                 * ending with a zero length message which
                 * will cause the test server to exit */
                for (i = MAX_MSG_LENGTH; i >= 0; i--)
                {
                    printDebug ("+%d", i);
                    pSendMsg->msgLength = (MsgLength) i;            
                    pSendMsg->msgType = (MsgType) (i - 1);
                    
                    for (x = 0; x < i; x++)
                    {
                        pSendMsg->msgBody[x] = x;
                    }
                    
                    pReceivedMsg = malloc (sizeof(Msg));
                    
                    if (pReceivedMsg != PNULL)
                    {
                        /* Send the message and get a response */
                        if (i > 0)
                        {
                            returnCode = runMessagingClient (oneWireServerPort, pSendMsg, pReceivedMsg);
                        }
                        else
                        {
                            returnCode = runMessagingClient (oneWireServerPort, pSendMsg, PNULL); /* No echo will result for a zero length message */                    
                        }
                        
                        if (returnCode == CLIENT_SUCCESS)
                        {
                            if (i > 0)
                            {                            
                                printDebug ("-%d", i);
                                if (!checkReceivedMsgContents (pSendMsg, pReceivedMsg))
                                {
                                    success = false;;
                                }
                            }
                        }
                        else
                        {
                            success = false;;
                        }
                        
                        free (pReceivedMsg);
                    }        
                    else
                    {
                        success = false;;
                        printDebug ("Failed to get memory to receive test message.\n");            
                    }            
                }
                
                free (pSendMsg);
            }
            else
            {
                success = false;
                printDebug ("Failed to get memory to send test message.\n");
            }
            
            /* Wait for server to exit */
            if (waitpid (serverPID, &serverStatus, 0) >= 0)
            {
                printDebug ("\nServer process exited, status = 0x%x.\n", serverStatus);
            }
            else
            {
                printDebug ("\nFailed to wait for server to exit, error %s.\n", strerror (errno));            
            }
        }
        else
        {
            success = false;
            printDebug ("Server process failed to start.\n");            
        }
    }
    
    if (success)
    {
        printProgress ("\nTests PASSED, messaging server will now exit.\n");        
    }
    else
    {
        printProgress ("\nTests FAILED, messaging server will now exit.\n");
    }
        
    return success;
}