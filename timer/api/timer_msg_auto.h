/*
 * Auto generated stuff: the message types, structures and unions
 * This header file is a bit special in that it is allowed to
 * include others.  None are gated, so be careful not to get
 * circular.
 */

#include <timer_msg_macros.h>
#include <timer_msgs.h>

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * MESSAGE TYPES
 */
typedef enum TimerMsgTypeTag
{
#undef TIMER_MSG_DEF
#define TIMER_MSG_DEF TIMER_MSG_DEF_TYPE
#include <timer_msgs.h>
MAX_NUM_TIMER_MSGS
} TimerMsgType;

/*
 * MESSAGES BODIES
 */
#undef TIMER_MSG_DEF
#define TIMER_MSG_DEF MAKE_TIMER_MSG_STRUCT
#include <timer_msgs.h>

/*
 * MESSAGE UNION
 */
typedef union TimerMsgUnionTag
{
#undef TIMER_MSG_DEF
#define TIMER_MSG_DEF MAKE_TIMER_UNION_MEMBER
#include <timer_msgs.h>
} TimerMsgUnion;

#pragma pack(pop) /* End of packing */ 