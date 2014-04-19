/*
 * Init state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <actions.h>
#include <init_state.h>
#include <docked_state.h>
#include <mobile_state.h>
#include <batteryidle_state.h>
#include <shutdown_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define INIT_STATE_NAME "Init" /* Not longer than STATE_NAME_STRING_LENGTH - 1 characters */

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
    Bool success = false;
    
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), INIT_STATE_NAME, strlen (INIT_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("Transitioning to %s state.\n", &(pState->name[0]));

    /* Now hook in the event handlers for this state */
    pState->pEventInitFailure = transitionToShutdown;
    pState->pEventTimerExpiry = transitionToBatteryIdle;
    pState->pEventTasksAvailable = transitionToMobile;
    pState->pEventMainsPowerAvailable = transitionToDocked;
    pState->pEventInsufficientPower = transitionToShutdown;
    pState->pEventShutdown = transitionToShutdown;
    
    /* Do the entry actions */
    
    /* Initialise Pi?  Assume it's already initialised */
    /* Initialise XBee?  Assume it's already initialised */
    /* Initialise OneWire devices?  Already initialised by RoboOne main() */
    
    /* Switch Pi and Hindbrain back to battery power in case they have been on mains (don't care about failure) */
    actionSwitchPiRioToBatteryPower();
    actionSwitchHindbrainToBatteryPower();
    
    /* Switch all the relays on */
    success = actionEnableAllRelays();
    if (success)
    {
        /* Power up hindbrain to see if all is good */
        success = actionSwitchOnHindbrain();
        if (success)
        {
            /* TODO: Initialise RIO AHRS (or check that it is initialised) */
            /* TODO: Check status of Wifi and camera */
            /* TODO: Start inactivity timer */
        }        
    }
            
    if (!success)
    {
        /* If we cannot initialise, send an InitFailure event */
        stateMachineServerSendReceive (STATE_MACHINE_EVENT_INIT_FAILURE, PNULL, 0, PNULL, PNULL);
    }
}