/*
 * Mobile state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine.h>
#include <mobile_state.h>

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

void transitionToMobile (RoboOneState *pState)
{
    /* Fill in default handlers first */
    defaultImplementation (pState);
    pState->pName = "Mobile";

}