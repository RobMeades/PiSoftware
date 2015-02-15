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

/*
 * FUNCTION PROTOTYPES
 */
void eventInitRoboOne (RoboOneContext *pInstance);
void eventInitFailureRoboOne (RoboOneContext *pInstance);
void eventTimerExpiryRoboOne (RoboOneContext *pInstance);
void eventTasksAvailableRoboOne (RoboOneContext *pInstance, RoboOneTaskReq *pTaskReq);
void eventNoTasksAvailableRoboOne (RoboOneContext *pInstance);
void eventMainsPowerAvailableRoboOne (RoboOneContext *pInstance);
void eventInsufficientPowerRoboOne (RoboOneContext *pInstance);
void eventFullyChargedRoboOne (RoboOneContext *pInstance);
void eventInsufficientChargeRoboOne (RoboOneContext *pInstance);
void eventShutdownRoboOne (RoboOneContext *pInstance);