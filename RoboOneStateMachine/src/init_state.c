/*
 * Init state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <init_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define INIT_STATE_NAME "Init" /* Not longer than STATE_NAME_STRING_LENGTH -1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */

/*
 * PUBLIC FUNCTIONS
 */

void transitionToInit (RoboOneState *pState)
{
    /* Fill in default handlers first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), INIT_STATE_NAME, strlen (INIT_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("Transitioning to %s state.\n", &(pState->name[0]));
}