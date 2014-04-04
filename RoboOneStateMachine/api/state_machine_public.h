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

typedef struct RoboOneContextTag
{
  RoboOneState state;
} RoboOneContext;

/*
 * FUNCTION PROTOTYPES
 */

RoboOneContext * createRoboOneStateMachine (void);
void destroyRoboOneStateMachine (RoboOneContext *pInstance);

void eventInitRoboOne (RoboOneContext *pInstance);
void eventInitFailureRoboOne (RoboOneContext *pInstance);
void eventTimerExpiryRoboOne (RoboOneContext *pInstance);
void eventTasksAvailableRoboOne (RoboOneContext *pInstance);
void eventNoTasksAvailableRoboOne (RoboOneContext *pInstance);
void eventMainsPowerAvailableRoboOne (RoboOneContext *pInstance);
void eventInsufficientPowerRoboOne (RoboOneContext *pInstance);
void eventFullyChargedRoboOne (RoboOneContext *pInstance);
void eventShutdownRoboOne (RoboOneContext *pInstance);