/*
 * Docked - Charging state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <docked_charging_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define DOCKED_CHARGING_STATE_NAME "Docked - Charging" /* Not longer than STATE_NAME_STRING_LENGTH -1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */

/*
 * PUBLIC FUNCTIONS
 */

void transitionToDocked_Charging (RoboOneState *pState)
{
    /* Fill in default handlers first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), DOCKED_CHARGING_STATE_NAME, strlen (DOCKED_CHARGING_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("Transitioning to %s state.\n", &(pState->name[0]));
}