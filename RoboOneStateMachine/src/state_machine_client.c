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
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * EXTERNS
 */
extern Char *pgStateMachineMessageNames[];

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
 * Send a message to the state machine server and
 * get the response back.
 * 
 * sendMsgType       the message type to send.
 * pSendMsgBody      pointer to the body of the
 *                   send REquest message beyond the
 *                   May be PNULL.
 * sendMsgBodyLength the length of the bit that
 *                   pSendMsgBody points to.
 * pReceivedMsgType  pointer to a place to put the
 *                   received msg type.  May be
 *                   PNULL, in which case this
 *                   function does not wait for a
 *                   reply from the sender.
 * pReceivedMsgBody  pointer to a place to store
 *                   the body of the received CNF
 *                   message.  May be PNULL.
 * 
 * @return           true if the message send/receive
 *                   is successful and the response
 *                   message indicates success,
 *                   otherwise false.
 */
Bool stateMachineServerSendReceive (StateMachineMsgType sendMsgType, void *pSendMsgBody, UInt16 sendMsgBodyLength, StateMachineMsgType *pReceivedMsgType, void *pReceivedMsgBody)
{
    ClientReturnCode returnCode;
    Bool success = false;
    Msg *pSendMsg;
    Msg *pReceivedMsg = PNULL;
    UInt16 receivedMsgBodyLength = 0;

    ASSERT_PARAM (sendMsgType < MAX_NUM_STATE_MACHINE_MSGS, sendMsgType);
    ASSERT_PARAM (sendMsgBodyLength <= MAX_MSG_BODY_LENGTH, sendMsgBodyLength);

    pSendMsg = malloc (sizeof (*pSendMsg));
    
    if (pSendMsg != PNULL)
    {
        if (pReceivedMsgType != PNULL)
        {
            /* The caller wants a reply so make space for the message */
            pReceivedMsg = malloc (sizeof (*pReceivedMsg));
            if (pReceivedMsg != PNULL)
            {
                pReceivedMsg->msgLength = 0;
            }
        }
        
        if ((pSendMsg != PNULL) && (pReceivedMsgType == PNULL || pReceivedMsg != PNULL))
        {
            /* Put in the bit before the body */
            pSendMsg->msgLength = 0;
            pSendMsg->msgType = sendMsgType;
            pSendMsg->msgLength += sizeof (pSendMsg->msgType);
                        
            /* Put in the specifics */
            if (pSendMsgBody != PNULL)
            {
                memcpy (&(pSendMsg->msgBody[0]), pSendMsgBody, sendMsgBodyLength);
            }
            pSendMsg->msgLength += sendMsgBodyLength;
            
            printDebug ("SM Client: sending message %s, length %d, hex dump:\n",  pgStateMachineMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
            printHexDump (pSendMsg, pSendMsg->msgLength + 1);
            returnCode = runMessagingClient ((SInt32) atoi (STATE_MACHINE_SERVER_PORT_STRING), PNULL, pSendMsg, pReceivedMsg);
                    
            printDebug ("SM Client: message system returnCode: %d\n", returnCode);
            /* This code makes assumptions about packing (i.e. that it's '1') so be careful */
            if (returnCode == CLIENT_SUCCESS)
            {
                if (pReceivedMsg != PNULL)
                {
                    if (pReceivedMsg->msgLength >= sizeof (pReceivedMsg->msgType))
                    { 
                        success = true;
                        /* Pass back the message type */
                        *pReceivedMsgType = pReceivedMsg->msgType;
                        /* Pass back the body of the message */
                        receivedMsgBodyLength = pReceivedMsg->msgLength - sizeof (pReceivedMsg->msgType);
                        printDebug ("SM Client: received message %s, receivedMsgBodyLength: %d\n", pgStateMachineMessageNames[pReceivedMsg->msgType], receivedMsgBodyLength);
                        
                        ASSERT_PARAM (receivedMsgBodyLength <= MAX_MSG_BODY_LENGTH, receivedMsgBodyLength);
                        
                        if ((pReceivedMsgBody != PNULL) && (receivedMsgBodyLength > 0))
                        {
                            /* Copy out the body */
                            memcpy (pReceivedMsgBody, &(pReceivedMsg->msgBody[0]), receivedMsgBodyLength);
                            printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);        
                        }
                    }
                    /* Free the space for the received message */
                    free (pReceivedMsg);
                }
                else
                {
                    success = true;
                    printDebug ("SM Client not bothering to wait for a reply.\n");                
                }
            }
        }
        /* Free the space for the sent message */
        free (pSendMsg);
    }

    return success;
}