/*
 * Public stuff for the generic messaging client.
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/* The return codes from the messaging client */
typedef enum ClientReturnCodeTag
{
    CLIENT_SUCCESS = 0,
    CLIENT_ERR_GENERAL_FAILURE,
    CLIENT_ERR_COULDNT_SEND_WHOLE_MESSAGE_TO_SERVER,
    CLIENT_ERR_FAILED_TO_CONNECT_TO_SERVER,
    CLIENT_ERR_FAILED_TO_CREATE_SOCKET,
    CLIENT_ERR_SEND_MESSAGE_IS_PNULL,
    CLIENT_ERR_MESSAGE_FROM_SERVER_INCOMPLETE_OR_TOO_LONG
} ClientReturnCode;

/*
 * FUNCTION PROTOTYPES
 */

ClientReturnCode runMessagingClient (UInt16 serverPort, Msg *pSendMsg, Msg *pReceivedMsg);