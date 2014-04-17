/*
 * Stuff to do with state machine events.
 */ 

/*
 * MANIFEST CONSTANTS
 */

/*
 *  FUNCTION PROTOTYPES
 */
Bool stateMachineServerSendReceive (StateMachineMsgType sendMsgType, void *pSendMsgBody, UInt16 sendMsgBodyLength, StateMachineMsgType *pReceivedMsgType, void *pReceivedMsgBody);
