/*
 * Docked state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <task_handler_types.h>
#include <state_machine_server.h>
#include <actions.h>
#include <docked_state.h>
#include <shutdown_state.h>
#include <mobile_state.h>
/*
 * MANIFEST CONSTANTS
 */
#define DOCKED_STATE_NAME "Docked" /* Not longer than STATE_NAME_STRING_LENGTH - 1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */

/*
 * PUBLIC FUNCTIONS
 */

void transitionToDocked (RoboOneState *pState)
{
    Bool success;
    
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), DOCKED_STATE_NAME, strlen (DOCKED_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("\n*** Transitioning to %s state.\n", &(pState->name[0]));
    
    /* Now hook in the event handlers for this state */
    pState->pEventTasksAvailable = transitionToMobile;
    pState->pEventInsufficientPower = transitionToShutdown;
    pState->pEventShutdown = transitionToShutdown;

    /* Do the entry actions */

    /* Switch on the external relays now as we need to fiddle with Hindbrain power and chargers */
    success = actionEnableExternalRelays();
    
    if (success)
    {
        /* Switch Pi to 12V power - don't care about failure as this will be handled once insufficient power is available */
        actionSwitchPiRioTo12VMainsPower();
    
        /* Switch Hindbrain to 12V power - again, don't care about faiure */
        actionSwitchHindbrainTo12VMainsPower();
    
        /* Switch off the Hindbrain to save power - again, don't care about failure here */
        actionSwitchOffHindbrain();
    }
    
    /* TODO: enter the relevant sub-state */
}