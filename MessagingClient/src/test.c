/*
 * test.c
 * Stubs for messaging client testing.
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
#include <messaging_client.h>

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
        printProgress ("\nSend message length (%d) and received message length (%d) are different.\n", pSendMsg->msgLength, pReceivedMsg->msgLength);            
    }
    else
    {
       if (pReceivedMsg->msgType != pSendMsg->msgType)
       {
           success = false;
           printProgress ("\nSend message type (%d) and received message type (%d) are different.\n", pSendMsg->msgType, pReceivedMsg->msgType);            
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
                   printProgress ("\nReceived message body differs from sent message body at byte %d, should be %d but is %d.\n", i, pSendMsg->msgBody[i], pReceivedMsg->msgBody[i]);                                       
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
    ClientReturnCode returnCode;
    Msg *pSendMsg;
    Msg *pReceivedMsg;
    SInt32 i; /* Deliberately signed */
    UInt32 x;

    printProgress ("This will test that the messaging client and server libraries talk.\n");
    printProgress ("Make sure that testserver is running (./testserver&) first.\n");
    pSendMsg = malloc (sizeof(Msg));
    
    if (pSendMsg != PNULL)
    {
        /* Send all the possible message lengths,
         * ending with a zero length message which
         * will cause the test server to exit */
        for (i = MAX_MSG_LENGTH; i >= 0; i--)
        {
            printProgress ("+%d", i);
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
                    returnCode = runMessagingClient (5000, pSendMsg, pReceivedMsg);
                }
                else
                {
                    returnCode = runMessagingClient (5000, pSendMsg, PNULL); /* No echo will result for a zero length message */                    
                }
                
                if (returnCode == CLIENT_SUCCESS)
                {
                    if (i > 0)
                    {
                        Bool success;
                        
                        printProgress ("-%d", i);
                        success = checkReceivedMsgContents (pSendMsg, pReceivedMsg);
                        if (!success)
                        {
                            returnCode = CLIENT_ERR_GENERAL_FAILURE;                    
                        }
                    }
                    else
                    {
                        printProgress ("\nTests passed, messaging server will now exit.\n");
                    }
                }
                
                free (pReceivedMsg);
            }        
            else
            {
                returnCode = CLIENT_ERR_GENERAL_FAILURE;
                printProgress ("Failed to get memory to receive test message.\n");            
            }            
        }
        
        free (pSendMsg);
    }
    else
    {
        returnCode = CLIENT_ERR_GENERAL_FAILURE;
        printProgress ("Failed to get memory to send test message.\n");
    }
    
    return returnCode;
}