/*
 * task_handler_server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <task_handler_types.h>
#include <task_handler_server.h>
#include <task_handler_msg_auto.h>
#include <task_handler_client.h>
#include <task_handler.h>

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
static ServerReturnCode doAction (TaskHandlerMsgType receivedMsgType, UInt8 * pReceivedMsgBody, Msg *pSendMsg)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
    Bool success = false;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);
    
    /* We always respond with the same message type */
    pSendMsg->msgType = (MsgType) receivedMsgType;
    /* Fill in the length so far */
    pSendMsg->msgLength += sizeof (pSendMsg->msgType);
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        /*
         * Messages to do with the server itself
         */
        case TASK_HANDLER_SERVER_START:
        {
            success = initTaskList();
        }
        break;
        case TASK_HANDLER_SERVER_STOP:
        {
            success = clearTaskList();
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        /*
         * Messages to do with tasks
         */
        case TASK_HANDLER_NEW_TASK:
        {
            success = handleNewTaskReq ((RoboOneTaskReq *) pReceivedMsgBody);
        }
        break;
        case TASK_HANDLER_TICK:
        {
            success = tickTaskHandler();
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (receivedMsgType);   
        }
        break;
    }

    /* Note: the following code assumes packing of 1 and that the start of a Cnf message contains the Bool 'success' */
    pSendMsg->msgBody[0] = success;
    pSendMsg->msgLength += sizeof (Bool);
    
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
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_TASK_HANDLER_MSGS, pReceivedMsg->msgType);
    
    printDebug ("TH Server received message %s, length %d.\n", pgTaskHandlerMessageNames[pReceivedMsg->msgType], pReceivedMsg->msgLength);
    printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);
    /* Do the thang */
    returnCode = doAction ((TaskHandlerMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
    if (pSendMsg->msgLength > 0)
    {
        printDebug ("TH Server responding with message %s, length %d.\n", pgTaskHandlerMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
    }
        
    return returnCode;
}
