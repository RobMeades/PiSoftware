/*
 * state_machine_server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <state_machine_events.h>
#include <init_state.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * GLOBALS - prefixed with g
 */

RoboOneContext *pgRoboOneContext = PNULL;


/*
 * STATIC FUNCTIONS
 */

/*
 * Handle a message that will cause us to start.
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionStateMachineServerStart (StateMachineServerStartCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    pgRoboOneContext = malloc (sizeof (RoboOneContext));
    
    pSendMsgBody->success = false;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
    if (pgRoboOneContext != PNULL)
    {
        transitionToInit (&(pgRoboOneContext->state));
        pSendMsgBody->success = true;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that will cause us to exit.
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionStateMachineServerStop (StateMachineServerStopCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    free (pgRoboOneContext);
    pgRoboOneContext = PNULL;

    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that will send back
 * the contents of the context to the
 * sender.
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionStateMachineServerGetContext (StateMachineServerGetContextCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    pSendMsgBody->roboOneContextContainer.isValid = false;
    if (pgRoboOneContext != PNULL)
    {
        memcpy (&(pSendMsgBody->roboOneContextContainer.roboOneContext), pgRoboOneContext, sizeof (pSendMsgBody->roboOneContextContainer.roboOneContext));
        pSendMsgBody->roboOneContextContainer.isValid = true;
        sendMsgBodyLength += sizeof (pSendMsgBody->roboOneContextContainer);
    }    
    
    return sendMsgBodyLength;
}

/*
 * Handle the received message and implement the action.
 * 
 * receivedMsgType  the msgType, extracted from the
 *                  received mesage.
 * pReceivedMsgBody pointer to the body part of the
 *                  received message.
 * pSendMsg         pointer to a message that we
 *                  can fill in with the response.
 * 
 * @return          SERVER_SUCCESS_KEEP_RUNNING unless
 *                  exitting in which case SERVER_EXIT_NORMALLY.
 */
static ServerReturnCode doAction (StateMachineMsgType receivedMsgType, UInt8 * pReceivedMsgBody, Msg *pSendMsg)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);
    
    /* Assume no response by default */
    pSendMsg->msgLength = 0;

    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        /*
         * Messages to do with the server itself
         */
        case STATE_MACHINE_SERVER_START:
        {
            pSendMsg->msgType = STATE_MACHINE_SERVER_START;
            pSendMsg->msgLength += sizeof (pSendMsg->msgType);
            pSendMsg->msgLength += actionStateMachineServerStart ((StateMachineServerStartCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_SERVER_STOP:
        {
            pSendMsg->msgType = STATE_MACHINE_SERVER_STOP;
            pSendMsg->msgLength += sizeof (pSendMsg->msgType);
            pSendMsg->msgLength += actionStateMachineServerStop ((StateMachineServerStopCnf *) &(pSendMsg->msgBody[0]));
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        case STATE_MACHINE_SERVER_GET_CONTEXT:
        {
            pSendMsg->msgType = STATE_MACHINE_SERVER_GET_CONTEXT;
            pSendMsg->msgLength += sizeof (pSendMsg->msgType);
            pSendMsg->msgLength += actionStateMachineServerGetContext ((StateMachineServerGetContextCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        /*
         * Messages to do with events
         */
        case STATE_MACHINE_EVENT_INIT:
        {
            eventInitRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_INIT_FAILURE:
        {
            eventInitFailureRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_TIMER_EXPIRY:
        {
            eventTimerExpiryRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_TASKS_AVAILABLE:
        {
            eventTasksAvailableRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_NO_TASKS_AVAILABLE:
        {
            eventNoTasksAvailableRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_MAINS_POWER_AVAILABLE:
        {
            eventMainsPowerAvailableRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_INSUFFICIENT_POWER:
        {
            eventInsufficientPowerRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_FULLY_CHARGED:
        {
            eventFullyChargedRoboOne (pgRoboOneContext);
        }
        break;
        case STATE_MACHINE_EVENT_SHUTDOWN:
        {
            eventShutdownRoboOne (pgRoboOneContext);
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (receivedMsgType);   
        }
        break;
    }
    
    return returnCode;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Handle a whole message received from the client
 * and send back a response.
 * 
 * pReceivedMsg   a pointer to the buffer containing the
 *                incoming message.
 * pSendMsg       a pointer to a message buffer to put
 *                the response into. Not touched if return
 *                code is a failure one.
 * 
 * @return        whatever doAction() returns.
 */
ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg *pSendMsg)
{
    ServerReturnCode returnCode;
    
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);

    /* Check the type */
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_STATE_MACHINE_MSGS, pReceivedMsg->msgType);
    
    printDebug ("SM Server received message type %d, length %d.\n", pReceivedMsg->msgType, pReceivedMsg->msgLength);
    /* Do the thang */
    returnCode = doAction ((StateMachineMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
        
    return returnCode;
}
