/*
 * Access stuff for the task handler server.
 */

/*
 * MANIFEST CONSTANTS
 */

/* Suggested 500ms delay before task handler server is ready */
#define TASK_HANDLER_SERVER_START_DELAY_PI_US 500000L

/*
 * TYPES
 */

/*
 * FUNCTION PROTOTYPES
 */
Bool taskHandlerServerSendReceive (TaskHandlerMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength);