/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <state_machine.h>
#include <init_state.h>

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
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

RoboOneContext * createRoboOneStateMachine (void)
{
    RoboOneContext *pInstance;
    
    pInstance = malloc (sizeof (RoboOneContext));
    
    if (pInstance != PNULL)
    {
        transitionToInit (&(pInstance->state));
    }   
    
    return pInstance;
}

void destroyRoboOneStateMachine (RoboOneContext *pInstance)
{
    free (pInstance);
}

void initRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pInit (&(pInstance->state));
}

void initFailureRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pInitFailure (&(pInstance->state));
}

void timerExpiryRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pTimerExpiry (&(pInstance->state));
}

void tasksAvailableRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pTasksAvailable (&(pInstance->state));
}

void noTasksAvailableRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pNoTasksAvailable (&(pInstance->state));
}

void mainsPowerAvailableRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pMainsPowerAvailable (&(pInstance->state));
}

void insufficientPowerRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pMainsPowerAvailable (&(pInstance->state));
}

void fullyChargedRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pFullyCharged (&(pInstance->state));
}

void shutdownRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pShutdown (&(pInstance->state));
}