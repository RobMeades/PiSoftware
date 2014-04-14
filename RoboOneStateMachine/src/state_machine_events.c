/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <state_machine_public.h>
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
    pInstance->state.pEventInit (&(pInstance->state));
}

void eventInitFailureRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventInitFailure (&(pInstance->state));
}

void eventTimerExpiryRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventTimerExpiry (&(pInstance->state));
}

void eventTasksAvailableRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventTasksAvailable (&(pInstance->state));
}

void eventNoTasksAvailableRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventNoTasksAvailable (&(pInstance->state));
}

void eventMainsPowerAvailableRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventMainsPowerAvailable (&(pInstance->state));
}

void eventInsufficientPowerRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventMainsPowerAvailable (&(pInstance->state));
}

void eventFullyChargedRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventFullyCharged (&(pInstance->state));
}

void eventShutdownRoboOne (RoboOneContext *pInstance)
{
    pInstance->state.pEventShutdown (&(pInstance->state));
}