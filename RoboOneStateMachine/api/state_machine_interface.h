/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

typedef struct RoboOneStateTag
{
    Char *pName;
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

/*
 * FUNCTION PROTOTYPES
 */

void defaultImplementation (RoboOneState *pState);