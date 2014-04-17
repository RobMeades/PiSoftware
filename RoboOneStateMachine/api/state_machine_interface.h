/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */

/*
 * MANIFEST CONSTANTS
 */
#define STATE_NAME_STRING_LENGTH 25


/*
 * TYPES
 */
#pragma pack(push, 1) /* This structure is used in messages and so HAS to be fully packed */

typedef struct RoboOneStateTag
{
    Char name[STATE_NAME_STRING_LENGTH];
    void (*pEventInit) (struct RoboOneStateTag *pState);
    void (*pEventInitFailure) (struct RoboOneStateTag *pState);
    void (*pEventTimerExpiry) (struct RoboOneStateTag *pState);
    void (*pEventTasksAvailable) (struct RoboOneStateTag *pState);
    void (*pEventNoTasksAvailable) (struct RoboOneStateTag *pState);
    void (*pEventMainsPowerAvailable) (struct RoboOneStateTag *pState);
    void (*pEventInsufficientPower) (struct RoboOneStateTag *pState);
    void (*pEventFullyCharged) (struct RoboOneStateTag *pState);
    void (*pEventShutdown) (struct RoboOneStateTag *pState);
} RoboOneState;

#pragma pack(pop) /* End of packing */ 

/*
 * FUNCTION PROTOTYPES
 */

void defaultImplementation (RoboOneState *pState);