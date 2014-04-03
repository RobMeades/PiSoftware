/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <state_machine.h>
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

static void defaultInit (RoboOneState *pState)
{
    printProgress ("Unsupported Init event in state %s.\n", pState->pName);
}

static void defaultInitFailure (RoboOneState *pState)
{
    printProgress ("Unsupported InitFailure event in state %s.\n", pState->pName);    
}

static void defaultTimerExpiry (RoboOneState *pState)
{
    printProgress ("Unsupported TimerExpiry event in state %s.\n", pState->pName);        
}

static void defaultTasksAvailable (RoboOneState *pState)
{
    printProgress ("Unsupported TasksAvailable event in state %s.\n", pState->pName);    
}

static void defaultNoTasksAvailable (RoboOneState *pState)
{
    printProgress ("Unsupported NoTasksAvailable event in state %s.\n", pState->pName);        
}

static void defaultMainsPowerAvailable (RoboOneState *pState)
{    
    printProgress ("Unsupported MainsPowerAvailable event in state %s.\n", pState->pName);    
}

static void defaultInsufficientPower (RoboOneState *pState)
{
    printProgress ("Unsupported InsufficientPower event in state %s.\n", pState->pName);    
}

static void defaultFullyCharged (RoboOneState *pState)
{
    printProgress ("Unsupported FullyCharged event in state %s.\n", pState->pName);
}

static void defaultShutdown (RoboOneState *pState)
{
    printProgress ("Unsupported Shutdown event in state %s.\n", pState->pName);
}

/*
 * PUBLIC FUNCTIONS:
 */

void defaultImplementation (RoboOneState *pState)
{
    pState->pInit = defaultInit;
    pState->pInitFailure = defaultInitFailure;
    pState->pTimerExpiry = defaultTimerExpiry;
    pState->pTasksAvailable = defaultTasksAvailable;
    pState->pNoTasksAvailable = defaultNoTasksAvailable;
    pState->pMainsPowerAvailable = defaultMainsPowerAvailable;
    pState->pInsufficientPower = defaultInsufficientPower;
    pState->pFullyCharged = defaultFullyCharged;
    pState->pShutdown = defaultShutdown;
}