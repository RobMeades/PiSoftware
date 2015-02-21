/*
 * Shutdown state
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
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <actions.h>
#include <shutdown_state.h>

/*
 * MANIFEST CONSTANTS
 */
#define SHUTDOWN_STATE_NAME "Shutdown" /* Not longer than STATE_NAME_STRING_LENGTH - 1 characters */

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
    Bool chargingPermitted = false;
    
    /* Fill in default handlers and name first */
    defaultImplementation (pState);
    memcpy (&(pState->name[0]), SHUTDOWN_STATE_NAME, strlen (SHUTDOWN_STATE_NAME) + 1); /* +1 for terminator */
    printDebug ("\n*** Transitioning to %s state.\n", &(pState->name[0]));

    /* There are no event handlers for this state */
    
    /* Do the entry actions */

    /* Let the Battery Manager know that charging is no longer possible */
    batteryManagerServerSendReceive (BATTERY_MANAGER_CHARGING_PERMITTED, &chargingPermitted, sizeof (chargingPermitted), PNULL);
    
    /* Switch off Hindbrain and disable relays */
    actionSwitchOffHindbrain();
    actionDisableExternalRelays();
    actionDisableOnPCBRelays();
    
    /* TODO: put Pi to sleep/ */

}