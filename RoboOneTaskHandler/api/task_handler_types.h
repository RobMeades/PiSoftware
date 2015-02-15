/*
 * Task Handler server types used in messages.
 * These are pulled out into a separate file, which
 * is allowed to nest the blah_protocol.h headers,
 * as messages general go to the Task Handler server
 * via the State Machine server (which needs to know
 * what's going on for stateful purposes) and hence the
 * types need to be available to all while the messages
 * only get to the Task Handler server via the State
 * Machine server. 
 */

#include <hindbrain_direct_task_protocol.h>
#include <motion_task_protocol.h>

/*
 * MANIFEST CONSTANTS
 */
#define MAX_LEN_IP_ADDRESS_STRING 17 /* Includes null terminator */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL TYPES USED IN MESSAGES
 */

/* The task protocols that can be employed */ 
typedef enum RoboOneTaskProtocolTag
{
    TASK_PROTOCOL_HD,     /* Hindbrain Direct */
    TASK_PROTOCOL_MOTION, /* Motion */
    MAX_NUM_TASK_PROTOCOL_TYPES
} RoboOneTaskProtocol;

/* Union of the REQ and IND bodies */
typedef union RoboOneTaskUnionTag
{
    RoboOneHDTaskReq hdReq;
    RoboOneHDTaskInd hdInd;
    RoboOneMotionTaskReq motionReq;
    RoboOneMotionTaskInd motionInd;
} RoboOneTaskUnion;

/* Generic container for both task REQ and task IND */
typedef struct RoboOneTaskContainerTag
{
    RoboOneTaskProtocol protocol;
    RoboOneTaskUnion detail;
} RoboOneTaskContainer;

/*
 * TYPES FOR REQ MESSAGES
 */

/* Header used on request messages if indications of progress are required */
typedef struct RoboOneTaskReqHeaderTag
{
    UInt32 handle;
    SInt32 sourceServerPort;
    Bool sourceServerIpAddressStringPresent;
    Char sourceServerIpAddressString[MAX_LEN_IP_ADDRESS_STRING];    
} RoboOneTaskReqHeader;

/* REQuest that a task is performed */
typedef struct RoboOneTaskReqTag
{
    Bool headerPresent;          /* If the header is present then INDications of command progress may be sent back */
    RoboOneTaskReqHeader header;
    RoboOneTaskContainer body;
} RoboOneTaskReq;

/*
 * TYPES FOR IND MESSAGES
 */

/* INDication about the progress of a task */
typedef struct RoboOneTaskIndTag
{
    UInt8 handle;
    RoboOneTaskContainer body;
} RoboOneTaskInd;

#pragma pack(pop) /* End of packing */