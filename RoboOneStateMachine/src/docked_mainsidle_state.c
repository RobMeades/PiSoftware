/*
 * Docked - Mains Idle state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <docked_mainsidle_state.h>

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

void transitionToDocked_MainsIdle (RoboOneState *pState)
{
    /* Fill in default handlers first */
    defaultImplementation (pState);
    pState->pName = "MainsIdle";
}