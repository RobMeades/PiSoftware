/*
 * onewireserver.c
 * Builds my onewire.a library into a sockets server.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <rob_system.h>
#include <one_wire.h>
#include <messaging_server.h>
#include <one_wire_server.h>
#include <one_wire_msg_auto.h>

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
 * Handle a message that calls disableTestModeDS2408().
 * 
 * pMsgHeader    pointer to the message header.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionDisableTestModeDS2408 (OneWireReqMsgHeader *pMsgHeader, DisableTestModeDS2408Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = disableTestModeDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]));
    pSendMsgBody->oneWireResult.success = success;
    sendMsgBodyLength += sizeof (OneWireResult);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readControlRegisterDS2408().
 * 
 * pMsgHeader    pointer to the message header.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadControlRegisterDS2408 (OneWireReqMsgHeader *pMsgHeader, ReadControlRegisterDS2408Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 data = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readControlRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
    pSendMsgBody->oneWireResult.success = success;
    sendMsgBodyLength += sizeof (OneWireResult);
    pSendMsgBody->oneWireReadByte.data = data;
    sendMsgBodyLength += sizeof (OneWireReadByte);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that calls writeControlRegisterDS2408().
 * 
 * pMsgHeader    pointer to the message header.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * data          the data to write.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionWriteControlRegisterDS2408 (OneWireReqMsgHeader *pMsgHeader, WriteControlRegisterDS2408Cnf *pSendMsgBody, UInt8 data)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = writeControlRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), data);
    pSendMsgBody->oneWireResult.success = success;
    sendMsgBodyLength += sizeof (OneWireResult);
    
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
 * @return          none.
 */
static void doAction (MsgType receivedMsgType, UInt8 * pReceivedMsgBody, Msg *pSendMsg)
{
    OneWireReqMsgHeader msgHeader;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);
    
    /* Get the message header, which is at the start of the message body */
    memcpy (&msgHeader, pReceivedMsgBody, sizeof (msgHeader));

    /* We always respond with the same message type */
    pSendMsg->msgType = receivedMsgType;
    /* Fill in the length so far, will make it right for each message later */
    pSendMsg->msgLength = OFFSET_TO_MSG_BODY;
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        case DISABLE_TEST_MODE_DS2408:
        {
            pSendMsg->msgLength += actionDisableTestModeDS2408 (&msgHeader, (DisableTestModeDS2408Cnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case READ_CONTROL_REGISTER_DS2408:
        {
            pSendMsg->msgLength += actionReadControlRegisterDS2408 (&msgHeader, (ReadControlRegisterDS2408Cnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case WRITE_CONTROL_REGISTER_DS2408:
        {
            UInt8 data = ((WriteControlRegisterDS2408Req *) pReceivedMsgBody)->oneWireWriteByte.data;
            pSendMsg->msgLength += actionWriteControlRegisterDS2408 (&msgHeader, (WriteControlRegisterDS2408Cnf *) &(pSendMsg->msgBody[0]), data);
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (receivedMsgType);   
        }
        break;
    }
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    ServerReturnCode returnCode;

    returnCode = runMessagingServer (ONE_WIRE_SERVER_PORT);
        
    return returnCode;
}

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
 * @return        always SERVER_SUCCESS_KEEP_RUNNING.
 */
ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg *pSendMsg)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
    
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);

    /* Check the type */
    printProgress ("Server: received message of length %d and type %d.\n", pReceivedMsg->msgLength, pReceivedMsg->msgType);    
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_ONE_WIRE_MSG, pReceivedMsg->msgType);
    
    /* Do the thang */
    doAction (pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
        
    return returnCode;
}