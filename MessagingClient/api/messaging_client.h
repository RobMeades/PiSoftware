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
    CLIENT_ERR_GENERAL_FAILURE = -1,
    CLIENT_ERR_COULDNT_SEND_WHOLE_MESSAGE_TO_SERVER = -2,
    CLIENT_ERR_FAILED_TO_CONNECT_TO_SERVER = -3,
    CLIENT_ERR_FAILED_TO_CREATE_SOCKET = -4,
    CLIENT_ERR_SEND_MESSAGE_IS_PNULL = -5,
    CLIENT_ERR_MESSAGE_FROM_SERVER_INCOMPLETE_OR_TOO_LONG = -6,
    CLIENT_ERR_FAILED_ON_RECV = -7
} ClientReturnCode;

/*
 * FUNCTION PROTOTYPES
 */

ClientReturnCode runMessagingClient (UInt16 serverPort, Char *pIpAddress, Msg *pSendMsg, Msg *pReceivedMsg);