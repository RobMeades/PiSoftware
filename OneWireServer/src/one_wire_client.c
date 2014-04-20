/*
 * Access functions.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <one_wire.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <one_wire_server.h>
#include <one_wire_msg_auto.h>
#include <one_wire_client.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * EXTERNS
 */
extern Char *pgOneWireMessageNames[];

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
 * Send a message to the One Wire Server and
 * get the response back.
 * 
 * msgType               the message type to send.
 * portNumber            the port number for the opened
 *                       serial port where the OneWire
 *                       driver chip can be accessed.
 * pSerialNumber         pointer to the serial number of
 *                       the One Wire device we are
 *                       addressing.  PNULL makes sense
 *                       in some limited cases.
 * pSendMsgSpecifics     pointer to the portion of the
 *                       send REquest message beyond the
 *                       generic msgHeader part.  May be
 *                       PNULL.
 * specificsLength       the length of the bit that
 *                       pSendMsgSpecifics points to.
 * pReceivedMsgSpecifics pointer to the part of the
 *                       received CNF message after the
 *                       generic 'success' part.  May be
 *                       PNULL.
 * 
 * @return               true if the message send/receive
 *                       is successful and the response
 *                       message indicates success,
 *                       otherwise false.
 */
Bool oneWireServerSendReceive (OneWireMsgType msgType, SInt32 portNumber, UInt8 *pSerialNumber, void *pSendMsgSpecifics, UInt16 specificsLength, void *pReceivedMsgSpecifics)
{
    ClientReturnCode returnCode;
    Bool success = false;
    Msg *pSendMsg;
    OneWireMsgHeader sendMsgHeader;
    UInt16 sendMsgBodyLength = 0;
    Msg *pReceivedMsg;
    UInt16 receivedMsgBodyLength = 0;

    ASSERT_PARAM (portNumber >= 0, portNumber);
    ASSERT_PARAM (msgType < MAX_NUM_ONE_WIRE_MSGS, (unsigned long) msgType);
    ASSERT_PARAM (((pSerialNumber != PNULL) || ((msgType == ONE_WIRE_SERVER_EXIT) || (msgType == ONE_WIRE_START_BUS) || (msgType == ONE_WIRE_STOP_BUS) || (msgType == ONE_WIRE_FIND_ALL_DEVICES))), (unsigned long) pSerialNumber);
    ASSERT_PARAM (specificsLength <= MAX_MSG_BODY_LENGTH - sizeof (sendMsgHeader), specificsLength);

    pSendMsg = malloc (sizeof (Msg));
    
    if (pSendMsg != PNULL)
    {
        pReceivedMsg = malloc (sizeof (Msg));
        
        if (pReceivedMsg != PNULL)
        {
            /* Put in the bit before the body */
            pSendMsg->msgLength = 0;
            pSendMsg->msgType = msgType;
            pSendMsg->msgLength += sizeof (pSendMsg->msgType);
                        
            /* Put in the generic header at the start of the body */
            memset (&sendMsgHeader, 0, sizeof (sendMsgHeader));
            sendMsgHeader.portNumber = portNumber;
            if (pSerialNumber != PNULL)
            {
                memcpy (&sendMsgHeader.serialNumber, pSerialNumber, sizeof (sendMsgHeader.serialNumber));
            }
            memcpy (&(pSendMsg->msgBody[0]), &sendMsgHeader, sizeof (sendMsgHeader));
            sendMsgBodyLength += sizeof (sendMsgHeader);
            
            /* Put in the specifics */
            if (pSendMsgSpecifics != PNULL)
            {
                memcpy (&pSendMsg->msgBody[0] + sendMsgBodyLength, pSendMsgSpecifics, specificsLength);
                sendMsgBodyLength += specificsLength;
            }
            pSendMsg->msgLength += sendMsgBodyLength;
            
            pReceivedMsg->msgLength = 0;
    
            printDebug ("\nOW Client: sending message %s, length %d, hex dump:\n", pgOneWireMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
            printHexDump ((UInt8 *) pSendMsg, pSendMsg->msgLength + 1);
            returnCode = runMessagingClient ((SInt32) atoi (ONE_WIRE_SERVER_PORT_STRING), pSendMsg, pReceivedMsg);
                    
            printDebug ("OW Client: message system returnCode: %d\n", returnCode);
            /* This code makes assumptions about packing (i.e. that it's '1' and that the
             * Bool 'success' is at the start of the body) so be careful */
            if (returnCode == CLIENT_SUCCESS && (pReceivedMsg->msgLength > sizeof (pReceivedMsg->msgType)))
            { 
                /* Check the Bool 'success' at the start of the message body */
                receivedMsgBodyLength = pReceivedMsg->msgLength - sizeof (pReceivedMsg->msgType);
                printDebug ("OW Client: receivedMsgBodyLength: %d\n", receivedMsgBodyLength);
                if (receivedMsgBodyLength >= sizeof (Bool))
                {
                    printDebug ("OW Client: success field: %d\n", (Bool) pReceivedMsg->msgBody[0]);
                    if ((Bool) pReceivedMsg->msgBody[0])
                    {
                        success = true;
                        printDebug ("OW Client: received message %s, hex dump:\n", pgOneWireMessageNames[pReceivedMsg->msgType]);
                        printHexDump ((UInt8 *) pReceivedMsg, pReceivedMsg->msgLength + 1);

                        if (pReceivedMsgSpecifics != PNULL)
                        {
                            /* Copy out the bits beyond the success field for passing back */
                            memcpy (pReceivedMsgSpecifics, &pReceivedMsg->msgBody[0] + sizeof (Bool), receivedMsgBodyLength - sizeof (Bool));
                        }
                    }
                }
            }
            free (pReceivedMsg);
        }
        free (pSendMsg);
    }

    return success;
}