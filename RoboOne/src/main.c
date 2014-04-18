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
#include <ow_bus.h>
#include <dashboard.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <one_wire_server.h>
#include <one_wire_msg_auto.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_public.h>

/*
 * MANIFEST CONSTANTS
 */

#define ONE_WIRE_SERVER_EXE "./one_wire_server"
#define ONE_WIRE_SERVER_PORT_STRING "5234"
#define STATE_MACHINE_SERVER_EXE "./roboone_state_machine"
#define STATE_MACHINE_SERVER_PORT_STRING "5235"

/*
 * EXTERNS
 */

extern int errno;
extern SInt32 gOneWireServerPort;
extern SInt32 gStateMachineServerPort;

/*
 * GLOBALS (prefixed with g)
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * Send a message to stop the One Wire Server.
 * 
 * oneWireServerPort the port the server is listening on.
 * 
 * @return           true if successful, otherwise false.
 */
static Bool stopOneWireServer (SInt32 oneWireServerPort)
{
    ClientReturnCode returnCode;
    Bool success = true;
    Msg *pSendMsg;
    Msg *pReceivedMsg;
    
    ASSERT_PARAM (oneWireServerPort >= 0, oneWireServerPort);
    
    pSendMsg = malloc (sizeof (Msg));
    
    if (pSendMsg != PNULL)
    {
        /* Setup an exit message, only has the type in it */
        pSendMsg->msgLength = 1;
        pSendMsg->msgType = ONE_WIRE_SERVER_EXIT;
        
        pReceivedMsg = malloc (sizeof (Msg));
        if (pReceivedMsg != PNULL)
        {
            returnCode = runMessagingClient (oneWireServerPort, pSendMsg, pReceivedMsg);
            
            if ((returnCode != CLIENT_SUCCESS) || (pReceivedMsg->msgType != ONE_WIRE_SERVER_EXIT))
            {
                success = false;    
            }

            free (pReceivedMsg);
        }
        else
        {
            success = false;
        }        
        
        free (pSendMsg);
    }
    else
    {
        success = false;
    }

    return success;
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
        printf ("receivedMsgType %d, pStateMachineServerStartCnf->success %d", receivedMsgType, pStateMachineServerStartCnf->success);
        
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
    Bool  success = false;
    pid_t serverPID;
    pid_t stateMachinePID;
    
    /* setDebugPrintsOn(); */
    setProgressPrintsOn();

    /* Setup the globals for everyone to use */
    gOneWireServerPort = atoi (ONE_WIRE_SERVER_PORT_STRING);
    gStateMachineServerPort = atoi (STATE_MACHINE_SERVER_PORT_STRING);
    
    /* Spawn a child that will become the One Wire server. */
    serverPID = fork();
    if (serverPID == 0)
    {
        /* Start OneWire server process on a given port */
        static char *argv1[] = {ONE_WIRE_SERVER_EXE, ONE_WIRE_SERVER_PORT_STRING, PNULL};
        
        execv (ONE_WIRE_SERVER_EXE, argv1);
        printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", ONE_WIRE_SERVER_EXE, strerror (errno));
    }
    else
    {
        if (serverPID < 0)
        {
            printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", ONE_WIRE_SERVER_EXE, strerror (errno));
        }
        else
        {   /* Parent process */
            /* Setup what's necessary for OneWire bus stuff */
            usleep (SERVER_START_DELAY_PI_US); /* Wait for the server to start */
            success = startOneWireBus();
              
            /* Find and setup the devices on the OneWire bus */
            if (success)
            {
                success = setupDevices();                  
                if (success)
                {        
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
                }
                else
                {
                    /* If the setup fails, print out what devices we can find */
                    findAllDevices();
                }
                
                if (!success)
                {
                    printProgress ("\nFailed!\n");
                }
                    
                /* Shut the OneWire stuff down gracefully */
                stopOneWireBus ();
                stopOneWireServer (gOneWireServerPort);
                
                waitpid (serverPID, 0, 0); /* wait for server to exit */
            }
        }
    }
    
    if (!success)
    {
        exit (-1);
    }
    
    return success;
}