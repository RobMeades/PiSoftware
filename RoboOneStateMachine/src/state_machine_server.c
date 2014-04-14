/*
 * state_machine_server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <state_machine_interface.h>
#include <state_machine_public.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_events.h>
#include <init_state.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * Handle a message that will cause us to start.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineServerStart (RoboOneContext *pRoboOneContext, StateMachineServerStartCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    transitionToInit (&(pRoboOneContext->state));
    
    memcpy (&(pSendMsgBody->stateMachineMsgHeader.state), &(pRoboOneContext->state), sizeof (pSendMsgBody->stateMachineMsgHeader.state));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that will cause us to exit.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineServerStop (RoboOneContext *pRoboOneContext, StateMachineServerStopCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle an Init event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventInit (RoboOneContext *pRoboOneContext, StateMachineEventInitCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    eventInitRoboOne (pRoboOneContext);
    
    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle an InitFailure event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventInitFailure (RoboOneContext *pRoboOneContext, StateMachineEventInitFailureCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    eventInitFailureRoboOne (pRoboOneContext);

    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle a TimerExpiry event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventTimerExpiry (RoboOneContext *pRoboOneContext, StateMachineEventTimerExpiryCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    eventTimerExpiryRoboOne (pRoboOneContext);

    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle a TasksAvailable event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventTasksAvailable (RoboOneContext *pRoboOneContext, StateMachineEventTasksAvailableCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    eventTasksAvailableRoboOne (pRoboOneContext);

    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle a NoTasksAvailable event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventNoTasksAvailable (RoboOneContext *pRoboOneContext, StateMachineEventNoTasksAvailableCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    eventNoTasksAvailableRoboOne (pRoboOneContext);

    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle a Mains Power Available event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventMainsPowerAvailable (RoboOneContext *pRoboOneContext, StateMachineEventMainsPowerAvailableCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);

    eventMainsPowerAvailableRoboOne (pRoboOneContext);
    
    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle an Insufficient Power event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventInsufficientPower (RoboOneContext *pRoboOneContext, StateMachineEventInsufficientPowerCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    eventInsufficientPowerRoboOne (pRoboOneContext);

    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle a Fully Charged event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventFullyCharged (RoboOneContext *pRoboOneContext, StateMachineEventFullyChargedCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);

    eventFullyChargedRoboOne (pRoboOneContext);

    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
    return sendMsgBodyLength;
}

/*
 * Handle a Shutdown event.
 * 
 * pRoboOneContext pointer to context information.
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 * 
 * @return         the length of the message body
 *                 to send back.
 */
static UInt16 actionStateMachineEventShutdown (RoboOneContext *pRoboOneContext, StateMachineEventShutdownCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pRoboOneContext != PNULL, (unsigned long) pRoboOneContext);
    
    eventShutdownRoboOne (pRoboOneContext);

    memcpy (&(pSendMsgBody->stateMachineMsgHeader), pRoboOneContext, sizeof (pSendMsgBody->stateMachineMsgHeader));
    sendMsgBodyLength += sizeof (pSendMsgBody->stateMachineMsgHeader);
    
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
    StateMachineMsgHeader stateMachineMsgHeader;
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);
    
    pSendMsg->msgLength = 0;
    /* Get the message header, which is at the start of the message body */
    memcpy (&stateMachineMsgHeader, pReceivedMsgBody, sizeof (stateMachineMsgHeader));

    /* We always respond with the same message type */
    pSendMsg->msgType = (MsgType) receivedMsgType;
    /* Fill in the length so far, will make it right for each message later */
    pSendMsg->msgLength += sizeof (pSendMsg->msgType);
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        /*
         * Messages to do with the server itself
         */
        case STATE_MACHINE_SERVER_START:
        {
            pSendMsg->msgLength += actionStateMachineServerStart (&stateMachineMsgHeader, (StateMachineServerStartCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_SERVER_STOP:
        {
            pSendMsg->msgLength += actionStateMachineServerStop (&stateMachineMsgHeader, (StateMachineServerStopCnf *) &(pSendMsg->msgBody[0]));
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        /*
         * Messages to do with events
         */
        case STATE_MACHINE_EVENT_INIT:
        {
            pSendMsg->msgLength += actionStateMachineEventInit (&stateMachineMsgHeader, (StateMachineEventInitCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_INIT_FAILURE:
        {
            pSendMsg->msgLength += actionStateMachineEventInitFailure (&stateMachineMsgHeader, (StateMachineEventInitFailureCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_TIMER_EXPIRY:
        {
            pSendMsg->msgLength += actionStateMachineEventTimerExpiry (&stateMachineMsgHeader, (StateMachineEventTimerExpiryCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_TASKS_AVAILABLE:
        {
            pSendMsg->msgLength += actionStateMachineEventTasksAvailable (&stateMachineMsgHeader, (StateMachineEventTasksAvailableCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_NO_TASKS_AVAILABLE:
        {
            pSendMsg->msgLength += actionStateMachineEventNoTasksAvailable (&stateMachineMsgHeader, (StateMachineEventNoTasksAvailableCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_MAINS_POWER_AVAILABLE:
        {
            pSendMsg->msgLength += actionStateMachineEventMainsPowerAvailable (&stateMachineMsgHeader, (StateMachineEventMainsPowerAvailableCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_INSUFFICIENT_POWER:
        {
            pSendMsg->msgLength += actionStateMachineEventInsufficientPower (&stateMachineMsgHeader, (StateMachineEventInsufficientPowerCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_FULLY_CHARGED:
        {
            pSendMsg->msgLength += actionStateMachineEventFullyCharged (&stateMachineMsgHeader, (StateMachineEventFullyChargedCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_SHUTDOWN:
        {
            pSendMsg->msgLength += actionStateMachineEventShutdown (&stateMachineMsgHeader, (StateMachineEventShutdownCnf *) &(pSendMsg->msgBody[0]));
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
    
    /* Do the thang */
    returnCode = doAction ((StateMachineMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
        
    return returnCode;
}
