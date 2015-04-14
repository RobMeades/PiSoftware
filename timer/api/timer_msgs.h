/*
 * Messages that go from/to the timer server.
 * 
 * These messages never have a confirm, only Req and Ind
 * forms, therefore the structures here are slightly different
 * to those in the other servers.
 */

/* This file is included in the timer_msg_auto header and
 * defines the message structures for the messages sent to
 * and from the server.  The items in the list are:
 * 
 * - the message type enum,
 * - the message typedef struct,
 * - the message variable name for use in the message union,
 * - the message members that are needed in the message structure
 *   beyond any mandatory header (which is added automatagically),
 */

/*
 * Definitions
 */
TIMER_MSG_DEF (TIMER_SERVER_START_REQ, TimerServerStartReq, timerServerStartReq, TIMER_EMPTY)
TIMER_MSG_DEF (TIMER_SERVER_STOP_REQ, TimerServerStopReq, timerServerStopReq, TIMER_EMPTY)
/* Note: the id field is only required to be filled-in if the timer might ever be stopped */
TIMER_MSG_DEF (TIMER_START_REQ, TimerStartReq, timerStartReq, UInt32 expiryDeciSeconds; TimerId id; SInt32 sourcePort; ShortMsg expiryMsg)
TIMER_MSG_DEF (TIMER_STOP_REQ, TimerStopReq, timerStopReq, TimerId id; SInt32 sourcePort)