/*
 * Docked - Charging state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine.h>
#include <docked_charging_state.h>

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

void transitionToDocked_Charging (RoboOneState *pState)
{
    /* Fill in default handlers first */
    defaultImplementation (pState);
    pState->pName = "Docked - Charging";
}