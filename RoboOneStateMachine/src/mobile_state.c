/*
 * Mobile state
 */
#include <stdio.h>
#include <string.h>
#include <rob_system.h>
#include <task_handler_types.h>
#include <task_handler_server.h>
#include <task_handler_msg_auto.h>
#include <task_handler_client.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <actions.h>
#include <mobile_state.h>
#include <init_state.h>
#include <batteryidle_state.h>
#include <docked_state.h>
#include <shutdown_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define MOBILE_STATE_NAME "Mobile" /* Not longer than STATE_NAME_STRING_LENGTH - 1 characters */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: EVENT HANDLERS
 */

/*
 * Handle the arrival of a task.
 * 
 * pState    pointer to the state structure.
 * pTaskReq  pointer to the new task.
 */
static void eventTasksAvailable (RoboOneState *pState, RoboOneTaskReq *pTaskReq)
{
    Bool success;
    
    ASSERT_PARAM (pState != PNULL, (unsigned long) pState);
    ASSERT_PARAM (pTaskReq != PNULL, (unsigned long) pTaskReq);

    /* Forward the task to the task handler */
    success = taskHandlerServerSendReceive (TASK_HANDLER_NEW_TASK, pTaskReq, sizeof (*pTaskReq));
    
    /* TODO: something more sensible than this */
    if (!success)
    {
        printDebug ("!!!Task handler failed to accept command!!!.\n");
    }
}

/*
 * PUBLIC FUNCTIONS
 */
void transitionToMobile (RoboOneState *pState, RoboOneTaskReq *pTaskReq)
{
    Bool success = false;
    
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), MOBILE_STATE_NAME, strlen (MOBILE_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("\n*** Transitioning to %s state.\n", &(pState->name[0]));
    
    /* Now hook in the event handlers for this state */
    pState->pEventInitFailure = transitionToInit;
    pState->pEventTasksAvailable = eventTasksAvailable;
    pState->pEventNoTasksAvailable = transitionToBatteryIdle;
    pState->pEventMainsPowerAvailable = transitionToDocked;
    pState->pEventShutdown = transitionToShutdown;

    /* Do the entry actions */

    /* Switch Pi to battery power */
    success = actionSwitchPiRioToBatteryPower();
    
    if (success)
    {
        /* Switch Hindbrain to battery power */
        success = actionSwitchHindbrainToBatteryPower ();
        
        if (success)
        {
            /* Switch on Hindbrain */
            success = actionSwitchOnHindbrain ();
            
            if (success)
            {
                /* Now handle the event */
                eventTasksAvailable (pState, pTaskReq);
            }
        }
    }
    
    if (!success)
    {
        /* If we cannot initialise, send an InitFailure event */
        stateMachineServerSendReceive (STATE_MACHINE_EVENT_INIT_FAILURE, PNULL, 0, PNULL, PNULL);        
    }
}