/*
 * BatteryIdle state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <task_handler_types.h>
#include <state_machine_server.h>
#include <actions.h>
#include <batteryidle_state.h>
#include <mobile_state.h>
#include <docked_state.h>
#include <shutdown_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define BATTERYIDLE_STATE_NAME "BatteryIdle" /* Not longer than STATE_NAME_STRING_LENGTH - 1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */

/*
 * PUBLIC FUNCTIONS
 */

void transitionToBatteryIdle (RoboOneState *pState)
{
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), BATTERYIDLE_STATE_NAME, strlen (BATTERYIDLE_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("\n*** Transitioning to %s state.\n", &(pState->name[0]));
    
    /* Now hook in the event handlers for this state */
    pState->pEventTasksAvailable = transitionToMobile;
    pState->pEventMainsPowerAvailable = transitionToDocked;
    pState->pEventInsufficientPower = transitionToShutdown;
    pState->pEventShutdown = transitionToShutdown;

    /* Do the entry actions */

    /* Save power since there's nothing to do */
    actionSwitchOffHindbrain();
}