/*
 * Structures for the simple Hindbrain Direct task protocol.
 */

/*
 * MANIFEST CONSTANTS
 */

#define MAX_LEN_HINDBRAIN_DIRECT_COMMAND_STRING  30 /* Includes null terminator */
#define MAX_LEN_HINDBRAIN_DIRECT_RESPONSE_STRING 30 /* Includes null terminator */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL TYPES USED IN MESSAGES
 */

/*
 * TYPES FOR REQ MESSAGES
 */

typedef struct RoboOneHindbrainDirectTaskReqTag
{
  Char string[MAX_LEN_HINDBRAIN_DIRECT_COMMAND_STRING]; /* Must end with '\n' and then be null terminated */
    
} RoboOneHindbrainDirectTaskReq;

/*
 * TYPES FOR CNF MESSAGES
 */

typedef struct RoboOneHindbrainDirectTaskCnfTag
{
  Char string[MAX_LEN_HINDBRAIN_DIRECT_RESPONSE_STRING]; /* Will be null terminated */
  UInt8 stringLength;                                    /* Length includes null terminator */ 
} RoboOneHindbrainDirectTaskCnf;

/*
 * TYPES FOR IND MESSAGES
 */

typedef RoboOneHindbrainDirectTaskCnf RoboOneHindbrainDirectTaskInd;

#pragma pack(pop) /* End of packing */ 