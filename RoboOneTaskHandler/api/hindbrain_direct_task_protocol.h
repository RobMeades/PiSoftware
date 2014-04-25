/*
 * Structures for the simple Hindbrain Direct task protocol.
 */

/*
 * MANIFEST CONSTANTS
 */

#define MAX_LEN_HD_COMMAND_STRING  30 /* Includes null terminator */
#define MAX_LEN_HD_RESPONSE_STRING 30 /* Includes null terminator */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL TYPES USED IN MESSAGES
 */

/*
 * TYPES FOR REQ MESSAGES
 */

typedef struct RoboOneHDTaskReqTag
{
  Char string[MAX_LEN_HD_COMMAND_STRING]; /* Must end with '\n' and then be null terminated */
    
} RoboOneHDTaskReq;

/*
 * TYPES FOR IND MESSAGES
 */
typedef enum RoboOneHDResultTag
{
    HD_RESULT_SUCCESS,
    HD_RESULT_GENERAL_FAILURE,
    HD_RESULT_SEND_FAILURE,
    HD_NUM_RESULTS
} RoboOneHDResult;

typedef struct RoboOneHDTaskIndTag
{
  RoboOneHDResult result;
  UInt8 stringLength;                      /* Length includes null terminator */ 
  Char string[MAX_LEN_HD_RESPONSE_STRING]; /* Will be null terminated */
} RoboOneHDTaskInd;

#pragma pack(pop) /* End of packing */ 