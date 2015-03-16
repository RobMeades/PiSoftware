/*
 * timer_server.c
 * Builds a timer function into a sockets server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <timer_server.h>
#include <timer_msg_auto.h>
#include <timer_client.h>

/*
 * MANIFEST CONSTANTS
 */

/* The number of timers that can be simultaneously active */
#define MAX_NUM_TIMERS 100
/* Timer tick frequency (100 ms) */
#define TIMER_INTERVAL_NANOSECONDS 100000000L

/*
 * TYPES
 */

/* A type to hold a timer */
typedef struct TimerTag
{
    UInt32 expiryTimeDeciSeconds;
    TimerId id;
    SInt32 sourcePort;
    void *pContext;
} Timer;

/* A linked list entry holding a timer */
typedef struct TimerEntryTag
{
    bool inUse;
    Timer timer;
    struct TimerEntryTag * pPrevEntry;
    struct TimerEntryTag * pNextEntry;
} TimerEntry;

/*
 * EXTERNS
 */
extern Char *pgTimerMessageNames[];

/*
 * GLOBALS - prefixed with g
 */

/* Head of free timer linked list */
static TimerEntry gFreeTimerListHeadUnused;
/* Head of used timer linked list */
static TimerEntry gUsedTimerListHead;
/* Signal action structure for timer tick */
static struct sigaction gSa;
/* Signal event, timer Id and frequency of expiry */
static struct itimerspec gIts;
static struct sigevent gSev;
static timer_t gTimerId;
/* Time counter in tenths of a second */
static UInt32 gTimerTickDeciSeconds = 0;
/* Mutex to protect linked list manipulation */
pthread_mutex_t lockLinkedLists;

/*
 * STATIC FUNCTIONS
 */

/*
 * Get the process time in nanosecond resolution.
 * 
 * @return  the process time in nanoseconds.
 */
UInt32 getProcessTimeNanoSeconds (void)
{
    struct timespec time;

    if (clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &time) != 0)
    {
        /* If the call fails, set the time returned to zero */
        time.tv_nsec = 0;
    }
    
    return (UInt32) time.tv_nsec;
}

/*
 * Send a message to a port.
 * 
 * port                  the port number to send to.
 * msgType               the message type to send.
 * pSendMsgBody          pointer to the body of the
 *                       message to send. May be PNULL.
 * sendMsgBodyLength     the length of the data that
 *                       pSendMsg points to.
 * 
 * @return           true if the message send is
 *                   successful and the response
 *                   message indicates success,
 *                   otherwise false.
 */
static Bool timerServerSend (SInt32 port, TimerMsgType msgType, void *pSendMsgBody, UInt16 sendMsgBodyLength)
{
    ClientReturnCode returnCode;
    Bool success = false;
    Msg *pSendMsg;

    ASSERT_PARAM (msgType < MAX_NUM_TIMER_MSGS, msgType);
    ASSERT_PARAM (sendMsgBodyLength <= MAX_MSG_BODY_LENGTH, sendMsgBodyLength);

    pSendMsg = malloc (sizeof (*pSendMsg));
    
    if (pSendMsg != PNULL)
    {
        /* Put in the bit before the body */
        pSendMsg->msgLength = 0;
        pSendMsg->msgType = msgType;
        pSendMsg->msgLength += sizeof (pSendMsg->msgType);
                    
        /* Add the body to send */
        if (pSendMsgBody != PNULL)
        {
            memcpy (&pSendMsg->msgBody[0], pSendMsgBody, sendMsgBodyLength);
        }
        pSendMsg->msgLength += sendMsgBodyLength;
            
        printDebug ("Timer Server: sending to port %d, message %s, length %d, hex dump:\n", port, pgTimerMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
        printHexDump (pSendMsg, pSendMsg->msgLength + 1);
        returnCode = runMessagingClient (port, PNULL, pSendMsg, PNULL);
                    
        printDebug ("Timer Server: message system returnCode: %d\n", returnCode);
        if (returnCode == CLIENT_SUCCESS)
        { 
            success = true;
        }
        free (pSendMsg);
    }

    return success;
}

/*
 * Send a timer expiry message.
 * 
 * pTimer   the timer related to the
 *          expiry message to send.
 * 
 * @return  true if the message send is
 *          is successful, otherwise false.
 */
static Bool sendTimerExpiryIndMsg (Timer *pTimer)
{
    TimerExpiryInd msg;
    
    printDebug ("Sending TimerExpiryInd message to port %d (timer id %d, pContext 0x%lx).\n", pTimer->sourcePort, pTimer->id, pTimer->pContext);
    msg.id = pTimer->id;
    msg.pContext = pTimer->pContext;
    
    return timerServerSend (pTimer->sourcePort, TIMER_EXPIRY_IND, &msg, sizeof (msg));
}

/*
 * Free a timer.
 *
 * IMPORTANT: the lockLinkedLists mutex MUST be held
 * by the calling function!!!  It is not grabbed
 * here as this function is only called from places
 * where it is already held and recursive mutexes appear
 * to be only unreliably present in Linux.
 * 
 * pTimer  a pointer to the timer.
 */
static void freeTimerUnprotected (Timer *pTimer)
{
    UInt32 x = 0;
    TimerEntry * pEntry;

    ASSERT_PARAM (pTimer != PNULL, (unsigned long) pTimer);

    printDebug ("freeTimer: freeing the timer at 0x%lx...\n", pTimer);

    /* Find the entry in the list */
    pEntry = &gUsedTimerListHead;
    for (x = 0; (pEntry != PNULL) && (&(pEntry->timer) != pTimer) && (x < MAX_NUM_TIMERS); x++)
    {
        pEntry = pEntry->pNextEntry;
    }

    if ((pEntry != PNULL) && (&(pEntry->timer) == pTimer))
    {
        TimerEntry * pWantedEntry = pEntry;

        printDebug ("freeTimer: found the timer.\n");
        /* Found it, mark it as not in use and, if it is a malloc()ed
         * entry, unlink it from the current list and move it to the
         * free list */
        pWantedEntry->inUse = false;
        memset (&(pWantedEntry->timer), 0, sizeof (pWantedEntry->timer));
        if (pWantedEntry != &gUsedTimerListHead)
        {
            printDebug ("freeTimer: moving malloc()ed entry to free list.\n");
            if (pWantedEntry->pPrevEntry != PNULL)
            {
                pWantedEntry->pPrevEntry->pNextEntry = pWantedEntry->pNextEntry;
                pWantedEntry->pPrevEntry = PNULL;
            }
            if (pWantedEntry->pNextEntry != PNULL)
            {
                pWantedEntry->pNextEntry->pPrevEntry = pWantedEntry->pPrevEntry;
                pWantedEntry->pNextEntry = PNULL;
            }

            /* Find the end of the free list */
            pEntry = &gFreeTimerListHeadUnused;
            for (x = 0; (pEntry->pNextEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
            {
                pEntry = pEntry->pNextEntry;
            }

            /* Link it there */
            if (pEntry->pNextEntry == PNULL)
            {
                pEntry->pNextEntry = pWantedEntry;
                pWantedEntry->pPrevEntry = pEntry;
            }
            else
            {
                ASSERT_ALWAYS_PARAM (x);   
            }
        }
        else
        {
            printDebug ("freeTimer: just marking head entry as unused.\n");
        }
    }
    else
    {
        ASSERT_ALWAYS_PARAM ((UInt32) pTimer);
    }
}

/*
 * Handle a periodic timer callback from the OS.
 * Increment the deci-second counter and see if
 * any timers have expired.
 */
static void tickHandler (int sig, siginfo_t *si, void *uc)
{
    UInt32 x = 0;
    TimerEntry * pEntry;
    UInt32 tickProcessingStartNanoSeconds;

    tickProcessingStartNanoSeconds = getProcessTimeNanoSeconds();

    gTimerTickDeciSeconds++;
    
    /* Wrap is several years so don't need to deal with it */

    if (pthread_mutex_trylock (&lockLinkedLists) == 0)
    {
        pEntry = &gUsedTimerListHead;
        for (x = 0; (pEntry != PNULL) && (pEntry->inUse) && (x < (MAX_NUM_TIMERS)); x++)
        {
            if (pEntry->timer.expiryTimeDeciSeconds <= gTimerTickDeciSeconds)
            {
                Timer * pTimer = &(pEntry->timer);
    
                printDebug ("tickHandler: timer from port %d (id %d) has expired at tick %d (value was %d).\n", pEntry->timer.sourcePort, pEntry->timer.id, gTimerTickDeciSeconds, pEntry->timer.expiryTimeDeciSeconds);
                
                /* Timer has expired, send a message back */
                sendTimerExpiryIndMsg (pTimer);
                
                /* Move to the next entry and then free this timer entry */
                pEntry = pEntry->pNextEntry;
                freeTimerUnprotected (pTimer);
            }
            else
            {
                pEntry = pEntry->pNextEntry;
            }
        }
        
        pthread_mutex_unlock (&lockLinkedLists);
        printDebug ("tickHandler: processing a tick took %d nanoseconds.\n", getProcessTimeNanoSeconds() - tickProcessingStartNanoSeconds);
    }
    else
    {
        printDebug ("tickHandler: linked lists locked, skipping tick processing at tick %d.\n", gTimerTickDeciSeconds);
    }

    signal (sig, SIG_IGN);
}

/*
 * Sort the list of user timers so that the ones
 * going to expires soonest are at the start using
 * a simple bubble sort.
 *
 * IMPORTANT: the lockLinkedLists mutex MUST be held
 * by the calling function!!!  It is not grabbed
 * here as this function is only called from places
 * where it is already held and recursive mutexes appear
 * to be only unreliably present in Linux.
 */
static void sortUsedListUnprotected (void)
{
    UInt32 x = 0;
    TimerEntry * pEntry;
    UInt32 sortingStartNanoSeconds;

    sortingStartNanoSeconds = getProcessTimeNanoSeconds();

    pEntry = &gUsedTimerListHead;
    for (x = 0; (pEntry != PNULL) && (pEntry->inUse) && (x < (MAX_NUM_TIMERS * MAX_NUM_TIMERS)); x++)
    {
        if ((pEntry->pNextEntry != PNULL) && (pEntry->timer.expiryTimeDeciSeconds > pEntry->pNextEntry->timer.expiryTimeDeciSeconds))
        {
            TimerEntry * pThisEntry = pEntry;
            TimerEntry * pNextEntry = pEntry->pNextEntry;

            printDebug ("sortUsedList: swapping entries %d with %d.\n", pEntry->timer.expiryTimeDeciSeconds, pEntry->pNextEntry->timer.expiryTimeDeciSeconds);
            /* If this entry has a later expiry time than the next one, swap them */
            pThisEntry->pNextEntry = pNextEntry->pNextEntry;
            pNextEntry->pPrevEntry = pThisEntry->pPrevEntry;
            pThisEntry->pPrevEntry = pNextEntry;
            pNextEntry->pNextEntry = pThisEntry;
            
            /* Restart the sort from the beginning */
            pEntry = &gUsedTimerListHead;
        }
        else
        {
            pEntry = pEntry->pNextEntry;
        }
    }
    
    if (x == MAX_NUM_TIMERS * MAX_NUM_TIMERS)
    {
        printDebug ("sortUsedList: WARNING, sorting the timer list hit the buffers (%d iterations, %d nanoseconds).\n", x, getProcessTimeNanoSeconds() - sortingStartNanoSeconds);
    }
    else
    {
        printDebug ("sortUsedList: sorting the timer list took %d iterations, %d nanoseconds.\n", x, getProcessTimeNanoSeconds() - sortingStartNanoSeconds);        
    }
}

/*
 * Allocate a timer and re-sort the timer list.
 * 
 * @return  a pointer to the timer or PNULL if
 *          unable to allocate one.
 */
static Timer * allocTimer (void)
{
    Timer * pAlloc = PNULL;
    UInt32 x = 0;
    TimerEntry * pEntry;

    pthread_mutex_lock (&lockLinkedLists);

    /* If there is already an unused entry in the list, just return it */
    pEntry = &gUsedTimerListHead;
    for (x = 0; (pEntry != PNULL) && (pEntry->inUse) && (x < MAX_NUM_TIMERS); x++)
    {
        pEntry = pEntry->pNextEntry;
    }

    if ((pEntry != PNULL) && (!pEntry->inUse))
    {
        printDebug ("allocTimer: found existing unused entry in list.\n");
        /* Good, use this one */
        memset (&(pEntry->timer), 0, sizeof (pEntry->timer));
        pEntry->inUse = true;
        pAlloc = &(pEntry->timer);
    }
    else
    {
        printDebug ("allocTimer: finding malloc()ed entry in the free list...\n");
        /* Find the malloc()ed entry at the end of the free list */
        pEntry = &gFreeTimerListHeadUnused;
        for (x = 0; (pEntry->pNextEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
        {
            pEntry = pEntry->pNextEntry;
        }

        /* If there is one, move it to the used list */
        if ((pEntry != PNULL) && (pEntry->pNextEntry == PNULL) && pEntry != &gFreeTimerListHeadUnused)
        {
            TimerEntry * pWantedEntry = pEntry;

            /* Unlink it from the end of the free list */
            if (pWantedEntry->pPrevEntry != PNULL)
            {
                pWantedEntry->pPrevEntry->pNextEntry = pWantedEntry->pNextEntry;
                pWantedEntry->pPrevEntry = PNULL;
            }

            /* Attach it to the end of the used list */
            pEntry = &gUsedTimerListHead;
            for (x = 0; (pEntry->pNextEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
            {
                pEntry = pEntry->pNextEntry;
            }

            if (pEntry->pNextEntry == PNULL)
            {
                pEntry->pNextEntry = pWantedEntry;
                pWantedEntry->pPrevEntry = pEntry;
                pWantedEntry->inUse = true;
                pAlloc = &(pWantedEntry->timer);
                
                /* Now sort the list */
                sortUsedListUnprotected();
            }
            else
            {
                ASSERT_ALWAYS_PARAM (x);   
            }
        }
        else
        {
            ASSERT_ALWAYS_PARAM (x);   
        }
    }
    
    pthread_mutex_unlock (&lockLinkedLists);

    return pAlloc;
}

/*
 * Free unused timers
 *
 * IMPORTANT: the lockLinkedLists mutex MUST be held
 * by the calling function!!!  It is not grabbed
 * here as this function is only called from places
 * where it is already held and recursive mutexes appear
 * to be only unreliably present in Linux.
 */
static void freeUnusedTimersUnprotected (void)
{
    UInt32 x = 0;
    TimerEntry * pEntry = PNULL;

    printDebug ("freeUnusedTimers: freeing unused timers.\n");
    
    /* Go to the end of the list */
    pEntry = &gUsedTimerListHead;
    for (x = 0; (pEntry->pNextEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        pEntry = pEntry->pNextEntry;
    }

    if ((pEntry != PNULL) && (pEntry->pNextEntry == PNULL))
    {
        /* Now work backwards up the list freeing unused things until we get
         * to the head */
        for (x = 0; (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
        {
            TimerEntry * pNextOne = pEntry->pPrevEntry;
            /* Check that this entry is unused and not the first static entry then,
             * if it is, free it */
            if ((!(pEntry->inUse)) && (pEntry != &gUsedTimerListHead))
            {
                freeTimerUnprotected (&(pEntry->timer));
                printDebug ("freeUnusedTimers: freeing an entry.\n");
            }
            pEntry = pNextOne;
        }
    }
    else
    {
        ASSERT_ALWAYS_PARAM (x);   
    }
}

/*
 * Empty the used timer list
 */
static void freeAllTimers (void)
{
    UInt32 x;
    TimerEntry * pEntry = PNULL;

    pthread_mutex_lock (&lockLinkedLists);

    printDebug ("freeAllTimers: freeing all timers.\n");
    
    pEntry = &gUsedTimerListHead;
    for (x = 0; (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        pEntry->inUse = false;
        pEntry = pEntry->pNextEntry;
    }

    freeUnusedTimersUnprotected();

    pthread_mutex_unlock (&lockLinkedLists);
}

/*
 * Handle a message that will cause us to start.
 */
static void actionTimerServerStart (void)
{
    UInt32 x;
    TimerEntry * pPrevEntry = &gFreeTimerListHeadUnused;
    TimerEntry ** ppEntry = &(gFreeTimerListHeadUnused.pNextEntry);

    /* Create the mutex */
    if (pthread_mutex_init (&lockLinkedLists, NULL) != 0)
    {
        ASSERT_ALWAYS_STRING ("actionTimerServerStart: failed pthread_mutex_init().");
    }
    
    memset (&gFreeTimerListHeadUnused, 0, sizeof (gFreeTimerListHeadUnused));
    memset (&gUsedTimerListHead, 0, sizeof (gUsedTimerListHead));

    /* Create a malloc()ed free list attached to the (unused) static
     * head of the free list */
    for (x = 1; x < MAX_NUM_TIMERS; x++)  /* from 1 as it's from head.pNextEntry onwards */
    {
        *ppEntry = (TimerEntry *) malloc (sizeof (TimerEntry));
        if (*ppEntry != PNULL)
        {
            memset (*ppEntry, 0, sizeof (**ppEntry));
            /* Link it in */
            (*ppEntry)->pPrevEntry = pPrevEntry;
            (*ppEntry)->pNextEntry = PNULL;
            (*ppEntry)->inUse = false;
            if (pPrevEntry != PNULL)
            {
                pPrevEntry->pNextEntry = *ppEntry;
            }
            pPrevEntry = *ppEntry;

            /* Next */
            ppEntry = &((*ppEntry)->pNextEntry);
        }
        else
        {
            ASSERT_ALWAYS_STRING ("actionTimerServerStart: failed malloc().");
        }
    }
    
    /* The following code is taken from the example at
     * http://man7.org/linux/man-pages/man2/timer_create.2.html */
    
    /* Establish the handler for a tick from the OS */
    gSa.sa_flags = SA_SIGINFO;
    gSa.sa_sigaction = tickHandler;
    sigemptyset (&gSa.sa_mask);
    if (sigaction (SIGRTMIN, &gSa, PNULL) != 0)
    {
        ASSERT_ALWAYS_STRING ("actionTimerServerStart: failed sigaction().");
    }
    
    /* Create the tick event */
    gSev.sigev_notify = SIGEV_SIGNAL;
    gSev.sigev_signo = SIGRTMIN;
    gSev.sigev_value.sival_ptr = &gTimerId;
    if (timer_create (CLOCK_MONOTONIC, &gSev, &gTimerId) != 0)
    {
        ASSERT_ALWAYS_STRING ("actionTimerServerStart: failed timer_create().");        
    }
    
    /* Start the timer */
     gIts.it_value.tv_sec = TIMER_INTERVAL_NANOSECONDS / 1000000000;
     gIts.it_value.tv_nsec = TIMER_INTERVAL_NANOSECONDS % 1000000000;
     gIts.it_interval.tv_sec = gIts.it_value.tv_sec;
     gIts.it_interval.tv_nsec = gIts.it_value.tv_nsec;

     if (timer_settime (gTimerId, 0, &gIts, PNULL) !== 0)
     {
         ASSERT_ALWAYS_STRING ("actionTimerServerStart: failed timer_settime().");                 
     }
}

/*
 * Handle a message that will cause us to stop.
 */
static void actionTimerServerStop (void)
{
    UInt32 x = 0;
    TimerEntry * pEntry = PNULL;

    /* Return used timers to the free list */
    freeAllTimers ();

    pthread_mutex_lock (&lockLinkedLists);
    
    /* Free the free list from the end */
    pEntry = &gFreeTimerListHeadUnused;
    for (x = 0; (pEntry->pNextEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        pEntry = pEntry->pNextEntry;
    }
    for (x = 0; (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        TimerEntry * pNextOne = pEntry->pPrevEntry;
        free (pEntry);
        pEntry = pNextOne;
    }
    
    if (timer_delete (&gTimerId) !== 0)
    {
        ASSERT_ALWAYS_STRING ("actionTimerServerStop: failed timer_delete().");                 
    }

    pthread_mutex_unlock (&lockLinkedLists);
    
    
    /* Destroy the mutex */
    pthread_mutex_destroy (&lockLinkedLists);
}

/*
 * Handle a message that will start a timer.
 *
 * pTimerStartReq  the timer start request message.
 */
static void actionTimerStart (TimerStartReq *pTimerStartReq)
{
    Timer * pTimer;
    
    printDebug ("actionTimerStart: starting a timer of duration %d 10ths of a second (from port %d, id %d, pContext 0x%lx).\n",
                pTimerStartReq->expiryDeciSeconds,
                pTimerStartReq->sourcePort,
                pTimerStartReq->id,
                pTimerStartReq->pContext);

    /* Allocate a timer */
    pTimer = allocTimer();
    if (pTimer != PNULL)
    {
        /* Fill in the contents */
        pTimer->expiryTimeDeciSeconds = gTimerTickDeciSeconds + pTimerStartReq->expiryDeciSeconds;
        pTimer->id = pTimerStartReq->id;
        pTimer->sourcePort = pTimerStartReq->sourcePort;
        pTimer->pContext = pTimerStartReq->pContext;
    }
    else
    {
        ASSERT_ALWAYS_STRING ("actionTimerStart: unable to allocate a timer.");        
    }    
}

/*
 * Handle a message that will stop a timer.
 * 
 * pTimerStopReq  the timer stop request message.
 */
static void actionTimerStop (TimerStopReq *pTimerStopReq)
{
    UInt32 x;
    Bool found = false;
    TimerEntry * pEntry;

    printDebug ("actionTimerStop: stopping timer id %d from port %d.\n",
                pTimerStopReq->id,
                pTimerStopReq->sourcePort);

    pEntry = &gUsedTimerListHead;
    for (x = 0; !found && (pEntry != PNULL) && (pEntry->inUse) && (x < MAX_NUM_TIMERS); x++)
    {
        if ((pEntry->timer.id == pTimerStopReq->id) && (pEntry->timer.sourcePort == pTimerStopReq->sourcePort))
        {
            found = true;
            
            pthread_mutex_lock (&lockLinkedLists);
            freeTimerUnprotected (&(pEntry->timer));
            pthread_mutex_unlock (&lockLinkedLists);
        }
        pEntry = pEntry->pNextEntry;
    }
}

/*
 * Handle the received message and implement the action.
 * 
 * receivedMsgType  the msgType, extracted from the
 *                  received mesage.
 * pReceivedMsgBody pointer to the body part of the
 *                  received message.
 *
 * @return          SERVER_SUCCESS_KEEP_RUNNING unless
 *                  exiting in which case SERVER_EXIT_NORMALLY.
 */
static ServerReturnCode doAction (TimerMsgType receivedMsgType, UInt8 * pReceivedMsgBody)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        case TIMER_SERVER_START_REQ:
        {
            actionTimerServerStart();
        }
        break;
        case TIMER_SERVER_STOP_REQ:
        {
            actionTimerServerStop();
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        case TIMER_START_REQ:
        {
            actionTimerStart ((TimerStartReq *) pReceivedMsgBody);
        }
        break;
        case TIMER_STOP_REQ:
        {
            actionTimerStop ((TimerStopReq *) pReceivedMsgBody);
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (receivedMsgType);   
        }
        break;
    }
    
    return returnCode;
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
 *                the response into, may be PNULL as
 *                this server never sends responses.
 * 
 * @return        whatever doAction() returns.
 */
ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg *pSendMsg)
{
    ServerReturnCode returnCode;
    
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);

    /* Never return any confirmations so set this length to zero */ 
    if (pSendMsg != PNULL)
    {
        pSendMsg->msgLength = 0;
    }
    
    /* Check the type */
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_TIMER_MSGS, pReceivedMsg->msgType);
    
    printDebug ("T  Server received message %s, length %d.\n", pgTimerMessageNames[pReceivedMsg->msgType], pReceivedMsg->msgLength);
    printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);
    /* Do the thang */
    returnCode = doAction ((TimerMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody);
        
    return returnCode;
}
