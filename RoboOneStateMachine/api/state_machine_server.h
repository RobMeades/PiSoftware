/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */

/*
 * MANIFEST CONSTANTS
 */
#define STATE_MACHINE_SERVER_EXE "./roboone_state_machine_server"
#define STATE_MACHINE_SERVER_PORT_STRING "5231"

#define STATE_NAME_STRING_LENGTH 25

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE TYPES
 */

typedef struct RoboOneStateTag
{
    Char name[STATE_NAME_STRING_LENGTH];
    void (*pEventInit) (struct RoboOneStateTag *pState);
    void (*pEventInitFailure) (struct RoboOneStateTag *pState);
    void (*pEventTimerExpiry) (struct RoboOneStateTag *pState);
    void (*pEventTasksAvailable) (struct RoboOneStateTag *pState, RoboOneTaskReq *pTaskReq);
    void (*pEventNoTasksAvailable) (struct RoboOneStateTag *pState);
    void (*pEventMainsPowerAvailable) (struct RoboOneStateTag *pState);
    void (*pEventInsufficientPower) (struct RoboOneStateTag *pState);
    void (*pEventFullyCharged) (struct RoboOneStateTag *pState);
    void (*pEventInsufficientCharge) (struct RoboOneStateTag *pState);
    void (*pEventShutdown) (struct RoboOneStateTag *pState);
} RoboOneState;

typedef struct RoboOneContextTag
{
  RoboOneState state;
} RoboOneContext;

/*
 * TYPES FOR REQ MESSAGES
 */

typedef struct RoboOneContextContainerTag
{
    Bool isValid;
    RoboOneContext context;
} RoboOneContextContainer;

/*
 * TYPES FOR CNF MESSAGES
 */

#pragma pack(pop) /* End of packing */ 

/*
 * FUNCTION PROTOTYPES
 */

void defaultImplementation (RoboOneState *pState);
