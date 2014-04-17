/*
 * Mobile state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <state_machine_public.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <events.h>
#include <actions.h>
#include <mobile_state.h>
#include <init_state.h>
#include <batteryidle_state.h>
#include <docked_state.h>
#include <shutdown_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define MOBILE_STATE_NAME "Mobile" /* Not longer than STATE_NAME_STRING_LENGTH -1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */
static void eventTasksAvailable (RoboOneState *pState)
{
    printDebug ("Unsupported Tasks Available event in state %s.\n", &(pState->name[0]));
}

/*
 * PUBLIC FUNCTIONS
 */

void transitionToMobile (RoboOneState *pState)
{
    Bool success = false;
    
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), MOBILE_STATE_NAME, strlen (MOBILE_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("Transitioning to %s state.\n", &(pState->name[0]));
    
    /* Now hook in the event handlers for this state */
    pState->pEventInitFailure = transitionToInit;
    pState->pEventTasksAvailable = eventTasksAvailable;
    pState->pEventNoTasksAvailable = transitionToBatteryIdle;
    pState->pEventMainsPowerAvailable = transitionToDocked;
    pState->pEventShutdown = transitionToShutdown;

    /* Do the entry actions */

    /* Switch Pi to battery power */
    success = switchPiRioToBatteryPower();
    
    if (success)
    {
        /* Switch Hindbrain to battery power */
        success = switchHindbrainToBatteryPower ();
        
        if (success)
        {
            /* Switch on Hindbrain */
            success = switchOnHindbrain ();
        }
    }
    
    if (!success)
    {
        /* If we cannot initialise, send an InitFailure event */
        stateMachineServerSendReceive (STATE_MACHINE_EVENT_INIT_FAILURE, PNULL, 0, PNULL, PNULL);        
    }
}