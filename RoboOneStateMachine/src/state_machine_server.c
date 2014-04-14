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
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineServerStart (StateMachineServerStartCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    transitionToInit (&(pSendMsgBody->stateMachineMsgHeader.state));
    
    return sendMsgSpecificsLength;
}

/*
 * Handle a message that will cause us to exit.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineServerStop (StateMachineServerStopCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    return sendMsgSpecificsLength;
}

/*
 * Handle an Init event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventInit (StateMachineEventInitCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    eventInitRoboOne (&(pSendMsgBody->stateMachineMsgHeader));
    
    return sendMsgSpecificsLength;
}

/*
 * Handle an InitFailure event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventInitFailure (StateMachineEventInitFailureCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    eventInitFailureRoboOne (&(pSendMsgBody->stateMachineMsgHeader));
    
    return sendMsgSpecificsLength;
}

/*
 * Handle a TimerExpiry event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventTimerExpiry (StateMachineEventTimerExpiryCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    eventTimerExpiryRoboOne (&(pSendMsgBody->stateMachineMsgHeader));

    return sendMsgSpecificsLength;
}

/*
 * Handle a TasksAvailable event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventTasksAvailable (StateMachineEventTasksAvailableCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    eventTasksAvailableRoboOne (&(pSendMsgBody->stateMachineMsgHeader));

    return sendMsgSpecificsLength;
}

/*
 * Handle a NoTasksAvailable event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventNoTasksAvailable (StateMachineEventNoTasksAvailableCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    eventNoTasksAvailableRoboOne (&(pSendMsgBody->stateMachineMsgHeader));

    return sendMsgSpecificsLength;
}

/*
 * Handle a Mains Power Available event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventMainsPowerAvailable (StateMachineEventMainsPowerAvailableCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    eventMainsPowerAvailableRoboOne (&(pSendMsgBody->stateMachineMsgHeader));
    
    return sendMsgSpecificsLength;
}

/*
 * Handle an Insufficient Power event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventInsufficientPower (StateMachineEventInsufficientPowerCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    eventInsufficientPowerRoboOne (&(pSendMsgBody->stateMachineMsgHeader));

    return sendMsgSpecificsLength;
}

/*
 * Handle a Fully Charged event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventFullyCharged (StateMachineEventFullyChargedCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    eventFullyChargedRoboOne (&(pSendMsgBody->stateMachineMsgHeader));

    return sendMsgSpecificsLength;
}

/*
 * Handle a Shutdown event.
 * 
 * pSendMsgBody    pointer to the relevant message
 *                 type to fill in with a response,
 *                 which will be overlaid over the
 *                 body of the response message.
 *                 The standard StateMachineMsgHeader
 *                 should already have been filled in
 *                 (though this function may modify
 *                 its contents).
 * 
 * @return         the length of any specifics added
 *                 to the message body, over and above
 *                 the StateMachineMsgHeader.
 */
static UInt16 actionStateMachineEventShutdown (StateMachineEventShutdownCnf *pSendMsgBody)
{
    UInt16 sendMsgSpecificsLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    eventShutdownRoboOne (&(pSendMsgBody->stateMachineMsgHeader));
    
    return sendMsgSpecificsLength;
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
    
    pSendMsg->msgLength = 0;

    /* We always respond with the same message type */
    pSendMsg->msgType = (MsgType) receivedMsgType;
    
    /* Fill in the length so far, will make it right for each message later */
    pSendMsg->msgLength += sizeof (pSendMsg->msgType);
    
    /* Add the initial version of the outgoing header, it can then be modified in-place by the action functions */
    /* NOTE: this relies on the header being at the start of the msgBody */
    memcpy (&(pSendMsg->msgBody[0]), pReceivedMsgBody, sizeof (StateMachineMsgHeader));
    pSendMsg->msgLength += sizeof (StateMachineMsgHeader);
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        /*
         * Messages to do with the server itself
         */
        case STATE_MACHINE_SERVER_START:
        {
            pSendMsg->msgLength += actionStateMachineServerStart ((StateMachineServerStartCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_SERVER_STOP:
        {
            pSendMsg->msgLength += actionStateMachineServerStop ((StateMachineServerStopCnf *) &(pSendMsg->msgBody[0]));
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        /*
         * Messages to do with events
         */
        case STATE_MACHINE_EVENT_INIT:
        {
            pSendMsg->msgLength += actionStateMachineEventInit ((StateMachineEventInitCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_INIT_FAILURE:
        {
            pSendMsg->msgLength += actionStateMachineEventInitFailure ((StateMachineEventInitFailureCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_TIMER_EXPIRY:
        {
            pSendMsg->msgLength += actionStateMachineEventTimerExpiry ((StateMachineEventTimerExpiryCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_TASKS_AVAILABLE:
        {
            pSendMsg->msgLength += actionStateMachineEventTasksAvailable ((StateMachineEventTasksAvailableCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_NO_TASKS_AVAILABLE:
        {
            pSendMsg->msgLength += actionStateMachineEventNoTasksAvailable ((StateMachineEventNoTasksAvailableCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_MAINS_POWER_AVAILABLE:
        {
            pSendMsg->msgLength += actionStateMachineEventMainsPowerAvailable ((StateMachineEventMainsPowerAvailableCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_INSUFFICIENT_POWER:
        {
            pSendMsg->msgLength += actionStateMachineEventInsufficientPower ((StateMachineEventInsufficientPowerCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_FULLY_CHARGED:
        {
            pSendMsg->msgLength += actionStateMachineEventFullyCharged ((StateMachineEventFullyChargedCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case STATE_MACHINE_EVENT_SHUTDOWN:
        {
            pSendMsg->msgLength += actionStateMachineEventShutdown ((StateMachineEventShutdownCnf *) &(pSendMsg->msgBody[0]));
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
