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
    SInt32 serialPortNumber;
    UInt16 sendMsgBodyLength = 0;
    
    serialPortNumber = oneWireStartBus (pSerialPortString);
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->serialPortNumber = serialPortNumber;
    sendMsgBodyLength += sizeof (pSendMsgBody->serialPortNumber);
    
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
static UInt16 actionOneWireStopBus (OneWireMsgHeader *pMsgHeader, OneWireStopBusCnf *pSendMsgBody)
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
static UInt16 actionOneWireFindAllDevices (OneWireMsgHeader *pMsgHeader, OneWireFindAllDevicesCnf *pSendMsgBody)
{
    UInt8 numDevicesFound;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    numDevicesFound = oneWireFindAllDevices (pMsgHeader->portNumber, &pSendMsgBody->deviceList.address[0], MAX_DEVICES_TO_FIND);
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
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
 * Handle a message that calls oneWireAccessDevice().
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
static UInt16 actionOneWireAccessDevice (OneWireMsgHeader *pMsgHeader, OneWireAccessDeviceCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    pSendMsgBody->success = oneWireAccessDevice (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]));
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
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
static UInt16 actionDisableTestModeDS2408 (OneWireMsgHeader *pMsgHeader, DisableTestModeDS2408Cnf *pSendMsgBody)
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
 * Handle a message that calls any of the functions
 * that read a byte-length register.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pMsgHeader    pointer to the message header.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadByteRegister (OneWireMsgType msgType, OneWireMsgHeader *pMsgHeader, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 data = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
    	case READ_CONTROL_REGISTER_DS2408:
		{
    	    success = readControlRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
    	    ((ReadControlRegisterDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((ReadControlRegisterDS2408Cnf *) pSendMsgBody)->success);
    	    ((ReadControlRegisterDS2408Cnf *) pSendMsgBody)->data = data;
    	    sendMsgBodyLength += sizeof (((ReadControlRegisterDS2408Cnf *) pSendMsgBody)->data);
		}
    	break;
        case READ_PIO_LOGIC_STATE_DS2408:
		{
    	    success = readPIOLogicStateDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
    	    ((ReadPIOLogicStateDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((ReadPIOLogicStateDS2408Cnf *) pSendMsgBody)->success);
    	    ((ReadPIOLogicStateDS2408Cnf *) pSendMsgBody)->data = data;
    	    sendMsgBodyLength += sizeof (((ReadPIOLogicStateDS2408Cnf *) pSendMsgBody)->data);
		}
    	break;
        case READ_PIO_OUTPUT_LATCH_STATE_REGISTER_DS2408:
		{
    	    success = readPIOOutputLatchStateRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
    	    ((ReadPIOOutputLatchStateRegisterDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((ReadPIOOutputLatchStateRegisterDS2408Cnf *) pSendMsgBody)->success);
    	    ((ReadPIOOutputLatchStateRegisterDS2408Cnf *) pSendMsgBody)->data = data;
    	    sendMsgBodyLength += sizeof (((ReadPIOOutputLatchStateRegisterDS2408Cnf *) pSendMsgBody)->data);
		}
    	break;
        case READ_PIO_ACTIVITY_LATCH_STATE_REGISTER_DS2408:
        {
            success = readPIOActivityLatchStateRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
            ((ReadPIOActivityLatchStateRegisterDS2408Cnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((ReadPIOActivityLatchStateRegisterDS2408Cnf *) pSendMsgBody)->success);
            ((ReadPIOActivityLatchStateRegisterDS2408Cnf *) pSendMsgBody)->data = data;
            sendMsgBodyLength += sizeof (((ReadPIOActivityLatchStateRegisterDS2408Cnf *) pSendMsgBody)->data);
        }
        break;
        case READ_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408:
		{
    	    success = readCSChannelSelectionMaskRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
    	    ((ReadCSChannelSelectionMaskRegisterDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((ReadCSChannelSelectionMaskRegisterDS2408Cnf *) pSendMsgBody)->success);
    	    ((ReadCSChannelSelectionMaskRegisterDS2408Cnf *) pSendMsgBody)->data = data;
    	    sendMsgBodyLength += sizeof (((ReadCSChannelSelectionMaskRegisterDS2408Cnf *) pSendMsgBody)->data);
		}
    	break;
        case READ_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408:
		{
    	    success = readCSChannelPolaritySelectionRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data);
    	    ((ReadCSChannelPolaritySelectionRegisterDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((ReadCSChannelPolaritySelectionRegisterDS2408Cnf *) pSendMsgBody)->success);
    	    ((ReadCSChannelPolaritySelectionRegisterDS2408Cnf *) pSendMsgBody)->data = data;
    	    sendMsgBodyLength += sizeof (((ReadCSChannelPolaritySelectionRegisterDS2408Cnf *) pSendMsgBody)->data);
		}
    	break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that writes to any byte-length
 * register.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
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
static UInt16 actionWriteByteRegister (OneWireMsgType msgType, OneWireMsgHeader *pMsgHeader, UInt8 data, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
    	case WRITE_CONTROL_REGISTER_DS2408:
		{
		    success = writeControlRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), data);
    	    ((WriteControlRegisterDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((WriteControlRegisterDS2408Cnf *) pSendMsgBody)->success);
		}
    	break;
        case WRITE_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408:
		{
    	    success = writeCSChannelSelectionMaskRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), data);
    	    ((WriteCSChannelSelectionMaskRegisterDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((WriteCSChannelSelectionMaskRegisterDS2408Cnf *) pSendMsgBody)->success);
		}
    	break;
        case WRITE_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408:
		{
    	    success = writeCSChannelPolaritySelectionRegisterDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), data);
    	    ((WriteCSChannelPolaritySelectionRegisterDS2408Cnf *) pSendMsgBody)->success = success;
    	    sendMsgBodyLength += sizeof (((WriteCSChannelPolaritySelectionRegisterDS2408Cnf *) pSendMsgBody)->success);
		}
    	break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that calls resetActivityLatchesDS2408().
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
static UInt16 actionResetActivityLatchesDS2408 (OneWireMsgHeader *pMsgHeader, ResetActivityLatchesDS2408Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = resetActivityLatchesDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]));
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls channelAccessReadDS2408().
 *
 * pMsgHeader     pointer to the message header.
 * numBytesToRead what it says it is.
 * pSendMsgBody   pointer to the relevant message
 *                type to fill in with a response,
 *                which will be overlaid over the
 *                body of the response message.
 *
 * @return        the length of the message body
 *                to send back.
 */
static UInt16 actionChannelAccessReadDS2408 (OneWireMsgHeader *pMsgHeader, UInt8 numBytesToRead, ChannelAccessReadDS2408Cnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;
    UInt8 data [DS2408_MAX_BYTES_IN_CHANNEL_ACCESS];
    UInt8 numBytesRead;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (numBytesToRead <= DS2408_MAX_BYTES_IN_CHANNEL_ACCESS, numBytesToRead);

    memset (&data, 0, sizeof (data));
    numBytesRead = channelAccessReadDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &data[0], numBytesToRead);
	pSendMsgBody->success = true;
	sendMsgBodyLength += sizeof (pSendMsgBody->success);
	if (numBytesRead > DS2408_MAX_BYTES_IN_CHANNEL_ACCESS)
	{
		numBytesRead = DS2408_MAX_BYTES_IN_CHANNEL_ACCESS;
	}
	pSendMsgBody->channelAccessReadDS2408.dataLength = numBytesRead;
	sendMsgBodyLength += sizeof (pSendMsgBody->channelAccessReadDS2408);
	memcpy (&(pSendMsgBody->channelAccessReadDS2408.data[0]), &data, numBytesRead);
	sendMsgBodyLength += sizeof (pSendMsgBody->channelAccessReadDS2408);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls channelAccessWriteDS2408().
 *
 * pMsgHeader    pointer to the message header.
 * pData         pointer to an array of bytes of size
 *               DS2408_MAX_BYTES_IN_CHANNEL_ACCESS to write.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 *
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionChannelAccessWriteDS2408 (OneWireMsgHeader *pMsgHeader, UInt8 *pData, ChannelAccessWriteDS2408Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pData != PNULL, (unsigned long) pData);

	success = channelAccessWriteDS2408 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), pData);
	pSendMsgBody->success = success;
	sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readNVPageDS2438().
 *
 * pMsgHeader     pointer to the message header.
 * page           the page to read.
 * pSendMsgBody   pointer to the relevant message
 *                type to fill in with a response,
 *                which will be overlaid over the
 *                body of the response message.
 *
 * @return        the length of the message body
 *                to send back.
 */
static UInt16 actionReadNVPageDS2438 (OneWireMsgHeader *pMsgHeader, UInt8 page, ReadNVPageDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 mem [DS2438_NUM_BYTES_IN_PAGE];

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (page < DS2438_NUM_PAGES, page);

    memset (&mem, 0, sizeof (mem));
    success = readNVPageDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), page, &mem[0]);
	pSendMsgBody->success = success;
	sendMsgBodyLength += sizeof (pSendMsgBody->success);
	memcpy (&pSendMsgBody->mem[0], &mem, DS2438_NUM_BYTES_IN_PAGE);
	sendMsgBodyLength += DS2438_NUM_BYTES_IN_PAGE;

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls writeNVPageDS2438().
 *
 * pMsgHeader    pointer to the message header.
 * page          the page to write to.
 * pMem          pointer to an array of bytes of up to
 *               DS2438_NUM_BYTES_IN_PAGE to write.
 * size          the number of bytes to write.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 *
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionWriteNVPageDS2438 (OneWireMsgHeader *pMsgHeader, UInt8 page, UInt8 *pMem, UInt8 size, WriteNVPageDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (page < DS2438_NUM_PAGES, page);
    ASSERT_PARAM (pMem != PNULL, (unsigned long) pMem);
    ASSERT_PARAM (size <= DS2438_NUM_BYTES_IN_PAGE, size);

	success = writeNVPageDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), page, pMem, size);
	pSendMsgBody->success = success;
	sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readVddDS2438() or readVadDS2438().
 *
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pMsgHeader    pointer to the message header.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 *
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadVxdDS2438 (OneWireMsgType msgType, OneWireMsgHeader *pMsgHeader, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt16 voltage = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
    	case READ_VDD_DS2438:
		{
			success = readVddDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &voltage);
			((ReadVddDS2438Cnf *) pSendMsgBody)->success = success;
			sendMsgBodyLength += sizeof (((ReadVddDS2438Cnf *) pSendMsgBody)->success);
			((ReadVddDS2438Cnf *) pSendMsgBody)->voltage = voltage;
			sendMsgBodyLength += sizeof (((ReadVddDS2438Cnf *) pSendMsgBody)->voltage);
		}
    	break;
        case READ_VAD_DS2438:
		{
			success = readVadDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &voltage);
			((ReadVadDS2438Cnf *) pSendMsgBody)->success = success;
			sendMsgBodyLength += sizeof (((ReadVadDS2438Cnf *) pSendMsgBody)->success);
			((ReadVadDS2438Cnf *) pSendMsgBody)->voltage = voltage;
			sendMsgBodyLength += sizeof (((ReadVadDS2438Cnf *) pSendMsgBody)->voltage);
		}
    	break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readTemperatureDS2438()
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
static UInt16 actionReadTemperatureDS2438 (OneWireMsgHeader *pMsgHeader, ReadTemperatureDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    double temperature = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readTemperatureDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &temperature);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->temperature = temperature;
    sendMsgBodyLength += sizeof (pSendMsgBody->temperature);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readCurrentDS2438()
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
static UInt16 actionReadCurrentDS2438 (OneWireMsgHeader *pMsgHeader, ReadCurrentDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    SInt16 current = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readCurrentDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &current);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->current = current;
    sendMsgBodyLength += sizeof (pSendMsgBody->current);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readBatteryDS2438()
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
static UInt16 actionReadBatteryDS2438 (OneWireMsgHeader *pMsgHeader, ReadBatteryDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt16 voltage = 0;
    SInt16 current = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readBatteryDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &voltage, &current);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->batteryDS2438.voltage = voltage;
    pSendMsgBody->batteryDS2438.current = current;
    sendMsgBodyLength += sizeof (pSendMsgBody->batteryDS2438);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readNVConfigThresholdDS2438()
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
static UInt16 actionReadNVConfigThresholdDS2438 (OneWireMsgHeader *pMsgHeader, ReadNVConfigThresholdDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 config = 0;
    UInt8 threshold = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readNVConfigThresholdDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &config, &threshold);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->readNVConfigThresholdDS2438.config = config;
    pSendMsgBody->readNVConfigThresholdDS2438.threshold = threshold;
    sendMsgBodyLength += sizeof (pSendMsgBody->readNVConfigThresholdDS2438);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls writeNVConfigThresholdDS2438().
 *
 * pMsgHeader       pointer to the message header.
 * config           the config value to write.
 * thresholdPresent whether threshold is present or not.
 * threshold        what it says.
 * pSendMsgBody     pointer to the relevant message
 *                  type to fill in with a response,
 *                  which will be overlaid over the
 *                  body of the response message.
 *
 * @return          the length of the message body
 *                  to send back.
 */
static UInt16 actionWriteNVConfigThresholdDS2438 (OneWireMsgHeader *pMsgHeader, UInt8 config, Bool thresholdPresent, UInt8 threshold, WriteNVConfigThresholdDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 *pThreshold = PNULL;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    if (thresholdPresent)
    {
        pThreshold = &threshold;
    }
    success = writeNVConfigThresholdDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &config, pThreshold);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readTimeCapacityCalDS2438()
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
static UInt16 actionReadTimeCapacityCalDS2438 (OneWireMsgHeader *pMsgHeader, ReadTimeCapacityCalDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt32 elapsedTime = 0;
    UInt16 remainingCapacity = 0;
    SInt16 offsetCal = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readTimeCapacityCalDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &elapsedTime, &remainingCapacity, &offsetCal);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->readTimeCapacityCalDS2438.elapsedTime = elapsedTime;
    pSendMsgBody->readTimeCapacityCalDS2438.remainingCapacity = remainingCapacity;
    pSendMsgBody->readTimeCapacityCalDS2438.offsetCal = offsetCal;
    sendMsgBodyLength += sizeof (pSendMsgBody->readTimeCapacityCalDS2438);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls writeTimeCapacityDS2438().
 *
 * pMsgHeader               pointer to the message header.
 * elapsedTime              the elapsed time value to write.
 * remainingCapacityPresent whether remainingCapacity is
 *                          present or not.
 * remainingCapacity        what it says.
 * pSendMsgBody             pointer to the relevant message
 *                          type to fill in with a response,
 *                          which will be overlaid over the
 *                          body of the response message.
 *
 * @return          the length of the message body
 *                  to send back.
 */
static UInt16 actionWriteTimeCapacityDS2438 (OneWireMsgHeader *pMsgHeader, UInt32 elapsedTime, Bool remainingCapacityPresent, UInt16 remainingCapacity, WriteTimeCapacityDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt16 *pRemainingCapacity = PNULL;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    if (remainingCapacityPresent)
    {
        pRemainingCapacity = &remainingCapacity;
    }
    success = writeTimeCapacityDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &elapsedTime, pRemainingCapacity);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readTimePiOffChargingStoppedDS2438()
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
static UInt16 actionReadTimePiOffChargingStoppedDS2438 (OneWireMsgHeader *pMsgHeader, ReadTimePiOffChargingStoppedDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt32 piOff = 0;
    UInt32 chargingStopped = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readTimePiOffChargingStoppedDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &piOff, &chargingStopped);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->readTimePiOffChargingStoppedDS2438.piOff = piOff;
    pSendMsgBody->readTimePiOffChargingStoppedDS2438.chargingStopped = chargingStopped;
    sendMsgBodyLength += sizeof (pSendMsgBody->readTimePiOffChargingStoppedDS2438);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readNVChargeDischargeDS2438()
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
static UInt16 actionReadNVChargeDischargeDS2438 (OneWireMsgHeader *pMsgHeader, ReadNVChargeDischargeDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt32 charge = 0;
    UInt32 discharge = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readNVChargeDischargeDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &charge, &discharge);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->readNVChargeDischargeDS2438.charge = charge;
    pSendMsgBody->readNVChargeDischargeDS2438.discharge = discharge;
    sendMsgBodyLength += sizeof (pSendMsgBody->readNVChargeDischargeDS2438);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls writeNVChargeDischargeDS2438().
 *
 * pMsgHeader       pointer to the message header.
 * charge           the charge value to write.
 * dischargePresent whether discharge is present or not.
 * discharge        what it says.
 * pSendMsgBody     pointer to the relevant message
 *                  type to fill in with a response,
 *                  which will be overlaid over the
 *                  body of the response message.
 *
 * @return          the length of the message body
 *                  to send back.
 */
static UInt16 actionWriteNVChargeDischargeDS2438 (OneWireMsgHeader *pMsgHeader, UInt32 charge, Bool dischargePresent, UInt32 discharge, WriteNVChargeDischargeDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt32 *pDischarge = PNULL;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    if (dischargePresent)
    {
        pDischarge = &discharge;
    }
    success = writeNVChargeDischargeDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &charge, pDischarge);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls readNVUserDataDS2438().
 *
 * pMsgHeader     pointer to the message header.
 * block          the user data block to read.
 * pSendMsgBody   pointer to the relevant message
 *                type to fill in with a response,
 *                which will be overlaid over the
 *                body of the response message.
 *
 * @return        the length of the message body
 *                to send back.
 */
static UInt16 actionReadNVUserDataDS2438 (OneWireMsgHeader *pMsgHeader, UInt8 block, ReadNVUserDataDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 mem [DS2438_NUM_BYTES_IN_PAGE];

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (block < DS2438_NUM_USER_DATA_PAGES, block);

    memset (&mem, 0, sizeof (mem));
    success = readNVUserDataDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), block, &mem[0]);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    memcpy (&pSendMsgBody->mem[0], &mem, DS2438_NUM_BYTES_IN_PAGE);
    sendMsgBodyLength += DS2438_NUM_BYTES_IN_PAGE;

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls writeNVUserDataDS2438().
 *
 * pMsgHeader    pointer to the message header.
 * block         the page to write to.
 * pMem          pointer to an array of bytes of up to
 *               DS2438_NUM_BYTES_IN_PAGE to write.
 * size          the number of bytes to write.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 *
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionWriteNVUserDataDS2438 (OneWireMsgHeader *pMsgHeader, UInt8 block, UInt8 *pMem, UInt8 size, WriteNVUserDataDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (block < DS2438_NUM_USER_DATA_PAGES, block);
    ASSERT_PARAM (pMem != PNULL, (unsigned long) pMem);
    ASSERT_PARAM (size <= DS2438_NUM_BYTES_IN_PAGE, size);

    success = writeNVUserDataDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), block, pMem, size);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);

    return sendMsgBodyLength;
}

/*
 * Handle a message that calls performCalDS2438()
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
static UInt16 actionPerformCalDS2438 (OneWireMsgHeader *pMsgHeader, PerformCalDS2438Cnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    SInt16 offsetCal = 0;
    
    ASSERT_PARAM (pMsgHeader != PNULL, (unsigned long) pMsgHeader);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = performCalDS2438 (pMsgHeader->portNumber, &(pMsgHeader->serialNumber[0]), &offsetCal);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->offsetCal = offsetCal;
    sendMsgBodyLength += sizeof (pSendMsgBody->offsetCal);

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
    OneWireMsgHeader oneWireMsgHeader;
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);
    
    pSendMsg->msgLength = 0;
    /* Get the message header, which is at the start of the message body */
    memcpy (&oneWireMsgHeader, pReceivedMsgBody, sizeof (oneWireMsgHeader));

    /* We always respond with the same message type */
    pSendMsg->msgType = (MsgType) receivedMsgType;
    /* Fill in the length so far, will make it right for each message later */
    pSendMsg->msgLength += sizeof (pSendMsg->msgType);
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
		/*
		 * Messages to do with the One Wire bus itself
		 */
        case ONE_WIRE_START_BUS:
        {
            Char * pSerialPortString = &((OneWireStartBusReq *) pReceivedMsgBody)->serialPortString[0];
            pSendMsg->msgLength += actionOneWireStartBus (pSerialPortString, (OneWireStartBusCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case ONE_WIRE_STOP_BUS:
        {
            pSendMsg->msgLength += actionOneWireStopBus (&oneWireMsgHeader, (OneWireStopBusCnf *) &(pSendMsg->msgBody[0]));
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
            pSendMsg->msgLength += actionOneWireFindAllDevices (&oneWireMsgHeader, (OneWireFindAllDevicesCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case ONE_WIRE_ACCESS_DEVICE:
        {
            pSendMsg->msgLength += actionOneWireAccessDevice (&oneWireMsgHeader, (OneWireAccessDeviceCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
		/*
		 * Messages to do with the DS2408 PIO chip
		 */
        case DISABLE_TEST_MODE_DS2408:
        {
            pSendMsg->msgLength += actionDisableTestModeDS2408 (&oneWireMsgHeader, (DisableTestModeDS2408Cnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case READ_CONTROL_REGISTER_DS2408:
        case READ_PIO_LOGIC_STATE_DS2408:
        case READ_PIO_OUTPUT_LATCH_STATE_REGISTER_DS2408:
        case READ_PIO_ACTIVITY_LATCH_STATE_REGISTER_DS2408:
        case READ_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408:
        case READ_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408:
        {
            pSendMsg->msgLength += actionReadByteRegister (receivedMsgType, &oneWireMsgHeader, &pSendMsg->msgBody[0]);
        }
        break;
        case WRITE_CONTROL_REGISTER_DS2408:
        {
            UInt8 data = ((WriteControlRegisterDS2408Req *) pReceivedMsgBody)->data;
            pSendMsg->msgLength += actionWriteByteRegister (receivedMsgType, &oneWireMsgHeader, data, &pSendMsg->msgBody[0]);
        }
        break;
        case WRITE_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408:
        {
            UInt8 data = ((WriteCSChannelSelectionMaskRegisterDS2408Req *) pReceivedMsgBody)->data;
            pSendMsg->msgLength += actionWriteByteRegister (receivedMsgType, &oneWireMsgHeader, data, &pSendMsg->msgBody[0]);
        }
        break;
        case WRITE_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408:
        {
            UInt8 data = ((WriteCSChannelPolaritySelectionRegisterDS2408Req *) pReceivedMsgBody)->data;
            pSendMsg->msgLength += actionWriteByteRegister (receivedMsgType, &oneWireMsgHeader, data,  &pSendMsg->msgBody[0]);
        }
        break;
        case RESET_ACTIVITY_LATCHES_DS2408:
        {
            pSendMsg->msgLength += actionResetActivityLatchesDS2408 (&oneWireMsgHeader, (ResetActivityLatchesDS2408Cnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case CHANNEL_ACCESS_READ_DS2408:
        {
            UInt8 numBytesToRead = ((ChannelAccessReadDS2408Req *) pReceivedMsgBody)->numBytesToRead;
            pSendMsg->msgLength += actionChannelAccessReadDS2408 (&oneWireMsgHeader, numBytesToRead, (ChannelAccessReadDS2408Cnf *) &pSendMsg->msgBody[0]);
        }
        break;
        case CHANNEL_ACCESS_WRITE_DS2408:
        {
            UInt8 *pData = &((ChannelAccessWriteDS2408Req *) pReceivedMsgBody)->data[0];
            pSendMsg->msgLength += actionChannelAccessWriteDS2408 (&oneWireMsgHeader, pData, (ChannelAccessWriteDS2408Cnf *) &pSendMsg->msgBody[0]);
        }
        break;
		/*
		 * Messages to do with the DS2438 battery monitoring chip
		 */
        case READ_NV_PAGE_DS2438:
        {
            UInt8 page = ((ReadNVPageDS2438Req *) pReceivedMsgBody)->page;
            pSendMsg->msgLength += actionReadNVPageDS2438 (&oneWireMsgHeader, page, (ReadNVPageDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
        break;
        case WRITE_NV_PAGE_DS2438:
        {
            UInt8 page = ((WriteNVPageDS2438Req *) pReceivedMsgBody)->writeNVPageDS2438.page;
            UInt8 *pMem = &(((WriteNVPageDS2438Req *) pReceivedMsgBody)->writeNVPageDS2438.mem[0]);
            UInt8 memLength = ((WriteNVPageDS2438Req *) pReceivedMsgBody)->writeNVPageDS2438.memLength;
            pSendMsg->msgLength += actionWriteNVPageDS2438 (&oneWireMsgHeader, page, pMem, memLength, (WriteNVPageDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
        break;
        case READ_VDD_DS2438:
        case READ_VAD_DS2438:
        {
            pSendMsg->msgLength += actionReadVxdDS2438 (receivedMsgType, &oneWireMsgHeader, &pSendMsg->msgBody[0]);
        }
        break;
        case READ_TEMPERATURE_DS2438:
        {
            pSendMsg->msgLength += actionReadTemperatureDS2438 (&oneWireMsgHeader, (ReadTemperatureDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case READ_CURRENT_DS2438:
        {
            pSendMsg->msgLength += actionReadCurrentDS2438 (&oneWireMsgHeader, (ReadCurrentDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case READ_BATTERY_DS2438:
        {
            pSendMsg->msgLength += actionReadBatteryDS2438 (&oneWireMsgHeader, (ReadBatteryDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case READ_NV_CONFIG_THRESHOLD_DS2438:
        {
            pSendMsg->msgLength += actionReadNVConfigThresholdDS2438 (&oneWireMsgHeader, (ReadNVConfigThresholdDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case WRITE_NV_CONFIG_THRESHOLD_DS2438:
        {
            UInt8 config = ((WriteNVConfigThresholdDS2438Req *) pReceivedMsgBody)->writeNVConfigThresholdDS2438.config;
            Bool thresholdPresent = ((WriteNVConfigThresholdDS2438Req *) pReceivedMsgBody)->writeNVConfigThresholdDS2438.thresholdPresent;
            UInt8 threshold = ((WriteNVConfigThresholdDS2438Req *) pReceivedMsgBody)->writeNVConfigThresholdDS2438.threshold;
            pSendMsg->msgLength += actionWriteNVConfigThresholdDS2438 (&oneWireMsgHeader, config, thresholdPresent, threshold, (WriteNVConfigThresholdDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case READ_TIME_CAPACITY_CAL_DS2438:
        {
            pSendMsg->msgLength += actionReadTimeCapacityCalDS2438 (&oneWireMsgHeader, (ReadTimeCapacityCalDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case WRITE_TIME_CAPACITY_DS2438:
        {
            UInt32 elapsedTime = ((WriteTimeCapacityDS2438Req *) pReceivedMsgBody)->writeTimeCapacityDS2438.elapsedTime;
            Bool remainingCapacityPresent = ((WriteTimeCapacityDS2438Req *) pReceivedMsgBody)->writeTimeCapacityDS2438.remainingCapacityPresent;
            UInt16 remainingCapacity = ((WriteTimeCapacityDS2438Req *) pReceivedMsgBody)->writeTimeCapacityDS2438.remainingCapacity;
            pSendMsg->msgLength += actionWriteTimeCapacityDS2438 (&oneWireMsgHeader, elapsedTime, remainingCapacityPresent, remainingCapacity, (WriteTimeCapacityDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case READ_TIME_PI_OFF_CHARGING_STOPPED_DS2438:
        {
            pSendMsg->msgLength += actionReadTimePiOffChargingStoppedDS2438 (&oneWireMsgHeader, (ReadTimePiOffChargingStoppedDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case READ_NV_CHARGE_DISCHARGE_DS2438:
        {
            pSendMsg->msgLength += actionReadNVChargeDischargeDS2438 (&oneWireMsgHeader, (ReadNVChargeDischargeDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case WRITE_NV_CHARGE_DISCHARGE_DS2438:
        {
            UInt32 charge = ((WriteNVChargeDischargeDS2438Req *) pReceivedMsgBody)->writeNVChargeDischargeDS2438.charge;
            Bool dischargePresent = ((WriteNVChargeDischargeDS2438Req *) pReceivedMsgBody)->writeNVChargeDischargeDS2438.dischargePresent;
            UInt16 discharge = ((WriteNVChargeDischargeDS2438Req *) pReceivedMsgBody)->writeNVChargeDischargeDS2438.discharge;
            pSendMsg->msgLength += actionWriteNVChargeDischargeDS2438 (&oneWireMsgHeader, charge, dischargePresent, discharge, (WriteNVChargeDischargeDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case READ_NV_USER_DATA_DS2438:
        {
            UInt8 block = ((ReadNVUserDataDS2438Req *) pReceivedMsgBody)->block;
            pSendMsg->msgLength += actionReadNVUserDataDS2438 (&oneWireMsgHeader, block, (ReadNVUserDataDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case WRITE_NV_USER_DATA_DS2438:
        {
            UInt8 block = ((WriteNVUserDataDS2438Req *) pReceivedMsgBody)->writeNVUserDataDS2438.block;
            UInt8 *pMem = &(((WriteNVUserDataDS2438Req *) pReceivedMsgBody)->writeNVUserDataDS2438.mem[0]);
            UInt8 memLength = ((WriteNVUserDataDS2438Req *) pReceivedMsgBody)->writeNVUserDataDS2438.memLength;
            pSendMsg->msgLength += actionWriteNVUserDataDS2438 (&oneWireMsgHeader, block, pMem, memLength, (WriteNVUserDataDS2438Cnf *) &pSendMsg->msgBody[0]);
        }
		break;
        case PERFORM_CAL_DS2438:
        {
            pSendMsg->msgLength += actionPerformCalDS2438 (&oneWireMsgHeader, (PerformCalDS2438Cnf *) &pSendMsg->msgBody[0]);
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
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_ONE_WIRE_MSGS, pReceivedMsg->msgType);
    
    /* Do the thang */
    returnCode = doAction ((OneWireMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
        
    return returnCode;
}
