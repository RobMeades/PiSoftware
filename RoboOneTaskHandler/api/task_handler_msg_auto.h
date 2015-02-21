/*
 * Auto generated stuff: the message types, structures and unions
 * This header file is a bit special in that it is allowed to
 * include others.  None are gated, so be careful not to get
 * circular.
 */

#include <task_handler_msg_macros.h>
#include <task_handler_msgs.h>

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * MESSAGE TYPES
 */
typedef enum TaskHandlerMsgTypeTag
{
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF TASK_HANDLER_MSG_DEF_TYPE
#include <task_handler_msgs.h>
MAX_NUM_TASK_HANDLER_MSGS
} TaskHandlerMsgType;

/*
 * MESSAGES BODIES: REQ MESSAGES
 */
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF MAKE_TASK_HANDLER_MSG_STRUCT_REQ
#include <task_handler_msgs.h>

/*
 * MESSAGES BODIES: CNF MESSAGES
 */
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF MAKE_TASK_HANDLER_MSG_STRUCT_CNF
#include <task_handler_msgs.h>

/*
 * MESSAGES BODIES: IND MESSAGES
 */
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF MAKE_TASK_HANDLER_MSG_STRUCT_IND
#include <task_handler_msgs.h>

/*
 * MESSAGE UNION: REQ MESSAGES
 */
typedef union TaskHandlerMsgUnionReqTag
{
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF MAKE_TASK_HANDLER_UNION_MEMBER_REQ
#include <task_handler_msgs.h>
} TaskHandlerMsgUnionReq;

/*
 * MESSAGE UNION: CNF MESSAGES
 */
typedef union TaskHandlerMsgUnionCnfTag
{
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF MAKE_TASK_HANDLER_UNION_MEMBER_CNF
#include <task_handler_msgs.h>
} TaskHandlerMsgUnionCnf;

/*
 * MESSAGE UNION: IND MESSAGES
 */
typedef union TaskHandlerMsgUnionIndTag
{
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF MAKE_TASK_HANDLER_UNION_MEMBER_IND
#include <task_handler_msgs.h>
} TaskHandlerMsgUnionInd;

#pragma pack(pop) /* End of packing */ 