/*
 * Utility functions for accessing RoboOne hardware
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <hardware_types.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * EXTERNS
 */
extern Char *pgHardwareMessageNames[];

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Send a message to the Hardware Server and
 * get the response back.
 * 
 * msgType               the message type to send.
 * pSendMsgBody          pointer to the body of the
 *                       REquest message to send.
 *                       May be PNULL.
 * sendMsgBodyLength     the length of the data that
 *                       pSendMsg points to.
 * pReceivedMsgSpecifics pointer to the part of the
 *                       received CNF message after the
 *                       generic 'success' part.  May be
 *                       PNULL.
 * 
 * @return           true if the message send/receive
 *                   is successful and the response
 *                   message indicates success,
 *                   otherwise false.
 */
Bool hardwareServerSendReceive (HardwareMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength, void *pReceivedMsgSpecifics)
{
    ClientReturnCode returnCode;
    Bool success = false;
    Msg *pSendMsg;
    Msg *pReceivedMsg;
    UInt16 receivedMsgBodyLength = 0;

    ASSERT_PARAM (msgType < MAX_NUM_HARDWARE_MSGS, msgType);
    ASSERT_PARAM (sendMsgBodyLength <= MAX_MSG_BODY_LENGTH, sendMsgBodyLength);

    pSendMsg = malloc (sizeof (*pSendMsg));
    
    if (pSendMsg != PNULL)
    {
        pReceivedMsg = malloc (sizeof (*pReceivedMsg));
        
        if (pReceivedMsg != PNULL)
        {
            /* Put in the bit before the body */
            pSendMsg->msgLength = 0;
            pSendMsg->msgType = msgType;
            pSendMsg->msgLength += sizeof (pSendMsg->msgType);
                        
            /* Put any stuff to send */
            if (pSendMsgBody != PNULL)
            {
                memcpy (&pSendMsg->msgBody[0], pSendMsgBody, sendMsgBodyLength);
            }
            pSendMsg->msgLength += sendMsgBodyLength;
            
            pReceivedMsg->msgLength = 0;
    
            printDebug ("HW Client: sending message %s, length %d, hex dump:\n", pgHardwareMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
            printHexDump (pSendMsg, pSendMsg->msgLength + 1);
            returnCode = runMessagingClient ((SInt32) atoi (HARDWARE_SERVER_PORT_STRING), PNULL, pSendMsg, pReceivedMsg);
                    
            printDebug ("HW Client: message system returnCode: %d\n", returnCode);
            /* This code makes assumptions about packing (i.e. that it's '1' and that the
             * Bool 'success' is at the start of the body) so be careful */
            if (returnCode == CLIENT_SUCCESS && (pReceivedMsg->msgLength > sizeof (pReceivedMsg->msgType)))
            { 
                /* Check the Bool 'success' at the start of the message body */
                receivedMsgBodyLength = pReceivedMsg->msgLength - sizeof (pReceivedMsg->msgType);
                printDebug ("HW Client: receivedMsgBodyLength: %d\n", receivedMsgBodyLength);
                printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);
                if (receivedMsgBodyLength >= sizeof (Bool))
                {
                    printDebug ("HW Client: success field: %d\n", (Bool) pReceivedMsg->msgBody[0]);
                    if ((Bool) pReceivedMsg->msgBody[0])
                    {
                        success = true;
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