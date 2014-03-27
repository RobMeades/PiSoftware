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
#include <onewire_server.h>

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
 * @return        server return code.
 */
ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg *pSendMsg)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
    OneWireReqMsgHeader msgHeader;
    
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);

    /* First check the type */
    printProgress ("Server: received message of length %d and type %d.\n", pReceivedMsg->msgLength, pReceivedMsg->msgType);    
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_ONE_WIRE_MSG, pReceivedMsg->msgType);
    
    /* Then get the header */
    memcpy (&msgHeader, (pReceivedMsg + OFFSET_TO_MSG_BODY), sizeof (msgHeader));
    printProgress ("Header has port number 0x%x and serialNumber 0x%x%x%x%x%x%x%x%x.\n", msgHeader.portNumber, msgHeader.serialNumber[0], msgHeader.serialNumber[1], msgHeader.serialNumber[2], msgHeader.serialNumber[3], msgHeader.serialNumber[4], msgHeader.serialNumber[5], msgHeader.serialNumber[6], msgHeader.serialNumber[7]);
    
    /* TODO: now process the message and create the response */
    pSendMsg->msgType = 1;
    pSendMsg->msgLength = 1;
    
    return returnCode;
}