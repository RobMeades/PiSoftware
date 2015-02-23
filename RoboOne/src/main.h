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

/* Settings, driven by the command line */
typedef struct RoboOneSettingsTag
{
    char * pTerminal;
    UInt32 baudRate;
} RoboOneSettings;

typedef struct RoboOneGlobalsTag
{
    RoboOneTaskInfo roboOneTaskInfo;
    RoboOneSettings roboOneSettings;
} RoboOneGlobals;

/*
 *  FUNCTION PROTOTYPES
 */