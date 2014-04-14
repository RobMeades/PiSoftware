/*
 * Auto generated stuff: the message types, structures and unions
 * This header file is a bit special in that it is allowed to
 * include others.  None are gated, so be careful not to get
 * circular.
 */

#include <state_machine_msg_macros.h>
#include <state_machine_msgs.h>

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * MESSAGE TYPES
 */
typedef enum StateMachineMsgTypeTag
{
#undef STATE_MACHINE_MSG_DEF
#define STATE_MACHINE_MSG_DEF STATE_MACHINE_MSG_DEF_TYPE
#include <state_machine_msgs.h>
MAX_NUM_STATE_MACHINE_MSGS
} StateMachineMsgType;

/*
 * MESSAGES BODIES: REQ MESSAGES
 */
#undef STATE_MACHINE_MSG_DEF
#define STATE_MACHINE_MSG_DEF MAKE_STATE_MACHINE_MSG_STRUCT_REQ
#include <state_machine_msgs.h>

/*
 * MESSAGES BODIES: CNF MESSAGES
 */
#undef STATE_MACHINE_MSG_DEF
#define STATE_MACHINE_MSG_DEF MAKE_STATE_MACHINE_MSG_STRUCT_CNF
#include <state_machine_msgs.h>

/*
 * MESSAGE UNION: REQ MESSAGES
 */
typedef union StateMachineMsgUnionReqTag
{
#undef STATE_MACHINE_MSG_DEF
#define STATE_MACHINE_MSG_DEF MAKE_STATE_MACHINE_UNION_MEMBER_REQ
#include <state_machine_msgs.h>
} StateMachineMsgUnionReq;

/*
 * MESSAGE UNION: CNF MESSAGES
 */
typedef union StateMachineMsgUnionCnfTag
{
#undef STATE_MACHINE_MSG_DEF
#define STATE_MACHINE_MSG_DEF MAKE_STATE_MACHINE_UNION_MEMBER_CNF
#include <state_machine_msgs.h>
} StateMachineMsgUnionCnf;
#pragma pack(pop) /* End of packing */ 