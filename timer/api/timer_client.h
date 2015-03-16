/*
 * Access functions for the timer server
 */

/*
 * MANIFEST CONSTANTS
 */

/* Suggested 0.1s delay before the server is ready */
#define TIMER_SERVER_START_DELAY_PI_US 100000L

/*
 * TYPES
 */

/*
 * FUNCTION PROTOTYPES
 */
Bool timerServerSend (TimerMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength);