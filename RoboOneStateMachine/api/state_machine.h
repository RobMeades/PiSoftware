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
    void (*pInit) (struct RoboOneStateTag *pState);
    void (*pInitFailure) (struct RoboOneStateTag *pState);
    void (*pTimerExpiry) (struct RoboOneStateTag *pState);
    void (*pTasksAvailable) (struct RoboOneStateTag *pState);
    void (*pNoTasksAvailable) (struct RoboOneStateTag *pState);
    void (*pMainsPowerAvailable) (struct RoboOneStateTag *pState);
    void (*pInsufficientPower) (struct RoboOneStateTag *pState);
    void (*pFullyCharged) (struct RoboOneStateTag *pState);
    void (*pShutdown) (struct RoboOneStateTag *pState);
} RoboOneState;

/*
 * FUNCTION PROTOTYPES
 */
void defaultImplementation (RoboOneState *pState);
