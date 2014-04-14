/*
 * BatteryIdle state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <batteryidle_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define BATTERYIDLE_STATE_NAME "BatteryIdle" /* Not longer than STATE_NAME_STRING_LENGTH -1 characters */

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
    /* Fill in default handlers first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), BATTERYIDLE_STATE_NAME, strlen (BATTERYIDLE_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("Transitioning to %s state.\n", &(pState->name[0]));
}