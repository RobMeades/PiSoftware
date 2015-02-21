/*
 * Access functions for the Battery Manager Server
 */

/*
 * MANIFEST CONSTANTS
 */

/* Suggested 5s delay before state machine server is ready */
#define BATTERY_MANAGER_SERVER_START_DELAY_PI_US 500000L

/*
 * TYPES
 */

/*
 * FUNCTION PROTOTYPES
 */
Bool batteryManagerServerSendReceive (BatteryManagerMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength, void *pReceivedMsgSpecifics);