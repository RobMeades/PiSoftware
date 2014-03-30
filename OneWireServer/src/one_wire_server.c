/*
 * onewireserver.c
 * Builds my onewire.a library into a sockets server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 * Handle a message that calls oneWireStartBus().
 * 
 * pSerialPortString the string representation of the
 *                   serial port, e.g. "/dev/USBSerial".
 * pSendMsgBody      pointer to the relevant message
 *                   type to fill in with a response,
 *                   which will be overlaid over the
 *                   body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionOneWireStartBus (Char *pSerialPortString, OneWireStartBusCnf *pSendMsgBody)
{
    SInt32 port;
    UInt16 sendMsgBodyLength = 0;
    
    port = oneWireStartBus (pSerialPortString);
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->port = port;
    sendMsgBodyLength += sizeof (pSendMsgBody->port);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that calls oneWireStopBus().
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
static UInt16 actionOneWireStopBus (MsgHeader *pMsgHeader, OneWireStopBusCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    oneWireStopBus (pMsgHeader->portNumber);
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that will cause us to exit.
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
static UInt16 actionOneWireServerExit (OneWireServerExitCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that calls oneWireFindAllDevices().
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
static UInt16 actionOneWireFindAllDevices (MsgHeader *pMsgHeader, OneWireFindAllDevicesCnf *pSendMsgBody)
{
    UInt8 numDevicesFound;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    numDevicesFound = oneWireFindAllDevices (pMsgHeader->portNumber, &pSendMsgBody->deviceList.address[0], MAX_DEVICES_TO_FIND);
    pSendMsgBody->success = true;
    pSendMsgBody->deviceList.numDevices = numDevicesFound; /* Yes, numDevicesFound can be bigger than MAX_DEVICES_TO_FIND */
    sendMsgBodyLength += sizeof (pSendMsgBody->deviceList.numDevices);
    if (numDevicesFound > MAX_DEVICES_TO_FIND)
    {
        numDevicesFound = MAX_DEVICES_TO_FIND;
    }
    sendMsgBodyLength += NUM_BYTES_IN_SERIAL_NUM * numDevicesFound;
    
    return sendMsgBodyLength;
}

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
static UInt16 actionDisableTestModeDS2408 (MsgHeader *pMsgHeader, DisableTestModeDS2408Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = disableTestModeDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]));
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
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
static UInt16 actionReadControlRegisterDS2408 (MsgHeader *pMsgHeader, ReadControlRegisterDS2408Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 data = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readControlRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->data = data;
    sendMsgBodyLength += sizeof (pSendMsgBody->data);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that calls writeControlRegisterDS2408().
 * 
 * pMsgHeader    pointer to the message header.
 * data          the byte to write.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionWriteControlRegisterDS2408 (MsgHeader *pMsgHeader, UInt8 data, WriteControlRegisterDS2408Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = writeControlRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), data);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
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
static ServerReturnCode doAction (OneWireMsgType receivedMsgType, UInt8 * pReceivedMsgBody, Msg *pSendMsg)
{
    MsgHeader msgHeader;
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);
    
    /* Get the message header, which is at the start of the message body */
    memcpy (&msgHeader, pReceivedMsgBody, sizeof (msgHeader));

    /* We always respond with the same message type */
    pSendMsg->msgType = (MsgType) receivedMsgType;
    /* Fill in the length so far, will make it right for each message later */
    pSendMsg->msgLength = OFFSET_TO_MSG_BODY;
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        case ONE_WIRE_START_BUS:
        {
            Char * pSerialPortString= &((OneWireStartBusReq *) &pReceivedMsgBody)->serialPortString[0];
            pSendMsg->msgLength += actionOneWireStartBus (pSerialPortString, (OneWireStartBusCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case ONE_WIRE_STOP_BUS:
        {
            pSendMsg->msgLength += actionOneWireStopBus (&msgHeader, (OneWireStopBusCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case ONE_WIRE_SERVER_EXIT:
        {
            pSendMsg->msgLength += actionOneWireServerExit ((OneWireServerExitCnf *) &(pSendMsg->msgBody[0]));
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        case ONE_WIRE_FIND_ALL_DEVICES:
        {
            pSendMsg->msgLength += actionOneWireFindAllDevices (&msgHeader, (OneWireFindAllDevicesCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
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
            UInt8 data = ((WriteControlRegisterDS2408Req *) pReceivedMsgBody)->data;
            pSendMsg->msgLength += actionWriteControlRegisterDS2408 (&msgHeader, data, (WriteControlRegisterDS2408Cnf *) &(pSendMsg->msgBody[0]));
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
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_ONE_WIRE_MSG, pReceivedMsg->msgType);
    printProgress ("Server received msgType %d\n", pReceivedMsg->msgType);
    
    /* Do the thang */
    returnCode = doAction ((OneWireMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
        
    return returnCode;
}