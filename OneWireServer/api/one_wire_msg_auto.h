/*
 * Auto generated stuff: the message types, structures and unions
 * This header file is a bit special in that it is allowed to
 * include others.  None are gated, so be careful not to get
 * circular.
 */

#include <one_wire_msg_macros.h>
#include <one_wire_msgs.h>

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * MESSAGE TYPES
 */
typedef enum OneWireMsgTypeTag
{
#undef MSG_DEF
#define MSG_DEF MSG_DEF_TYPE
#include <one_wire_msgs.h>
MAX_NUM_ONE_WIRE_MSG
} OneWireMsgType;

/*
 * MESSAGES BODIES: REQ MESSAGES
 */
#undef MSG_DEF
#define MSG_DEF MAKE_ONE_WIRE_MSG_STRUCT_REQ
#include <one_wire_msgs.h>

/*
 * MESSAGES BODIES: CNF MESSAGES
 */
#undef MSG_DEF
#define MSG_DEF MAKE_ONE_WIRE_MSG_STRUCT_CNF
#include <one_wire_msgs.h>

/*
 * MESSAGE UNION: REQ MESSAGES
 */
typedef union OneWireMsgUnionReqTag
{
#undef MSG_DEF
#define MSG_DEF MAKE_UNION_MEMBER_REQ
#include <one_wire_msgs.h>
} OneWireMsgUnionReq;

/*
 * MESSAGE UNION: CNF MESSAGES
 */
typedef union OneWireMsgUnionCnfTag
{
#undef MSG_DEF
#define MSG_DEF MAKE_UNION_MEMBER_CNF
#include <one_wire_msgs.h>
} OneWireMsgUnionCnf;
#pragma pack(pop) /* End of packing */ 