/*
 * Main for RoboOne.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <pthread.h>
#include <rob_system.h>
#include <monitor.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <local_server.h>
#include <task_handler_types.h>
#include <task_handler_server.h>
#include <task_handler_msg_auto.h>
#include <task_handler_client.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <hardware_types.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <battery_manager_server.h>
#include <battery_manager_msg_auto.h>
#include <battery_manager_client.h>
#include <main.h>

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
RoboOneGlobals gRoboOneGlobals;

/*
 * STATIC FUNCTIONS
 */

/*
 * A local server that listens out for
 * the progress of completing tasks.
 * This local server should never return,
 * it is started in its own thread and
 * cancelled by main().
 * 
 * pServerPort pointer to the port to use.
 */
void *localServer (void * serverPort)
{
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;

    printProgress ("RO Server listening on port %d.\n", (SInt32) serverPort);
    returnCode = runMessagingServer ((SInt32) serverPort);
    
    ASSERT_ALWAYS_PARAM (returnCode);
    
    pthread_exit (&returnCode);
}

/*
 * Initalise globals.
 */
static void initGlobals (void)
{
    gRoboOneGlobals.roboOneTaskInfo.taskCounter = 0;    
    gRoboOneGlobals.roboOneTaskInfo.lastTaskSent[0] = 0;
    gRoboOneGlobals.roboOneTaskInfo.lastResultReceivedIsValid = false;
    gRoboOneGlobals.roboOneTaskInfo.lastIndString[0] = 0;    
}

/*
 * Start a local server in it's own thread
 * that listens out for the progress of task
 * completion.
 * 
 * serverPort         the port to use
 * pLocalServerThread pointer to a place
 *                    to store the thread
 *                    details
 * 
 * @return true if successful, otherwise false.
 */
static Bool startLocalServerThread (pthread_t *pLocalServerThread, SInt32 serverPort)
{
    Bool success = true;

    ASSERT_PARAM (pLocalServerThread != PNULL, (unsigned long) pLocalServerThread);

    if (pthread_create (pLocalServerThread, NULL, localServer, (void *) serverPort) != 0)
    {
        success = false;
        printDebug ("!!! Couldn't create RO Server thread, err: %s. !!!\n", strerror (errno));
    }
    
    return success;
}

/*
 * Stop the local server thread.
 * 
 * pLocalServerThread pointer to the thread
 *                    details
 * 
 * @return            true if successful,
 *                    otherwise false.
 */
static Bool stopLocalServerThread (pthread_t *pLocalServerThread)
{
    Bool success = true;

    ASSERT_PARAM (pLocalServerThread != PNULL, (unsigned long) pLocalServerThread);

    if (pthread_cancel (*pLocalServerThread) != 0)
    {
        success = false;
        printDebug ("!!! Couldn't cancel RO Server thread, err: %s. !!!\n", strerror (errno));
    }
    
    return success;
}

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
 * Send a message to start the task handler server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool startTaskHandlerServer (void)
{
    return taskHandlerServerSendReceive (TASK_HANDLER_SERVER_START, PNULL, 0);
}

/*
 * Send a message to stop the task handler server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool stopTaskHandlerServer (void)
{
    return taskHandlerServerSendReceive (TASK_HANDLER_SERVER_STOP, PNULL, 0);
}

/*
 * Send a message to start the battery manager server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool startBatteryManagerServer (void)
{
    return batteryManagerServerSendReceive (BATTERY_MANAGER_SERVER_START, PNULL, 0, PNULL);
}

/*
 * Send a message to stop the battery manager server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool stopBatteryManagerServer (void)
{
    return batteryManagerServerSendReceive (BATTERY_MANAGER_SERVER_STOP, PNULL, 0, PNULL);
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
    
    pStateMachineServerStartCnf = malloc (sizeof (*pStateMachineServerStartCnf));
    
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
    
    pStateMachineServerStopCnf = malloc (sizeof (*pStateMachineServerStopCnf));
    
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
    pid_t  hwServerPID;
    pid_t  thServerPID;
    pid_t  smServerPID;
    pid_t  bmServerPID;
    pthread_t localServerThread;
    
    setDebugPrintsOnToFile ("roboone.log");
    setProgressPrintsOn();

    initGlobals();
    
    /* Spawn a child that will become the Hardware server. */
    hwServerPID = fork();
    if (hwServerPID == 0)
    {
        /* Start Hardware server process on a given port */
        static char *argv1[] = {HARDWARE_SERVER_EXE, HARDWARE_SERVER_PORT_STRING, PNULL};
        
        execv (HARDWARE_SERVER_EXE, argv1);
        printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", HARDWARE_SERVER_EXE, strerror (errno));
    }
    else
    {
        if (hwServerPID < 0)
        {
            printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", HARDWARE_SERVER_EXE, strerror (errno));
        }
        else
        {   /* Parent process */
            /* Wait for the server to start */
            usleep (HARDWARE_SERVER_START_DELAY_PI_US);

            /* Spawn a child that will become the Task Handler server. */
            thServerPID = fork();
            if (thServerPID == 0)
            {
                /* Start Task Handler server process on a given port */
                static char *argv1[] = {TASK_HANDLER_SERVER_EXE, TASK_HANDLER_SERVER_PORT_STRING, PNULL};
                
                execv (TASK_HANDLER_SERVER_EXE, argv1);
                printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", TASK_HANDLER_SERVER_EXE, strerror (errno));
            }
            else
            {
                if (thServerPID < 0)
                {
                    printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", TASK_HANDLER_SERVER_EXE, strerror (errno));
                }
                else
                {   /* Parent process */
                    /* Wait for the server to start */
                    usleep (TASK_HANDLER_SERVER_START_DELAY_PI_US);
                    success = startTaskHandlerServer();

                    if (success)
                    {
                        /* Spawn a child that will become the RoboOne Battery Manager server. */
                        bmServerPID = fork();
                        if (bmServerPID == 0)
                        {
                            /* Start RoboOne Battery Manager server process */
                            static char *argv2[] = {BATTERY_MANAGER_SERVER_EXE, BATTERY_MANAGER_SERVER_PORT_STRING, PNULL};
                              
                            execv (BATTERY_MANAGER_SERVER_EXE, argv2);
                            printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", BATTERY_MANAGER_SERVER_EXE, strerror (errno));
                        }
                        else
                        {
                            if (bmServerPID < 0)
                            {
                                printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", BATTERY_MANAGER_SERVER_EXE, strerror (errno));
                            }
                            else
                            {   /* Parent process again */
                                /* Now setup the RoboOne Battery Manager server */
                                usleep (BATTERY_MANAGER_SERVER_START_DELAY_PI_US); /* Wait for the server to be ready before messaging it */
                                success = startBatteryManagerServer();
                                        
                                /* Spawn a child that will become the RoboOne state machine. */
                                smServerPID = fork();
                                if (smServerPID == 0)
                                {
                                    /* Start RoboOne state machine process */
                                    static char *argv2[] = {STATE_MACHINE_SERVER_EXE, STATE_MACHINE_SERVER_PORT_STRING, PNULL};
                                      
                                    execv (STATE_MACHINE_SERVER_EXE, argv2);
                                    printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", STATE_MACHINE_SERVER_EXE, strerror (errno));
                                }
                                else
                                {
                                    if (smServerPID < 0)
                                    {
                                        printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", STATE_MACHINE_SERVER_EXE, strerror (errno));
                                    }
                                    else
                                    {   /* Parent process again */
                                        /* Now setup the state machine */
                                        usleep (STATE_MACHINE_SERVER_START_DELAY_PI_US); /* Wait for the server to be ready before messaging it */
                                        success = startStateMachineServer();

                                        if (success)
                                        {
                                            /* Start the local server that listens out for task progress indications */
                                            success = startLocalServerThread (&localServerThread, LOCAL_SERVER_PORT);
                                            
                                            if (success)
                                            {
                                                /* Finally, display the monitor to display things and generate events */
                                                success = runMonitor();
                                                
                                                /* Tidy up the local server now that we're done */
                                                stopLocalServerThread (&localServerThread);
                                            }
                                            
                                            printProgress ("\nDone.\n");
                                            
                                            /* When done, tidy up the battery manager server */
                                            stopBatteryManagerServer();
                                            waitpid (bmServerPID, 0, 0); /* wait for state machine to exit */
                                        }
                                    }
                                }
                            }
                            
                            /* When done, tidy up the state machine */
                            stopStateMachineServer();
                            waitpid (smServerPID, 0, 0); /* wait for state machine to exit */
                        }
                    }
                    
                    /* Tidy up the task handler */
                    stopTaskHandlerServer();
                    waitpid (thServerPID, 0, 0); /* wait for state machine to exit */
                }
            }
                
            /* Shut the hardware gracefully */
            stopHardwareServer();
            waitpid (hwServerPID, 0, 0); /* wait for server to exit */
        }
    }
    
    setDebugPrintsOff();

    if (!success)
    {
        printProgress ("\nFailed!\n");
        exit (-1);
    }

    return success;
}