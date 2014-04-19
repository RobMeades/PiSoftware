/*
 * One Wire bus handling thread for RoboOne.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <rob_system.h>
#include <one_wire.h>
#include <dashboard.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * EXTERNS
 */

extern int errno;

/*
 * GLOBALS (prefixed with g)
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * Send a message to stop the Hardware Server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool stopHardwareServer (void)
{
    return hardwareServerSendReceive (HARDWARE_SERVER_EXIT, PNULL, 0, PNULL);
}

/*
 * Send a message to start the state machine server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool startStateMachineServer (void)
{
    Bool success = false;
    StateMachineMsgType receivedMsgType = STATE_MACHINE_SERVER_NULL;
    StateMachineServerStartCnf *pStateMachineServerStartCnf;
    
    pStateMachineServerStartCnf = malloc (sizeof (StateMachineServerStartCnf));
    
    if (pStateMachineServerStartCnf != PNULL)
    {   
        pStateMachineServerStartCnf->success = false;
        success = stateMachineServerSendReceive (STATE_MACHINE_SERVER_START, PNULL, 0, &receivedMsgType, pStateMachineServerStartCnf);
        
        if ((receivedMsgType != STATE_MACHINE_SERVER_START) || !pStateMachineServerStartCnf->success)
        {
            success = false;
        }
        
        free (pStateMachineServerStartCnf);
    }
    
    return success;
}

/*
 * Send a message to stop the state machine server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool stopStateMachineServer (void)
{
    Bool success = false;
    StateMachineMsgType receivedMsgType = STATE_MACHINE_SERVER_NULL;
    StateMachineServerStopCnf *pStateMachineServerStopCnf;
    
    pStateMachineServerStopCnf = malloc (sizeof (StateMachineServerStopCnf));
    
    if (pStateMachineServerStopCnf != PNULL)
    {
        pStateMachineServerStopCnf->success = false;
        success = stateMachineServerSendReceive (STATE_MACHINE_SERVER_STOP, PNULL, 0, &receivedMsgType, pStateMachineServerStopCnf);
        
        if ((receivedMsgType != STATE_MACHINE_SERVER_STOP) || !pStateMachineServerStopCnf->success)
        {
            success = false;
        }
        
        free (pStateMachineServerStopCnf);
    }
    
    return success;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool   success = false;
    pid_t  serverPID;
    pid_t  stateMachinePID;
    
    setDebugPrintsOn();
    setProgressPrintsOn();

    /* Spawn a child that will become the Hardware server. */
    serverPID = fork();
    if (serverPID == 0)
    {
        /* Start OneWire server process on a given port */
        static char *argv1[] = {HARDWARE_SERVER_EXE, HARDWARE_SERVER_PORT_STRING, PNULL};
        
        execv (HARDWARE_SERVER_EXE, argv1);
        printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", HARDWARE_SERVER_EXE, strerror (errno));
    }
    else
    {
        if (serverPID < 0)
        {
            printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", HARDWARE_SERVER_EXE, strerror (errno));
        }
        else
        {   /* Parent process */
            /* Wait for the server to start */
            usleep (SERVER_START_DELAY_PI_US);

            /* Spawn a child that will become the RoboOne state machine. */
            stateMachinePID = fork();
            if (stateMachinePID == 0)
            {
                /* Start RoboOne state machine process */
                static char *argv2[] = {STATE_MACHINE_SERVER_EXE, STATE_MACHINE_SERVER_PORT_STRING, PNULL};
                  
                execv (STATE_MACHINE_SERVER_EXE, argv2);
                printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", STATE_MACHINE_SERVER_EXE, strerror (errno));
            }
            else
            {
                if (stateMachinePID < 0)
                {
                    printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", STATE_MACHINE_SERVER_EXE, strerror (errno));
                }
                else
                {   /* Parent process again */
                    /* Now setup the state machine */
                    usleep (SERVER_START_DELAY_PI_US); /* Wait for the server to be ready before messaging it */
                    success = startStateMachineServer();
                    
                    if (success)
                    {
                        /* Finally, display the dashboard */
                        success = runDashboard();
                        
                        printProgress ("\nDone.\n");
                    }
                    
                    /* When done, tidy up the state machine */
                    stopStateMachineServer();
                    waitpid (stateMachinePID, 0, 0); /* wait for state machine to exit */
                }
            }
            if (!success)
            {
                printProgress ("\nFailed!\n");
            }
                
            /* Shut the hardware gracefully */
            stopHardwareServer();
            waitpid (serverPID, 0, 0); /* wait for server to exit */
        }
    }
    
    if (!success)
    {
        exit (-1);
    }
    
    return success;
}