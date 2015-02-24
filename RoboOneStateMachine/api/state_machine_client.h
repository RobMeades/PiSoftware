/*
 * Access stuff for the state machine server.
 */

/*
 * MANIFEST CONSTANTS
 */

/* Suggested 1s delay before state machine server is ready */
#define STATE_MACHINE_SERVER_START_DELAY_PI_US 1000000L

/*
 * TYPES
 */

/*
 * FUNCTION PROTOTYPES
 */
Bool stateMachineServerSendReceive (StateMachineMsgType sendMsgType, void *pSendMsgBody, UInt16 sendMsgBodyLength, StateMachineMsgType *pReceivedMsgType, void *pReceivedMsgBody);