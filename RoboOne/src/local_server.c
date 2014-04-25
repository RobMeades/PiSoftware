/*
 * A local server for RoboOne itself, used
 * to receive task progress indications.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <task_handler_types.h>
#include <task_handler_server.h>
#include <task_handler_msg_auto.h>

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
 * Handle a response from the Hindbrain.
 * 
 * pTaskInd  pointer to the Hindbrain Direct
 *           progress indication message.
 */
static void handleHDTaskInd (RoboOneHDTaskInd *pHDTaskInd)
{
    Char displayBuffer[MAX_O_STRING_LENGTH];
    
    ASSERT_PARAM (pHDTaskInd != PNULL, (unsigned long) pHDTaskInd);
    
    switch (pHDTaskInd->result)
    {
        case HD_RESULT_SUCCESS:
        {
            printDebug ("RO Server, HD Protocol: response '%s'.\n", removeCtrlCharacters (&(pHDTaskInd->string[0]), &(displayBuffer[0])));
        }
        break;
        case HD_RESULT_SEND_FAILURE:
        {
            printDebug ("RO Server, HD Protocol: failure to send task to Hindbrain.\n");
        }
        break;
        case HD_RESULT_GENERAL_FAILURE:
        {
            printDebug ("RO Server, HD Protocol: general failure.\n");
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (pHDTaskInd->result);            
        }
        break;
    }
}

/*
 * Handle an indication of task progress.
 * 
 * pTaskInd  pointer to the task indication
 *           to be handled.
 */
static void handleTaskInd (RoboOneTaskInd *pTaskInd)
{
    ASSERT_PARAM (pTaskInd != PNULL, (unsigned long) pTaskInd);

    switch (pTaskInd->body.protocol)
    {
        /* There is only one protocol at the moment */
        case TASK_PROTOCOL_HD:
        {
            handleHDTaskInd ((RoboOneHDTaskInd *) &(pTaskInd->body.detail.hdInd));
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (pTaskInd->body.protocol);   
        }
        break;
    }
}

/*
 * Handle the received message and implement the action.
 * 
 * receivedMsgType  the msgType, extracted from the
 *                  received mesage.
 * pReceivedMsgBody pointer to the body part of the
 *                  received message.
 * 
 * @return          Always SERVER_SUCCESS_KEEP_RUNNING.
 */
static ServerReturnCode doAction (TaskHandlerMsgType receivedMsgType, UInt8 * pReceivedMsgBody)
{
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        case TASK_HANDLER_TASK_IND:
        {
            handleTaskInd ((RoboOneTaskInd *) pReceivedMsgBody);
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (receivedMsgType);   
        }
        break;
    }

    return SERVER_SUCCESS_KEEP_RUNNING;
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
    
    /* This server never responds with anything */
    pSendMsg->msgLength = 0;

    printDebug ("RO Server received message %s, length %d.\n", pgTaskHandlerMessageNames[pReceivedMsg->msgType], pReceivedMsg->msgLength);
    printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);    
    
    /* Do the thang */
    returnCode = doAction ((TaskHandlerMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody);
    
    return returnCode;
}
