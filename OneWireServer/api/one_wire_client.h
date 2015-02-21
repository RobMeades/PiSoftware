/*
 * Access stuff for the One Wire server.
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
Bool oneWireServerSendReceive (OneWireMsgType msgType, SInt32 portNumber, UInt8 *pSerialNumber, void *pSendMsgSpecifics, UInt16 specificsLength, void *pReceivedMsgSpecifics);