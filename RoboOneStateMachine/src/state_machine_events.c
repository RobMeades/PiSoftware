/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <task_handler_types.h>
#include <state_machine_server.h>
#include <state_machine_events.h>
#include <init_state.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */
void eventInitRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventInit (&(pInstance->state));
}

void eventInitFailureRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventInitFailure (&(pInstance->state));
}

void eventTimerExpiryRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventTimerExpiry (&(pInstance->state));
}

void eventTasksAvailableRoboOne (RoboOneContext *pInstance, RoboOneTaskReq *pTaskReq)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventTasksAvailable (&(pInstance->state), pTaskReq);
}

void eventNoTasksAvailableRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventNoTasksAvailable (&(pInstance->state));
}

void eventMainsPowerAvailableRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventMainsPowerAvailable (&(pInstance->state));
}

void eventInsufficientPowerRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventMainsPowerAvailable (&(pInstance->state));
}

void eventFullyChargedRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventFullyCharged (&(pInstance->state));
}

void eventShutdownRoboOne (RoboOneContext *pInstance)
{
    ASSERT_PARAM (pInstance != PNULL, (unsigned long) pInstance);
    pInstance->state.pEventShutdown (&(pInstance->state));
}