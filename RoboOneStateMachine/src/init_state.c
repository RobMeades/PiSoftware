/*
 * Init state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine.h>
#include <init_state.h>

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

void transitionToInit (RoboOneState *pState)
{
    /* Fill in default handlers first */
    defaultImplementation (pState);
    pState->pName = "Init";
}