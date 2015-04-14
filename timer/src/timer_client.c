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

/*
 * Create a timer expiry message, for use with the
 * sendStartTimer() function.
 * 
 * pExpiryMsg         a pointer to the space where
 *                    the message will be created.
 * msgType            the msgType for the timer expiry
 *                    message.
 * pMsgBody           a pointer to the body of the timer
 *                    expiry message.
 * msgBodyLength      the length of the thing that pMsgBody
 *                    points to.
 * 
 * @return            true if the message send is
 *                    is successful and the response
 *                    message indicates success,
 *                    otherwise false.
 */
void createTimerExpiryMsg (ShortMsg *pExpiryMsg, MsgType msgType, void *pMsgBody, UInt16 msgBodyLength)
{
    ASSERT_PARAM (pExpiryMsg != PNULL, (unsigned long) pExpiryMsg);
    ASSERT_PARAM (msgBodyLength <= MAX_SHORT_MSG_BODY_LENGTH, msgBodyLength);

    pExpiryMsg->msgType = msgType;
    if (pMsgBody != PNULL)
    {
        memcpy (pExpiryMsg->msgBody, pMsgBody, msgBodyLength);
    }
    pExpiryMsg->msgLength = msgBodyLength + sizeof (pExpiryMsg->msgType);
}

/*
 * Send a Start Timer message to the Timer Server.
 * 
 * expiryDeciSeconds  the timer duration.
 * id                 an id for the timer, only
 *                    required if the timer is ever
 *                    to be stopped.
 * sourcePort         the port used by the sending
 *                    task.
 * pExpiryMsg         a pointer to the message
 *                    that will be sent when the timer
 *                    expires.  Use the createTimerExpiryMsg()
 *                    to create this message.  Cannot
 *                    be PNULL.  May be destroyed by the
 *                    sending task when this function returns.
 *  
 * @return            true if the message send is
 *                    is successful and the response
 *                    message indicates success,
 *                    otherwise false.
 */
Bool sendStartTimer (UInt32 expiryDeciSeconds, TimerId id, SInt32 sourcePort, ShortMsg *pExpiryMsg)
{
    TimerStartReq msg;
    
    ASSERT_PARAM (pExpiryMsg != PNULL, (unsigned long) pExpiryMsg);
    
    msg.expiryDeciSeconds = expiryDeciSeconds;
    msg.id = id;
    msg.sourcePort = sourcePort;
    memcpy (&(msg.expiryMsg), pExpiryMsg, sizeof (msg.expiryMsg));
    
    printDebug ("Starting %d decisecond timer, id %d, sourcePort %d, msg.expiryMsg.msgType 0x%x.\n", expiryDeciSeconds, id, sourcePort, msg.expiryMsg.msgType);
    return timerServerSend (TIMER_START_REQ, &msg, sizeof (msg));
}

/*
 * Send a Stopt Timer message to the Timer Server.
 * 
 * id                 the id of the timer to be stopped.
 * sourcePort         the port used by the sending
 *                    task.
 *  
 * @return            true if the message send is
 *                    is successful and the response
 *                    message indicates success,
 *                    otherwise false.
 */
Bool sendStopTimer (TimerId id, SInt32 sourcePort)
{   
    TimerStopReq msg;
    
    msg.id = id;
    msg.sourcePort = sourcePort;
    
    printDebug ("Stopping timer id %d, sourcePort %d.\n", id, sourcePort);
    return timerServerSend (TIMER_STOP_REQ, &msg, sizeof (msg));
}