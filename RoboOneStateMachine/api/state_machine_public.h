/*
 * Access stuff for the state machine server.
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * FUNCTION PROTOTYPES
 */
Bool stateMachineServerSendReceive (StateMachineMsgType sendMsgType, void *pSendMsgBody, UInt16 sendMsgBodyLength, StateMachineMsgType *pReceivedMsgType, void *pReceivedMsgBody);