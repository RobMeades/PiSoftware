/*
 * Entry point for the RoboOne State Machine
 */ 
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <state_machine_interface.h>
#include <state_machine_public.h>

/*
 * main
 */
int main (int argc, char **argv)
{
    Bool success = true;

    setDebugPrintsOn();
    setProgressPrintsOn();

    printProgress ("RoboOne state machine started.\n");
    
    if (!success)
    {
        exit (-1);
    }
    
    return success;
}
