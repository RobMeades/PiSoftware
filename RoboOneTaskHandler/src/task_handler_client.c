/*
 * Access functions.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <task_handler_types.h>
#include <task_handler_server.h>
#include <task_handler_msg_auto.h>
#include <task_handler_client.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * EXTERNS
 */
extern Char *pgTaskHandlerMessageNames[];

/*
 * GLOBALS - prefixed with g
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Send a message to the task handler server and
 * get the response back.
 * 
 * msgType               the message type to send.
 * pSendMsgBody          pointer to the body of the
 *                       REquest message to send.
 *                       May be PNULL.
 * sendMsgBodyLength     the length of the data that
 *                       pSendMsg points to.
 * 
 * @return           true if the message send/receive
 *                   is successful and the response
 *                   message indicates success,
 *                   otherwise false.
 */
Bool taskHandlerServerSendReceive (TaskHandlerMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength)
{
    ClientReturnCode returnCode;
    Bool success = false;
    Msg *pSendMsg;
    Msg *pReceivedMsg;
    UInt16 receivedMsgBodyLength = 0;

    ASSERT_PARAM (msgType < MAX_NUM_TASK_HANDLER_MSGS, (unsigned long) msgType);
    ASSERT_PARAM (sendMsgBodyLength <= MAX_MSG_BODY_LENGTH, sendMsgBodyLength);

    pSendMsg = malloc (sizeof (*pSendMsg));
    
    if (pSendMsg != PNULL)
    {
        pReceivedMsg = malloc (sizeof (*pReceivedMsg));
        
        if (pReceivedMsg != PNULL)
        {
            /* Put in the bit before the body */
            pSendMsg->msgLength = 0;
            pSendMsg->msgType = msgType;
            pSendMsg->msgLength += sizeof (pSendMsg->msgType);
                        
            /* Put any stuff to send */
            if (pSendMsgBody != PNULL)
            {
                memcpy (&pSendMsg->msgBody[0], pSendMsgBody, sendMsgBodyLength);
            }
            pSendMsg->msgLength += sendMsgBodyLength;
            
            pReceivedMsg->msgLength = 0;
    
            printDebug ("TH Client: sending message %s, length %d, hex dump:\n", pgTaskHandlerMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
            printHexDump ((UInt8 *) pSendMsg, pSendMsg->msgLength + 1);
            returnCode = runMessagingClient ((SInt32) atoi (TASK_HANDLER_SERVER_PORT_STRING), pSendMsg, pReceivedMsg);
                    
            printDebug ("TH Client: message system returnCode: %d\n", returnCode);
            /* This code makes assumptions about packing (i.e. that it's '1' and that the
             * Bool 'success' is at the start of the body) so be careful */
            if (returnCode == CLIENT_SUCCESS && (pReceivedMsg->msgLength > sizeof (pReceivedMsg->msgType)))
            { 
                /* Check the Bool 'success' at the start of the message body */
                receivedMsgBodyLength = pReceivedMsg->msgLength - sizeof (pReceivedMsg->msgType);
                printDebug ("TH Client: receivedMsgBodyLength: %d\n", receivedMsgBodyLength);
                printHexDump ((UInt8 *) pReceivedMsg, pReceivedMsg->msgLength + 1);
                if (receivedMsgBodyLength >= sizeof (Bool))
                {
                    printDebug ("TH Client: success field: %d\n", (Bool) pReceivedMsg->msgBody[0]);
                    success = (Bool) pReceivedMsg->msgBody[0];
                }
            }
            free (pReceivedMsg);
        }
        free (pSendMsg);
    }

    return success;
}