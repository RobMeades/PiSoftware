/*
 * OneWire bus handling thread for RoboOne.
 * This file encapsulates all the knowledge of
 * what is conected to what on the OneWire bus
 * so that nothing outside here should need to
 * know the contents of hw_config.h or the bus
 * state.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <rob_system.h>
#include <one_wire.h>
#include <ow_bus.h>
#include <hw_config.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <state_machine_interface.h>
#include <state_machine_public.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * EXTERN
 */

extern SInt32 gStateMachineServerPort;

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
    Bool success = true;
    Msg *pSendMsg;
    Msg *pReceivedMsg = PNULL;
    UInt16 receivedMsgBodyLength = 0;

    ASSERT_PARAM (gStateMachineServerPort >= 0, gStateMachineServerPort);
    ASSERT_PARAM (sendMsgType < MAX_NUM_STATE_MACHINE_MSGS, (unsigned long) sendMsgType);
    ASSERT_PARAM (sendMsgBodyLength <= MAX_MSG_BODY_LENGTH, sendMsgBodyLength);

    pSendMsg = malloc (sizeof (Msg));
    
    if (pSendMsg != PNULL)
    {
        if (pReceivedMsgType != PNULL)
        {
            /* The caller wants the body of the reply so make space for the message */
            pReceivedMsg = malloc (sizeof (Msg));
            if (pReceivedMsg != PNULL)
            {
                pReceivedMsg->msgLength = 0;
            }
            else
            {
                success = false;
            }
        }
        
        if (success)
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
            
            printDebug ("\nSM Client: sending message of type %d, length %d, hex dump:\n", pSendMsg->msgType, pSendMsg->msgLength);
            printHexDump ((UInt8 *) pSendMsg, pSendMsg->msgLength + 1);
            returnCode = runMessagingClient (gStateMachineServerPort, pSendMsg, pReceivedMsg);
                    
            printDebug ("SM Client: message system returnCode: %d\n", returnCode);
            /* This code makes assumptions about packing (i.e. that it's '1') so be careful */
            if (returnCode == CLIENT_SUCCESS)
            {
                if (pReceivedMsg->msgLength >= sizeof (pReceivedMsg->msgType))
                { 
                    /* Pass back the message type */
                    *pReceivedMsgType = pReceivedMsg->msgType;
                    if (pReceivedMsg != PNULL)
                    {
                        /* Pass back the body of the message */
                        receivedMsgBodyLength = pReceivedMsg->msgLength - sizeof (pReceivedMsg->msgType);
                        printDebug ("SM Client: received message type %d, receivedMsgBodyLength: %d\n", pReceivedMsg->msgType, receivedMsgBodyLength);
                        
                        ASSERT_PARAM (receivedMsgBodyLength <= MAX_MSG_BODY_LENGTH, receivedMsgBodyLength);
                        
                        if ((pReceivedMsgBody != PNULL) && (receivedMsgBodyLength > 0))
                        {
                            printHexDump ((UInt8 *) pReceivedMsg, pReceivedMsg->msgLength + 1);        
                            /* Copy out the body */
                            memcpy (pReceivedMsgBody, &(pReceivedMsg->msgBody[0]), receivedMsgBodyLength);
                        }
                    }
                    else
                    {
                        printDebug ("SM Client not bothering to wait for a reply.\n");                
                    }
                }
                else
                {
                    success = false;                
                }
            }
            else
            {
                success = false;                
            }
            
            /* Free the space for the message */
            free (pReceivedMsg);
        }
    }
    else
    {
        success = false;
    }

    return success;
}