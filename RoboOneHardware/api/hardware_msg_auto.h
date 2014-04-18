/*
 * Auto generated stuff: the message types, structures and unions
 * This header file is a bit special in that it is allowed to
 * include others.  None are gated, so be careful not to get
 * circular.
 */

#include <hardware_msg_macros.h>
#include <hardware_msgs.h>

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * MESSAGE TYPES
 */
typedef enum HardwareMsgTypeTag
{
#undef HARDWARE_MSG_DEF
#define HARDWARE_MSG_DEF HARDWARE_MSG_DEF_TYPE
#include <hardware_msgs.h>
MAX_NUM_HARDWARE_MSGS
} HardwareMsgType;

/*
 * MESSAGES BODIES: REQ MESSAGES
 */
#undef HARDWARE_MSG_DEF
#define HARDWARE_MSG_DEF MAKE_HARDWARE_MSG_STRUCT_REQ
#include <hardware_msgs.h>

/*
 * MESSAGES BODIES: CNF MESSAGES
 */
#undef HARDWARE_MSG_DEF
#define HARDWARE_MSG_DEF MAKE_HARDWARE_MSG_STRUCT_CNF
#include <hardware_msgs.h>

/*
 * MESSAGE UNION: REQ MESSAGES
 */
typedef union HardwareMsgUnionReqTag
{
#undef HARDWARE_MSG_DEF
#define HARDWARE_MSG_DEF MAKE_HARDWARE_UNION_MEMBER_REQ
#include <hardware_msgs.h>
} HardwareMsgUnionReq;

/*
 * MESSAGE UNION: CNF MESSAGES
 */
typedef union HardwareMsgUnionCnfTag
{
#undef HARDWARE_MSG_DEF
#define HARDWARE_MSG_DEF MAKE_HARDWARE_UNION_MEMBER_CNF
#include <hardware_msgs.h>
} HardwareMsgUnionCnf;
#pragma pack(pop) /* End of packing */ 