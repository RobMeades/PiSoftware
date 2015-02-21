/*
 * Docked state - should be a transitory state, will shift to one
 * of the two substates when the battery state has been determined.
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <task_handler_types.h>
#include <hardware_types.h>
#include <battery_manager_server.h>
#include <battery_manager_msg_auto.h>
#include <battery_manager_client.h>
#include <state_machine_server.h>
#include <actions.h>
#include <docked_state.h>
#include <docked_charging_state.h>
#include <docked_mainsidle_state.h>
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
    Bool chargingPermitted = true;
    
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), DOCKED_STATE_NAME, strlen (DOCKED_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("\n*** Transitioning to %s state.\n", &(pState->name[0]));
    
    /* Now hook in the event handlers for this state */
    pState->pEventTasksAvailable = transitionToMobile;
    pState->pEventInsufficientPower = transitionToShutdown;
    pState->pEventShutdown = transitionToShutdown;
    pState->pEventFullyCharged = transitionToDocked_MainsIdle;
    pState->pEventInsufficientCharge = transitionToDocked_Charging;

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
    
    /* Let the Battery Manager know that charging is possible */
     batteryManagerServerSendReceive (BATTERY_MANAGER_CHARGING_PERMITTED, &chargingPermitted, sizeof (chargingPermitted), PNULL);
}