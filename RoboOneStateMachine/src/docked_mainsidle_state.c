/*
 * Docked - Mains Idle state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_server.h>
#include <docked_mainsidle_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define DOCKED_MAINSIDLE_STATE_NAME "Docked - MainsIdle" /* Not longer than STATE_NAME_STRING_LENGTH - 1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */

/*
 * PUBLIC FUNCTIONS
 */

void transitionToDocked_MainsIdle (RoboOneState *pState)
{
    /* Fill in default handlers first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), DOCKED_MAINSIDLE_STATE_NAME, strlen (DOCKED_MAINSIDLE_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("\n*** Transitioning to %s state.\n", &(pState->name[0]));
}