/*
 * Access functions for the Hardware Server
 */

/*
 * MANIFEST CONSTANTS
 */

/* Suggested 5s delay before state machine server is ready */
#define HARDWARE_SERVER_START_DELAY_PI_US 5000000L

/*
 * TYPES
 */

/*
 * FUNCTION PROTOTYPES
 */
Bool hardwareServerSendReceive (HardwareMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength, void *pReceivedMsgSpecifics);