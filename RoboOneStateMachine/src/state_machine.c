/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <task_handler_types.h>
#include <state_machine_server.h>
#include <init_state.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS: DEFAULT EVENT HANDLERS
 */

static void defaultEventInit (RoboOneState *pState)
{
    printDebug ("Unsupported Init event in state %s.\n", &(pState->name[0]));
}

static void defaultEventInitFailure (RoboOneState *pState)
{
    printDebug ("Unsupported InitFailure event in state %s.\n", &(pState->name[0]));    
}

static void defaultEventTimerExpiry (RoboOneState *pState)
{
    printDebug ("Unsupported TimerExpiry event in state %s.\n", &(pState->name[0]));        
}

static void defaultEventTasksAvailable (RoboOneState *pState, RoboOneTaskReq *pTaskReq)
{
    printDebug ("Unsupported TasksAvailable event in state %s.\n", &(pState->name[0]));    
}

static void defaultEventNoTasksAvailable (RoboOneState *pState)
{
    printDebug ("Unsupported NoTasksAvailable event in state %s.\n", &(pState->name[0]));        
}

static void defaultEventMainsPowerAvailable (RoboOneState *pState)
{    
    printDebug ("Unsupported MainsPowerAvailable event in state %s.\n", &(pState->name[0]));    
}

static void defaultEventInsufficientPower (RoboOneState *pState)
{
    printDebug ("Unsupported InsufficientPower event in state %s.\n", &(pState->name[0]));    
}

static void defaultEventFullyCharged (RoboOneState *pState)
{
    printDebug ("Unsupported FullyCharged event in state %s.\n", &(pState->name[0]));
}

static void defaultEventInsufficientCharge (RoboOneState *pState)
{
    printDebug ("Unsupported InsufficientCharge event in state %s.\n", &(pState->name[0]));    
}

static void defaultEventShutdown (RoboOneState *pState)
{
    printDebug ("Unsupported Shutdown event in state %s.\n", &(pState->name[0]));
}

/*
 * PUBLIC FUNCTIONS:
 */

void defaultImplementation (RoboOneState *pState)
{
    pState->pEventInit = defaultEventInit;
    pState->pEventInitFailure = defaultEventInitFailure;
    pState->pEventTimerExpiry = defaultEventTimerExpiry;
    pState->pEventTasksAvailable = defaultEventTasksAvailable;
    pState->pEventNoTasksAvailable = defaultEventNoTasksAvailable;
    pState->pEventMainsPowerAvailable = defaultEventMainsPowerAvailable;
    pState->pEventInsufficientPower = defaultEventInsufficientPower;
    pState->pEventFullyCharged = defaultEventFullyCharged;
    pState->pEventInsufficientCharge = defaultEventInsufficientCharge;
    pState->pEventShutdown = defaultEventShutdown;
}