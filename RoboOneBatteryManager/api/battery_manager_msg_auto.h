/*
 * Auto generated stuff: the message types, structures and unions
 * This header file is a bit special in that it is allowed to
 * include others.  None are gated, so be careful not to get
 * circular.
 */

#include <battery_manager_msg_macros.h>
#include <battery_manager_msgs.h>

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * MESSAGE TYPES
 */
typedef enum BatteryManagerMsgTypeTag
{
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF BATTERY_MANAGER_MSG_DEF_TYPE
#include <battery_manager_msgs.h>
MAX_NUM_BATTERY_MANAGER_MSGS
} BatteryManagerMsgType;

/*
 * MESSAGES BODIES: IND MESSAGES
 */
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_MSG_STRUCT_IND
#include <battery_manager_msgs.h>

/*
 * MESSAGES BODIES: REQ MESSAGES
 */
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_MSG_STRUCT_REQ
#include <battery_manager_msgs.h>

/*
 * MESSAGES BODIES: RSP MESSAGES
 */
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_MSG_STRUCT_RSP
#include <battery_manager_msgs.h>

/*
 * MESSAGES BODIES: CNF MESSAGES
 */
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_MSG_STRUCT_CNF
#include <battery_manager_msgs.h>

/*
 * MESSAGE UNION: IND MESSAGES
 */
typedef union BatteryManagerMsgUnionIndTag
{
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_UNION_MEMBER_IND
#include <battery_manager_msgs.h>
} BatteryManagerMsgUnionInd;

/*
 * MESSAGE UNION: REQ MESSAGES
 */
typedef union BatteryManagerMsgUnionReqTag
{
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_UNION_MEMBER_REQ
#include <battery_manager_msgs.h>
} BatteryManagerMsgUnionReq;

/*
 * MESSAGE UNION: RSP MESSAGES
 */
typedef union BatteryManagerMsgUnionRspTag
{
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_UNION_MEMBER_RSP
#include <battery_manager_msgs.h>
} BatteryManagerMsgUnionRsp;

/*
 * MESSAGE UNION: CNF MESSAGES
 */
typedef union BatteryManagerMsgUnionCnfTag
{
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF MAKE_BATTERY_MANAGER_UNION_MEMBER_CNF
#include <battery_manager_msgs.h>
} BatteryManagerMsgUnionCnf;

#pragma pack(pop) /* End of packing */ 