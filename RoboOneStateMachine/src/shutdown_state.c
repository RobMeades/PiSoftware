/*
 * Shutdown state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <shutdown_state.h>

/*
 * MANIFEST CONSTANTS
 */

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
    /* Fill in default handlers first */
    defaultImplementation (pState);
    pState->pName = "Shutdown";

}