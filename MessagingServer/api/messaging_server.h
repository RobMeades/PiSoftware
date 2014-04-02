/*
 * Public stuff for the generic messaging server
 * See the bottom of this file for the things that the
 * user of this library must provide.
 */

/*
 * MANIFEST CONSTANTS
 */

/* Things to size and find bits inside all messages */
#define OFFSET_TO_MSG_LENGTH      0
#define SIZE_OF_MSG_LENGTH        1
#define OFFSET_TO_MSG_TYPE        OFFSET_TO_MSG_LENGTH + SIZE_OF_MSG_LENGTH
#define SIZE_OF_MSG_TYPE          1
#define OFFSET_TO_MSG_BODY        OFFSET_TO_MSG_TYPE + SIZE_OF_MSG_TYPE
#define MAX_MSG_LENGTH            256 - SIZE_OF_MSG_LENGTH
#define MAX_MSG_BODY_LENGTH       MAX_MSG_LENGTH - OFFSET_TO_MSG_BODY
#define MIN_MSG_LENGTH            SIZE_OF_MSG_TYPE

/* Suggested delay of 100 ms to allow the server to start on a Pi before accessing it */
#define SERVER_START_DELAY_PI_US  100000L

/*
 * TYPES
 */

/* These just to make things neater */
typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr SockAddr;

/* The return codes from the messaging server */
typedef enum ServerReturnCodeTag
{
    SERVER_SUCCESS_KEEP_RUNNING = 0,
    SERVER_EXIT_NORMALLY,
    SERVER_ERR_GENERAL_FAILURE,
    SERVER_ERR_FAILED_TO_ACCEPT_CLIENT_CONNECTION,
    SERVER_ERR_FAILED_TO_LISTEN_ON_SOCKET,
    SERVER_ERR_FAILED_TO_BIND_SOCKET,
    SERVER_ERR_FAILED_TO_CREATE_SOCKET,
    SERVER_ERR_FAILED_TO_SET_SOCKET_OPTIONS,
    SERVER_ERR_FAILED_TO_SEND_RESPONSE_TO_CLIENT,
    SERVER_ERR_FAILED_TO_GET_MEMORY_FOR_RESPONSE,
    SERVER_ERR_MESSAGE_FROM_CLIENT_INCOMPLETE_OR_TOO_LONG,
    SERVER_ERR_FAILED_TO_GET_MEMORY_FOR_RECEPTION,
    SERVER_ERR_MESSAGE_TOO_LARGE
} ServerReturnCode;

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/* The possible messages types, should be cast to whatever
 * enum is used in the eventual application */
typedef UInt8 MsgType;

/* The length of the message in bytes following this length indicator
 * (i.e. the length indicator is not included in the length) */
typedef UInt8 MsgLength;

/* The structure of a message */
typedef struct MsgTag
{
    MsgLength msgLength;                 /* The number of bytes to follow (so msgType and msgBody)
                                          * Zero length messages are received and passed to serverHandleMsg()
                                          * by the server but no zero length response messages will be
                                          * sent by the server */
    MsgType msgType;                     /* The type of the message, override with your intended type */
    UInt8 msgBody[MAX_MSG_BODY_LENGTH];  /* The data for this message type */
} Msg;

#pragma pack(pop) /* End of packing */

/*
 * FUNCTION PROTOTYPES
 */

ServerReturnCode runMessagingServer (UInt16 serverPort);

/*
 * EXTERNS: must be provided by the user of this library
 */

/* Handle a received message and, optionally, provide
 * a response message. If the response message is not
 * required, just leave pSendMsg alone.
 * 
 * pReceivedMsg   a pointer to the buffer containing the
 *                incoming message.
 * pSendMsg       a pointer to a message buffer to put
 *                the response into. Not touched if return
 *                code is a failure one.  Zero length
 *                messages will not be sent by the server.
 * 
 * @return        server return code.
 */
extern ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg* pSendMsg);