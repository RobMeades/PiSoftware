/*
 * Shutdown state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <actions.h>
#include <shutdown_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define SHUTDOWN_STATE_NAME "Shutdown" /* Not longer than STATE_NAME_STRING_LENGTH -1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */

/*
 * PUBLIC FUNCTIONS
 */

void transitionToShutdown (RoboOneState *pState)
{
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), SHUTDOWN_STATE_NAME, strlen (SHUTDOWN_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("Transitioning to %s state.\n", &(pState->name[0]));

    /* There are no event handlers for this state */
    
    /* Do the entry actions */

    switchOffHindbrain();

    /* TODO: disable all relays */
    /* TODO: put Pi to sleep/ */

}