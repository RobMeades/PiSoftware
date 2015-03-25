/*
 * test.c
 * Test main() for the timer_server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <unistd.h> /* for fork */
#include <pthread.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <timer_server.h>
#include <timer_msg_auto.h>
#include <timer_client.h>

/*
 * MANIFEST CONSTANTS
 */
#define LOCAL_SERVER_PORT 5229

/*
 * EXTERNS
 */
extern int errno;

/*
 * GLOBALS (prefixed with g)
 */
extern Char *pgTimerMessageNames[];

/*
 * STATIC FUNCTIONS
 */

/*
 * A local server that listens out for
 * timer expiries.
 * This local server should never return,
 * it is started in its own thread and
 * cancelled by main().
 * 
 * pServerPort pointer to the port to use.
 */
void *localServer (void * serverPort)
{
    ServerReturnCode returnCode = SERVER_ERR_GENERAL_FAILURE;

    printProgress ("Test Server listening on port %d.\n", (SInt32) serverPort);
    returnCode = runMessagingServer ((SInt32) serverPort);
    
    ASSERT_ALWAYS_PARAM (returnCode);
    
    pthread_exit (&returnCode);
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
        printDebug ("!!! Couldn't create Test Server thread, err: %s. !!!\n", strerror (errno));
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
        printDebug ("!!! Couldn't cancel Test Server thread, err: %s. !!!\n", strerror (errno));
    }
    
    return success;
}

static Bool sendStartTimer (UInt32 expiryDeciSeconds, TimerId id, SInt32 sourcePort, void *pContext)
{
    TimerStartReq msg;
    
    msg.expiryDeciSeconds = expiryDeciSeconds;
    msg.id = id;
    msg.sourcePort = sourcePort;
    msg.pContext = pContext;
    
    printDebug ("Starting %d decisecond timer, id %d, sourcePort %d, pContext 0x%x.\n", expiryDeciSeconds, id, sourcePort, pContext);
    return timerServerSend (TIMER_START_REQ, &msg, sizeof (msg));
}

static Bool sendStopTimer (TimerId id, SInt32 sourcePort)
{   
    TimerStopReq msg;
    
    msg.id = id;
    msg.sourcePort = sourcePort;
    
    printDebug ("Stopping timer id %d, sourcePort %d.\n", id, sourcePort);
    return timerServerSend (TIMER_STOP_REQ, &msg, sizeof (msg));
}

static void handleTimerExpiryInd (TimerExpiryInd *pTimerExpiryInd)
{   
    printDebug ("Received timer expiry, timer id %d, pContext 0x%x.\n",
                pTimerExpiryInd->id, pTimerExpiryInd->pContext);        
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Handle a whole message received from the client.
 * 
 * pReceivedMsg   a pointer to the buffer containing the
 *                incoming message.
 * pSendMsg       a pointer to a message buffer to put
 *                the response into. Not touched if return
 *                code is a failure one.
 * 
 * @return        whatever doAction() returns.
 */
ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg *pSendMsg)
{
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);

    /* Check the type */
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_TIMER_MSGS, pReceivedMsg->msgType);
    
    /* This server never responds with anything */
    pSendMsg->msgLength = 0;

    printDebug ("Test Server received message %s, length %d.\n", pgTimerMessageNames[pReceivedMsg->msgType], pReceivedMsg->msgLength);
    printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);    
    
    if ((TimerMsgType) pReceivedMsg->msgType == TIMER_EXPIRY_IND)
    {
        /* Do the thang */
        handleTimerExpiryInd ((TimerExpiryInd *) pReceivedMsg->msgBody);
    }
    else
    {
        ASSERT_ALWAYS_PARAM (pReceivedMsg->msgType);
    }
    
    return SERVER_SUCCESS_KEEP_RUNNING;
}

/*
 * Entry point.
 */
int main (int argc, char **argv)
{
    Bool   success = false;
    pid_t  tiServerPID;
    pthread_t localServerThread;
    UInt8  id = 0;

    setDebugPrintsOnToFile ("timer_test.log");
    setProgressPrintsOn();
    copyDebugPrintsToSyslogOn();
    copyProgressPrintsToSyslogOn();
    
    /* Spawn a child that will become the Timer server. */
    tiServerPID = fork();
    if (tiServerPID == 0)
    {
        /* Start Timer server process on a given port */
        static char *argv1[] = {TIMER_SERVER_EXE, TIMER_SERVER_PORT_STRING, PNULL};
        
        execv (TIMER_SERVER_EXE, argv1);
        printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", TIMER_SERVER_EXE, strerror (errno));
    }
    else
    {
        if (tiServerPID < 0)
        {
            printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", TIMER_SERVER_EXE, strerror (errno));
        }
        else
        {   /* Parent process */
            /* Wait for the server to start */
            usleep (TIMER_SERVER_START_DELAY_PI_US);
            /* Now setup the Timer server */
            success = timerServerSend (TIMER_SERVER_START_REQ, PNULL, 0);

            if (success)
            {
                /* Start the local server that listens out for timer expiries */
                success = startLocalServerThread (&localServerThread, LOCAL_SERVER_PORT);
                
                if (success)
                {
                    UInt32 x;
                    UInt32 rememberId;
                    
                    printProgress ("TEST 1: a 1 second timer which should expire.\n");
                    sendStartTimer (10, id, LOCAL_SERVER_PORT, PNULL);
                    sleep (2);
                    printProgress ("TEST 2: a 1.5 second timer which is stopped after 1 second.\n");
                    id++;
                    sendStartTimer (15, id, LOCAL_SERVER_PORT, PNULL);
                    sleep (1);
                    sendStopTimer (id, LOCAL_SERVER_PORT);
                    printProgress ("TEST 3: a 1 second timer followed by a 0.5 second timer which should both expire.\n");
                    id++;
                    sendStartTimer (10, id, LOCAL_SERVER_PORT, PNULL);
                    id++;
                    sendStartTimer (5, id, LOCAL_SERVER_PORT, PNULL);
                    sleep (2);
                    printProgress ("TEST 4: start 10 5(ish) second timers, stop the first five and then wait for the last five to expire.\n");
                    id++;
                    rememberId = id;
                    for (x = 0; x < 10; x++)
                    {
                        sendStartTimer (50 + x, id, LOCAL_SERVER_PORT, PNULL);
                        id++;
                    }
                    for (x = 0; x < 5; x++)
                    {
                        sendStopTimer (rememberId, LOCAL_SERVER_PORT);
                        rememberId++;
                    }
                    sleep (15);
                }
                
                /* Tidy up the local server now that we're done */
                stopLocalServerThread (&localServerThread);
               
                printProgress ("\nDone.\n");                    
            }
            
            /* When done, shut down the Timer server gracefully */
            timerServerSend (TIMER_SERVER_STOP_REQ, PNULL, 0);
            waitpid (tiServerPID, 0, 0); /* wait for server to exit */
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