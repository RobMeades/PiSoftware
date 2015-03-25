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
#define MAX_NUM_TIMER_EXPIRY_INDS_REMEMBERED 10

/*
 * EXTERNS
 */
extern int errno;

/*
 * GLOBALS (prefixed with g)
 */
extern Char *pgTimerMessageNames[];
/* An array to store the timer expiries in */
TimerExpiryInd gTimerExpiryInds[MAX_NUM_TIMER_EXPIRY_INDS_REMEMBERED];
/* The number of entries in the array */
UInt8_t gNumTimerExpiryInds = 0;

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
    UInt32_t x;
    
    printDebug ("Received timer expiry, timer id %d, pContext 0x%x.\n",
                pTimerExpiryInd->id, pTimerExpiryInd->pContext);
                
    /* Remember the data so that we can check it later */
    if (gNumTimerExpiryInds => MAX_NUM_TIMER_EXPIRY_INDS_REMEMBERED)
    {
        for (x = 0; x < MAX_NUM_TIMER_EXPIRY_INDS_REMEMBERED - 1; x++)
        {
            memcpy (&(gTimerExpiryInds[x]), &(gTimerExpiryInds[x + 1]), sizeof (gTimerExpiryInds[x]));
        }
    }
    
    memcpy (&(gTimerExpiryInds[gNumTimerExpiryInds]), pTimerExpiryInd, sizeof (gTimerExpiryInds[gNumTimerExpiryInds]));
    
    if (gNumTimerExpiryInds < MAX_NUM_TIMER_EXPIRY_INDS_REMEMBERED)
    {
        gNumTimerExpiryInds++;
    }
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
    char buf1 = "hello world";
    char buf2 = "goodbye cruel world";
    char buf3 = "hi world";

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
                    
                    printProgress ("------------------------------------------------------------------\n");
                    printProgress ("STARTING TIMER TESTS.\n");
                    printProgress ("- TEST 0: start a 25 second timer, with a context, that should over-arch\n          these tests.\n");
                    sendStartTimer (250, id, LOCAL_SERVER_PORT, buf2);
                    
                    printProgress ("- TEST 1: a 1 second timer which should expire.\n");
                    id++;
                    sendStartTimer (10, id, LOCAL_SERVER_PORT, PNULL);
                    sleep (2);
                    ASSERT_PARAM (gNumTimerExpiryInds == 1, gNumTimerExpiryInds);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 1].id == id, gTimerExpiryInds[gNumTimerExpiryInds - 1].id);

                    printProgress ("- TEST 2: a 1.5 second timer which is stopped after 1 second.\n");
                    id++;
                    sendStartTimer (15, id, LOCAL_SERVER_PORT, PNULL);
                    sleep (1);
                    sendStopTimer (id, LOCAL_SERVER_PORT);
                    ASSERT_PARAM (gNumTimerExpiryInds == 1, gNumTimerExpiryInds);
                    
                    printProgress ("- TEST 3: a 1 second timer followed by a 0.5 second timer which should both\n          expire.\n");
                    id++;
                    sendStartTimer (10, id, LOCAL_SERVER_PORT, PNULL);
                    id++;
                    sendStartTimer (5, id, LOCAL_SERVER_PORT, PNULL);
                    sleep (2);
                    ASSERT_PARAM (gNumTimerExpiryInds == 3, gNumTimerExpiryInds);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 2].id == id, gTimerExpiryInds[gNumTimerExpiryInds - 2].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 1].id == id - 1, gTimerExpiryInds[gNumTimerExpiryInds - 1].id);

                    printProgress ("- TEST 4: start 10 5(ish) second timers, stop the first five and then wait\n          for the last five to expire.\n");
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
                    sleep (8);
                    ASSERT_PARAM (gNumTimerExpiryInds == 8, gNumTimerExpiryInds);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 5].id == id - 4, gTimerExpiryInds[gNumTimerExpiryInds - 5].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 4].id == id - 3, gTimerExpiryInds[gNumTimerExpiryInds - 4].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 3].id == id - 2, gTimerExpiryInds[gNumTimerExpiryInds - 3].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 2].id == id - 1, gTimerExpiryInds[gNumTimerExpiryInds - 2].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 1].id == id, gTimerExpiryInds[gNumTimerExpiryInds - 1].id);

                    printProgress ("- TEST 5: start 5 timers of differing, overlapping, expiries and check that\n          they expire in the correct order.\n");
                    gNumTimerExpiryInds = 0;
                    id++;
                    sendStartTimer (21, id, LOCAL_SERVER_PORT, PNULL);
                    id++;
                    sendStartTimer (10, id, LOCAL_SERVER_PORT, PNULL);
                    id++;
                    sendStartTimer (25, id, LOCAL_SERVER_PORT, PNULL);
                    id++;
                    sendStartTimer (11, id, LOCAL_SERVER_PORT, PNULL);
                    id++;
                    sendStartTimer (5, id, LOCAL_SERVER_PORT, PNULL);
                    sleep (5);
                    ASSERT_PARAM (gNumTimerExpiryInds == 5, gNumTimerExpiryInds);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 5].id == id, gTimerExpiryInds[gNumTimerExpiryInds - 5].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 4].id == id - 3, gTimerExpiryInds[gNumTimerExpiryInds - 4].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 3].id == id - 1, gTimerExpiryInds[gNumTimerExpiryInds - 3].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 2].id == id - 4, gTimerExpiryInds[gNumTimerExpiryInds - 2].id);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 1].id == id - 2, gTimerExpiryInds[gNumTimerExpiryInds - 1].id);

                    printProgress ("- TEST 6: start a 0 second timer, it should expire straight away\n          (100 ms permitted).\n");
                    id++;
                    sendStartTimer (0, id, LOCAL_SERVER_PORT, PNULL);
                    usleep (100000);
                    ASSERT_PARAM (gNumTimerExpiryInds == 6, gNumTimerExpiryInds);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 1].id == id, gTimerExpiryInds[gNumTimerExpiryInds - 1].id);

                    printProgress ("- TEST 7: start 3 timers, with contexts, then stop the middle one and check\n          that the other two expire in the right order.\n");
                    id++;
                    sendStartTimer (30, id, LOCAL_SERVER_PORT, buf1);
                    id++;
                    sendStartTimer (20, id, LOCAL_SERVER_PORT, buf2);
                    id++;
                    sendStartTimer (10, id, LOCAL_SERVER_PORT, buf3);
                    sendStopTimer (id - 1, LOCAL_SERVER_PORT);
                    sleep (4);
                    ASSERT_PARAM (gNumTimerExpiryInds == 9, gNumTimerExpiryInds);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds].id == id, gTimerExpiryInds[gNumTimerExpiryInds].id);
                    ASSERT_PARAM (strcmp (gTimerExpiryInds[gNumTimerExpiryInds].pContext, buf3) == 0, strlen (gTimerExpiryInds[gNumTimerExpiryInds].pContext));
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds - 1].id == id - 2, gTimerExpiryInds[gNumTimerExpiryInds - 1].id);
                    ASSERT_PARAM (strcmp (gTimerExpiryInds[gNumTimerExpiryInds - 1].pContext, buf1) == 0, strlen (gTimerExpiryInds[gNumTimerExpiryInds].pContext));

                    printProgress ("- TEST 8: wait for the over-arching timer to expire.\n");
                    sleep (5);
                    ASSERT_PARAM (gNumTimerExpiryInds == 10, gNumTimerExpiryInds);
                    ASSERT_PARAM (gTimerExpiryInds[gNumTimerExpiryInds].id == 0, gTimerExpiryInds[gNumTimerExpiryInds].id);
                    ASSERT_PARAM (strcmp (gTimerExpiryInds[gNumTimerExpiryInds].pContext, buf2) == 0, strlen (gTimerExpiryInds[gNumTimerExpiryInds].pContext));

                    printProgress ("TESTING COMPLETED: if there are no asserts above then it's a pass.\n");
                    printProgress ("------------------------------------------------------------------\n");
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
