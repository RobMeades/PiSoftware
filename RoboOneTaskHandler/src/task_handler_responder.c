/*
 * Send responses to clients that asked for progress indications on how their task is doing.
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
#include <task_handler_responder.h>

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
 * Send a message to a client of the task handler.
 * 
 * pHeader            the header containing details
 *                    of the destination for the
 *                    response.
 * msgType            the message type to send.
 * pSendMsgBody       pointer to the body of the
 *                    REquest message to send.
 *                    May be PNULL.
 * sendMsgBodyLength  the length of the data that
 *                    pSendMsg points to.
 * 
 * @return            true if the message send is
 *                    successful otherwise false.
 */
Bool taskHandlerResponder (RoboOneTaskReqHeader *pHeader, TaskHandlerMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength)
{
    ClientReturnCode returnCode;
    Bool success = false;
    Msg *pSendMsg;
    Char *pIpAddress = "127.0.0.1";

    ASSERT_PARAM (pHeader != PNULL, (unsigned long) pHeader);
    ASSERT_PARAM (msgType < MAX_NUM_TASK_HANDLER_MSGS, msgType);
    ASSERT_PARAM (sendMsgBodyLength <= MAX_MSG_BODY_LENGTH, sendMsgBodyLength);

    pSendMsg = malloc (sizeof (*pSendMsg));
    
    if (pSendMsg != PNULL)
    {
        if (pHeader->sourceServerIpAddressStringPresent)
        {
            pIpAddress = &(pHeader->sourceServerIpAddressString[0]);
        }

        /* Put in the bit before the body */
        pSendMsg->msgLength = 0;
        pSendMsg->msgType = msgType;
        pSendMsg->msgLength += sizeof (pSendMsg->msgType);
                    
        /* Put in any body to send */
        if (pSendMsgBody != PNULL)
        {
            memcpy (&pSendMsg->msgBody[0], pSendMsgBody, sendMsgBodyLength);
        }
        pSendMsg->msgLength += sendMsgBodyLength;
        
        printDebug ("TH Responder: sending message %s, length %d, to port %d, IP address %s, hex dump:\n", pgTaskHandlerMessageNames[pSendMsg->msgType], pSendMsg->msgLength, pHeader->sourceServerPort, pIpAddress);
        printHexDump (pSendMsg, pSendMsg->msgLength + 1);
        returnCode = runMessagingClient (pHeader->sourceServerPort, pIpAddress, pSendMsg, PNULL);                
        printDebug ("TH Responder: message system returnCode: %d\n", returnCode);
        if (returnCode == CLIENT_SUCCESS)
        {
            success = true;
        }

        free (pSendMsg);
    }

    return success;
}