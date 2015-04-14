/*
 *  Public stuff for the timer server
 */

/*
 * MANIFEST CONSTANTS
 */

#define TIMER_SERVER_EXE "./timer_server"
#define TIMER_SERVER_PORT_STRING "5235"

/* The maximum message body length for a short message, designed to be small enough for it
 * to be nested inside another */
#define MAX_SHORT_MSG_BODY_LENGTH  MAX_MSG_BODY_LENGTH - 40


#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE TYPES
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

typedef UInt8 TimerId;

/* The structure of a timer expiry message
 * NOTE: this MUST follow the form of the Msg struct but with a slightly shorter
 * body (as it is a message designed to be carried within a message). */
typedef struct ShortMsgTag
{
    MsgLength msgLength;                      /* The number of bytes to follow (so msgType and msgBody) */
    MsgType msgType;                          /* The type of the message, override with your intended type */
    UInt8 msgBody[MAX_SHORT_MSG_BODY_LENGTH]; /* The data for this message type */
} ShortMsg;

#pragma pack(pop) /* End of packing */

/*
 * TYPES FOR REQ MESSAGES
 */

/*
 * TYPES FOR IND MESSAGES
 */

#pragma pack(pop) /* End of packing */ 