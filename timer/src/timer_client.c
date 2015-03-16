/*
 * Utility functions for accessing the Battery Manager
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <timer_server.h>
#include <timer_msg_auto.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * EXTERNS
 */
extern Char *pgTimerMessageNames[];

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Send a message to the Timer Server.
 * 
 * msgType               the message type to send.
 * pSendMsgBody          pointer to the body of the
 *                       REquest message to send.
 *                       May be PNULL.
 * sendMsgBodyLength     the length of the data that
 *                       pSendMsg points to.
 * 
 * @return           true if the message send is
 *                   is successful and the response
 *                   message indicates success,
 *                   otherwise false.
 */
Bool timerServerSend (TimerMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength)
{
    ClientReturnCode returnCode;
    Bool success = false;
    Msg *pSendMsg;

    ASSERT_PARAM (msgType < MAX_NUM_TIMER_MSGS, msgType);
    ASSERT_PARAM (sendMsgBodyLength <= MAX_MSG_BODY_LENGTH, sendMsgBodyLength);

    pSendMsg = malloc (sizeof (*pSendMsg));
    
    if (pSendMsg != PNULL)
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
            
        printDebug ("T  Client: sending message %s, length %d, hex dump:\n", pgTimerMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
        printHexDump (pSendMsg, pSendMsg->msgLength + 1);
        returnCode = runMessagingClient ((SInt32) atoi (TIMER_SERVER_PORT_STRING), PNULL, pSendMsg, PNULL);
                    
        printDebug ("T  Client: message system returnCode: %d\n", returnCode);
        if (returnCode == CLIENT_SUCCESS)
        { 
            success = true;
        }
        free (pSendMsg);
    }

    return success;
}