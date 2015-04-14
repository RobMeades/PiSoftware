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
#define MAX_NUM_TIMER_IDS_REMEMBERED 20

/*
 * TYPES
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

typedef enum TimerTestMsgTypeTag
{
    TIMER_TEST_EXPIRY_IND,
    MAX_NUM_TIMER_TEST_MSGS
} TimerTestMsgType;

/* A timer expiry message */
typedef struct TimerTestExpiryIndMsgTag
{
    TimerId id;
} TimerTestExpiryIndMsg;

#pragma pack(pop) /* End of packing */

/*
 * EXTERNS
 */
extern int errno;

/*
 * GLOBALS (prefixed with g)
 */
extern Char *pgTimerMessageNames[];
/* An array to store the timer expiry IDs in */
TimerId gId[MAX_NUM_TIMER_IDS_REMEMBERED];
/* The number of entries in the array */
UInt8 gNumIds = 0;

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

static void handleTimerTestExpiryInd (TimerTestExpiryIndMsg *pTimerTestExpiryIndMsg)
{
    UInt32 x;
    
    printDebug ("Received timer expiry, timer id %d.\n", pTimerTestExpiryIndMsg->id);
                
    /* Remember the data so that we can check it later */
    if (gNumIds >= MAX_NUM_TIMER_IDS_REMEMBERED)
    {
        for (x = 0; x < (MAX_NUM_TIMER_IDS_REMEMBERED - 1); x++)
        {
            gId[x] = gId[x + 1];
        }
        gNumIds = MAX_NUM_TIMER_IDS_REMEMBERED - 1;
    }
    
    gId[gNumIds] = pTimerTestExpiryIndMsg->id;
    
    if (gNumIds < MAX_NUM_TIMER_IDS_REMEMBERED)
    {
        gNumIds++;
    }
    
    printDebug ("[%d timers expired].\n", gNumIds);
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

    printDebug ("Test Server received message 0x%x, length %d.\n", pReceivedMsg->msgType, pReceivedMsg->msgLength);
    printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);    
    
    if ((TimerTestMsgType) pReceivedMsg->msgType == TIMER_TEST_EXPIRY_IND)
    {
        /* Do the thang */
        handleTimerTestExpiryInd ((TimerTestExpiryIndMsg *) pReceivedMsg->msgBody);
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
    ShortMsg msg;
    TimerTestExpiryIndMsg timerTestExpiryIndMsg;

    /*setDebugPrintsOnToFile ("timer_test.log");*/
    setProgressPrintsOn();
    /* setDebugPrintsOnToSyslog();*/ /* Don't switch debug on in here unless you have to, the load has a significant effect */
    
    
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
                    
                    timerTestExpiryIndMsg.id = 0;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    

                    /* Note: the timers in here are fairly generous as it is expected that lots
                     * of debug will be switched on and so time won't be as, err, timely as it might
                     * otherwise be */
                    printProgress ("------------------------------------------------------------------\n");
                    printProgress ("STARTING TIMER TESTS.\n");
                    printProgress ("- TEST 0: start a 40 second timer that should over-arch\n          these tests.\n");
                    sendStartTimer (400, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    
                    printProgress ("- TEST 1: a 1 second timer which should expire.\n");
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (10, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    sleep (2);
                    ASSERT_PARAM2 (gNumIds == 1, gNumIds, 1);
                    ASSERT_PARAM3 (gId[gNumIds - 1] == timerTestExpiryIndMsg.id,
                                   gId[gNumIds - 1],
                                   timerTestExpiryIndMsg.id,
                                   gNumIds);

                    printProgress ("- TEST 2: a 1.5 second timer which is stopped after 1 second.\n");
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (15, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    sleep (1);
                    sendStopTimer (timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT);
                    ASSERT_PARAM2 (gNumIds == 1, gNumIds, 1);
                    
                    printProgress ("- TEST 3: a 1 second timer followed by a 0.5 second timer which should both\n          expire.\n");
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (10, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (5, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    sleep (2);
                    ASSERT_PARAM2 (gNumIds == 3, gNumIds, 3);
                    ASSERT_PARAM3 (gId[gNumIds - 2] == timerTestExpiryIndMsg.id,
                                   gId[gNumIds - 2],
                                   timerTestExpiryIndMsg.id,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 1] == timerTestExpiryIndMsg.id - 1,
                                   gId[gNumIds - 1],
                                   timerTestExpiryIndMsg.id - 1,
                                   gNumIds);

                    printProgress ("- TEST 4: start 10 7(ish) second timers, stop the first five and then wait\n          for the last five to expire.\n");
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    rememberId = timerTestExpiryIndMsg.id;
                    for (x = 0; x < 10; x++)
                    {
                        sendStartTimer (70 + x, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                        timerTestExpiryIndMsg.id++;
                        createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    }
                    sleep (3);
                    for (x = 0; x < 5; x++)
                    {
                        sendStopTimer (rememberId, LOCAL_SERVER_PORT);
                        rememberId++;
                    }
                    sleep (10);
                    ASSERT_PARAM2 (gNumIds == 8, gNumIds, 8);
                    ASSERT_PARAM3 (gId[gNumIds - 5] == timerTestExpiryIndMsg.id - 5,
                                   gId[gNumIds - 5],
                                   timerTestExpiryIndMsg.id - 5,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 4] == timerTestExpiryIndMsg.id - 4,
                                   gId[gNumIds - 4],
                                   timerTestExpiryIndMsg.id - 4,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 3] == timerTestExpiryIndMsg.id - 3,
                                   gId[gNumIds - 3],
                                   timerTestExpiryIndMsg.id - 3,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 2] == timerTestExpiryIndMsg.id - 2,
                                   gId[gNumIds - 2],
                                   timerTestExpiryIndMsg.id - 2,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 1] == timerTestExpiryIndMsg.id - 1,
                                   gId[gNumIds - 1],
                                   timerTestExpiryIndMsg.id - 1,
                                   gNumIds);

                    printProgress ("- TEST 5: start 5 timers of differing, overlapping, expiries and check that\n          they expire in the correct order.\n");
                    gNumIds = 0; /* Reset the count to make it easier to see where we are */
                    sendStartTimer (80, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (20, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (60, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (40, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (5, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    sleep (10);
                    ASSERT_PARAM2 (gNumIds == 5, gNumIds, 5);
                    ASSERT_PARAM3 (gId[gNumIds - 5] == timerTestExpiryIndMsg.id,
                                   gId[gNumIds - 5],
                                   timerTestExpiryIndMsg.id,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 4] == timerTestExpiryIndMsg.id - 3,
                                   gId[gNumIds - 4],
                                   timerTestExpiryIndMsg.id - 3,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 3] == timerTestExpiryIndMsg.id - 1,
                                   gId[gNumIds - 3],
                                   timerTestExpiryIndMsg.id - 1,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 2] == timerTestExpiryIndMsg.id - 2,
                                   gId[gNumIds - 2],
                                   timerTestExpiryIndMsg.id - 2,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 1] == timerTestExpiryIndMsg.id - 4,
                                   gId[gNumIds - 1],
                                   timerTestExpiryIndMsg.id - 4,
                                   gNumIds);

                    printProgress ("- TEST 6: start a 0 second timer, it should expire straight away\n          (2 s permitted).\n");
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (0, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    sleep (2);
                    ASSERT_PARAM2 (gNumIds == 6, gNumIds, 6);
                    ASSERT_PARAM3 (gId[gNumIds - 1] == timerTestExpiryIndMsg.id,
                                   gId[gNumIds - 1],
                                   timerTestExpiryIndMsg.id,
                                   gNumIds);

                    printProgress ("- TEST 7: start 3 timers then stop the middle one and check\n          that the other two expire in the right order.\n");
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (30, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (20, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    timerTestExpiryIndMsg.id++;
                    createTimerExpiryMsg (&msg, TIMER_TEST_EXPIRY_IND, &timerTestExpiryIndMsg, sizeof (timerTestExpiryIndMsg));                    
                    sendStartTimer (10, timerTestExpiryIndMsg.id, LOCAL_SERVER_PORT, &msg);
                    sendStopTimer (timerTestExpiryIndMsg.id - 1, LOCAL_SERVER_PORT);
                    sleep (4);
                    ASSERT_PARAM2 (gNumIds == 8, gNumIds, 8);
                    ASSERT_PARAM3 (gId[gNumIds - 1] == timerTestExpiryIndMsg.id - 2,
                                   gId[gNumIds - 1],
                                   timerTestExpiryIndMsg.id - 2,
                                   gNumIds);
                    ASSERT_PARAM3 (gId[gNumIds - 2] == timerTestExpiryIndMsg.id,
                                   gId[gNumIds - 2],
                                   timerTestExpiryIndMsg.id,
                                   gNumIds);

                    printProgress ("- TEST 8: wait for the over-arching timer to expire.\n");
                    sleep (12);
                    ASSERT_PARAM2 (gNumIds == 9, gNumIds, 9);
                    ASSERT_PARAM3 (gId[gNumIds - 1] == 0,
                                   gId[gNumIds - 1],
                                   0,
                                   gNumIds);

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
