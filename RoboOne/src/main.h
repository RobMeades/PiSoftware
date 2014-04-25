/*
 * Global types for RoboOne.
 */ 

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

typedef struct RoboOneTaskInfoTag
{
    UInt32 taskCounter;    
    Char lastTaskSent[MAX_O_STRING_LENGTH];
    Bool lastResultReceivedIsValid;
    RoboOneHDResult lastResultReceived;
    Char lastIndString[MAX_O_STRING_LENGTH];    
} RoboOneTaskInfo;

typedef struct RoboOneGlobalsTag
{
    RoboOneTaskInfo roboOneTaskInfo;    
} RoboOneGlobals;

/*
 *  FUNCTION PROTOTYPES
 */