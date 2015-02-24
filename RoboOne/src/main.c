/*
 * Main for RoboOne.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> /* for open ()*/
#include <termios.h> /* for baud rate enum */
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
RoboOneSettings gSettings;

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
    gRoboOneGlobals.roboOneSettings.pTerminal = NULL;
    gRoboOneGlobals.roboOneSettings.baudRate = B0;
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
 * Send a message to start the Hardware Server.
 *
 * batteriesOnly  if true, only setup the battery
 *                devices, otherwise setup the lot.
 * 
 * @return true if successful, otherwise false.
 */
static Bool startHardwareServer (Bool batteriesOnly)
{
    return hardwareServerSendReceive (HARDWARE_SERVER_START, &batteriesOnly, sizeof (batteriesOnly), PNULL);
}

/*
 * Send a message to stop the Hardware Server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool stopHardwareServer (void)
{
    return hardwareServerSendReceive (HARDWARE_SERVER_STOP, PNULL, 0, PNULL);
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
 * Set the baud rate based on a string.
 * 
 * pBaudRateString  the string.
 * pBaudRate        the baud rate from termios.
 * 
 * @return true if successful, otherwise false.
 */
static Bool setBaudRate (const char * pBaudRateString, UInt32 * pBaudRate)
{
    Bool success = false;
    
    ASSERT_PARAM (pBaudRateString != PNULL, (unsigned long) pBaudRateString);
    ASSERT_PARAM (pBaudRate != PNULL, (unsigned long) pBaudRate);
    
    if (strstr (pBaudRateString, "300") == pBaudRateString)
    {
        *pBaudRate = B300;
        success = true;
    }
    else if (strstr (pBaudRateString, "1200") == pBaudRateString)
    {
        *pBaudRate = B1200;
        success = true;
    }    
    else if (strstr (pBaudRateString, "2400") == pBaudRateString)
    {
        *pBaudRate = B2400;
        success = true;
    }    
    else if (strstr (pBaudRateString, "4800") == pBaudRateString)
    {
        *pBaudRate = B4800;
        success = true;
    }    
    else if (strstr (pBaudRateString, "9600") == pBaudRateString)
    {
        *pBaudRate = B9600;
        success = true;
    }    
    else if (strstr (pBaudRateString, "19200") == pBaudRateString)
    {
        *pBaudRate = B19200;
        success = true;
    }    
    else if (strstr (pBaudRateString, "38400") == pBaudRateString)
    {
        *pBaudRate = B38400;
        success = true;
    }    
    else if (strstr (pBaudRateString, "57600") == pBaudRateString)
    {
        *pBaudRate = B57600;
        success = true;
    }    
    else if (strstr (pBaudRateString, "115200") == pBaudRateString)
    {
        *pBaudRate = B115200;
        success = true;
    }    
    else if (strstr (pBaudRateString, "230400") == pBaudRateString)
    {
        *pBaudRate = B230400;
        success = true;
    }    
    
    return success;
}

/*
 * Parse the parameters on the command line.
 * Prints out a "usage" string if they don't parse.
 * 
 * @return true if successful, otherwise false.
 */
static Bool parseParameters (int argc, char **argv)
{
    Bool success = true;
    int fd;
    
    if ((argc >= 1) && (argv[1] != NULL))
    {
        fd = open (argv[1], O_RDWR);
        if (fd < 0)
        {
            success = false;
            printProgress ("Cannot open terminal %s for reading/writing.\n", argv[1]);
        }
        else
        {
            gRoboOneGlobals.roboOneSettings.pTerminal = argv[1];
            close (fd);
        }
    }
    if ((argc >= 2) && (argv[2] != NULL))
    {
        success = setBaudRate (argv[2], &(gRoboOneGlobals.roboOneSettings.baudRate));
        if (!success)
        {
            printProgress ("Don't understand %s as a valid baud rate.\n", argv[2]);
        }
    }
    if ((argc >= 3) && (argv[3] != NULL))
    {
        success = false;
        printProgress ("Too many parameters (%s etc).\n", argv[3]);
    }
    
    if (!success)
    {
        printProgress ("\nusage: %s terminal baudrate\n\n", argv[0]);
        printProgress ("where the parameters (all of them optional) are:\n\n");
        printProgress ("terminal    a terminal to redirect the monitor output to.\n");
        printProgress ("baudrate    the baud rate to use.\n\n");
        printProgress ("example:\n\n");
        printProgress ("%s /dev/XBeeUSB 38400\n", argv[0]);
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
    
    success = parseParameters (argc, argv);
    
    if (success)
    {
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
                /* Now setup the Hardware server */
                success = startHardwareServer (false);
    
                if (success)
                {
                    /* Spawn a child that will become the Task Handler server. */
                    thServerPID = fork();
                    if (thServerPID == 0)
                    {
                        /* Start Task Handler server process on a given port */
                        static char *argv2[] = {TASK_HANDLER_SERVER_EXE, TASK_HANDLER_SERVER_PORT_STRING, PNULL};
                        
                        execv (TASK_HANDLER_SERVER_EXE, argv2);
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
                            /* Now setup the Task Handler server */
                            success = startTaskHandlerServer();
        
                            if (success)
                            {
                                /* Spawn a child that will become the RoboOne Battery Manager server. */
                                bmServerPID = fork();
                                if (bmServerPID == 0)
                                {
                                    /* Start Battery Manager server process */
                                    static char *argv3[] = {BATTERY_MANAGER_SERVER_EXE, BATTERY_MANAGER_SERVER_PORT_STRING, PNULL};
                                      
                                    execv (BATTERY_MANAGER_SERVER_EXE, argv3);
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
                                        /* Wait for the server to start */
                                        usleep (BATTERY_MANAGER_SERVER_START_DELAY_PI_US);
                                        /* Now setup the Battery Manager server */
                                        success = startBatteryManagerServer();
                                                
                                        if (success)
                                        {
                                            /* Spawn a child that will become the RoboOne state machine. */
                                            smServerPID = fork();
                                            if (smServerPID == 0)
                                            {
                                                /* Start State Machine server process */
                                                static char *argv4[] = {STATE_MACHINE_SERVER_EXE, STATE_MACHINE_SERVER_PORT_STRING, PNULL};
                                                  
                                                execv (STATE_MACHINE_SERVER_EXE, argv4);
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
                                                    /* Wait for the server to start */
                                                    usleep (STATE_MACHINE_SERVER_START_DELAY_PI_US);
                                                    /* Now setup the State Machine server */
                                                    success = startStateMachineServer();
            
                                                    if (success)
                                                    {
                                                        /* Start the local server that listens out for task progress indications */
                                                        success = startLocalServerThread (&localServerThread, LOCAL_SERVER_PORT);
                                                        
                                                        if (success)
                                                        {
                                                            /* Finally, display the monitor to display things and generate events */
                                                            success = runMonitor (gRoboOneGlobals.roboOneSettings.pTerminal, gRoboOneGlobals.roboOneSettings.baudRate);                                                            
                                                        }
                                                        
                                                        /* Tidy up the local server now that we're done */
                                                        stopLocalServerThread (&localServerThread);
                                                       
                                                        printProgress ("\nDone.\n");
                                                        
                                                        
                                                    }
                                                    
                                                    /* When done, tidy up the State Machine server */
                                                    stopStateMachineServer();
                                                    waitpid (smServerPID, 0, 0); /* wait for server to exit */
                                                }
                                            }                                            
                                        }
                                        
                                        /* When done, tidy up the Battery Manager server */
                                        stopBatteryManagerServer();
                                        waitpid (bmServerPID, 0, 0); /* wait for server to exit */
                                    }                            
                                }                                
                            }
                            
                            /* When done, tidy up the Task Handler server*/
                            stopTaskHandlerServer();
                            waitpid (thServerPID, 0, 0); /* wait for server to exit */
                        }
                    }                    
                }
                
                /* When done, shut down the Hardware server gracefully */
                stopHardwareServer();
                waitpid (hwServerPID, 0, 0); /* wait for server to exit */
            }
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