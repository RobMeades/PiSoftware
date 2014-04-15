/*
 * Stuff to do with state machine events.
 */ 

/*
 * MANIFEST CONSTANTS
 */

/*
 *  FUNCTION PROTOTYPES
 */
Bool stateMachineServerSendReceive (StateMachineMsgType sendMsgType, void *pSendMsgSpecifics, UInt16 specificsLength, StateMachineMsgType *pReceivedMsgType, void *pReceivedMsgBody);
