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
#include <pthread.h>
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
#define MAX_NUM_TIMERS 30
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
    Msg expiryMsg;
} Timer;

/* A linked list entry holding a timer */
typedef struct TimerEntryTag
{
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
static TimerEntry * pgFreeTimerListHead = PNULL;
/* Pointer to head of used timer linked list */
static TimerEntry * pgUsedTimerListHead = PNULL;
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
 * Print the used timer list for debug purposes.
 * 
 * IMPORTANT: the lockLinkedLists mutex MUST be held
 * by the calling function!!!  It is not grabbed
 * here as this function is only called from places
 * where it is already held and recursive mutexes appear
 * to be only unreliably present in Linux.
 * 
 */
void printDebugUsedTimerListUnprotected (void)
{
    UInt32 x;
    TimerEntry * pEntry = pgUsedTimerListHead;

    if (pgUsedTimerListHead == PNULL)
    {
        printDebug ("No timers running (pgUsedTimerListHead 0x%08x):\n", pgUsedTimerListHead);
    }
    else
    {
        printDebug ("Timers running (pgUsedTimerListHead 0x%08x):\n", pgUsedTimerListHead);
        for (x = 0; (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
        {
            printDebug (" %d (0x%08x): expires %d, id %d/%d, expiryMsg.msgType 0x%08x, next 0x%08x.\n",
                        x,
                        &(pEntry->timer),
                        pEntry->timer.expiryTimeDeciSeconds,
                        pEntry->timer.sourcePort,
                        pEntry->timer.id,
                        pEntry->timer.expiryMsg.msgType,
                        pEntry->pNextEntry);
            pEntry = pEntry->pNextEntry;
        }
    }
}

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
 * Send a timer expiry message.
 * 
 * pTimer   the timer related to the
 *          expiry message to send.
 * 
 * @return  true if the message send is
 *          is successful, otherwise false.
 */
static Bool sendTimerExpiryMsg (Timer *pTimer)
{
    ClientReturnCode returnCode;
    Bool success = false;

    ASSERT_PARAM (pTimer != PNULL, (unsigned long) pTimer); 

    printDebug ("Timer Server: sending expiry message to port %d, msgType 0x%08x, length %d, hex dump:\n", pTimer->sourcePort, pTimer->expiryMsg.msgType, pTimer->expiryMsg.msgLength);
    printHexDump (&(pTimer->expiryMsg), pTimer->expiryMsg.msgLength + 1);
    returnCode = runMessagingClient (pTimer->sourcePort, PNULL, &(pTimer->expiryMsg), PNULL);
                
    printDebug ("Timer Server: message system returnCode: %d\n", returnCode);
    if (returnCode == CLIENT_SUCCESS)
    { 
        success = true;
    }

    return success;
}

/*
 * Free a timer.  The timer must exist.
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
    TimerEntry * pEntry = pgUsedTimerListHead;
    TimerEntry * pPrevEntry = PNULL;

    ASSERT_PARAM (pTimer != PNULL, (unsigned long) pTimer);

    printDebug ("freeTimer: freeing the timer at 0x%08x...\n", pTimer);

    /* Find the entry in the list */
    for (x = 0; (pEntry != PNULL) && (&(pEntry->timer) != pTimer) && (x < MAX_NUM_TIMERS); x++)
    {
        pEntry = pEntry->pNextEntry;
    }

    if ((pEntry != PNULL) && (&(pEntry->timer) == pTimer))
    {
        TimerEntry * pWantedEntry = pEntry;
        TimerEntry ** ppEntry = PNULL;

        printDebug ("freeTimer: found the timer.\n");
        /* Unlink it from the used list */
        memset (&(pWantedEntry->timer), 0, sizeof (pWantedEntry->timer));
        printDebug ("freeTimer: moving entry to free list.\n");
        /* Link the previous entry's next entry pointer to the one ahead of this */
        if (pWantedEntry->pPrevEntry != PNULL)
        {
            pWantedEntry->pPrevEntry->pNextEntry = pWantedEntry->pNextEntry;
        }
        /* If it is the first entry in the list that is expiring, move the head */
        if (pWantedEntry == pgUsedTimerListHead)
        {
            pgUsedTimerListHead = pWantedEntry->pNextEntry;
        }
        /* Now link the next entry's previous entry pointer to the one before this */
        if (pWantedEntry->pNextEntry != PNULL)
        {
            pWantedEntry->pNextEntry->pPrevEntry = pWantedEntry->pPrevEntry;
        }

        /* Put it on the end of the free list. TODO putting it on the front would be quicker */
        ppEntry = &pgFreeTimerListHead;
        for (x = 0; (*ppEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
        {
            pPrevEntry = *ppEntry;
            ppEntry = &((*ppEntry)->pNextEntry);
        }
        if (*ppEntry == PNULL)
        {
            *ppEntry = pWantedEntry;
            pWantedEntry->pPrevEntry = pPrevEntry;
            pWantedEntry->pNextEntry = PNULL;
        }
        else
        {
            ASSERT_ALWAYS_PARAM (x);   
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
static void tickHandlerCallback (sigval_t sv)
{
    UInt32 x = 0;
    TimerEntry * pEntry;
    UInt32 tickProcessingStartNanoSeconds;
    
    UNUSED (sv);

    tickProcessingStartNanoSeconds = getProcessTimeNanoSeconds();

    gTimerTickDeciSeconds++;
    
    /* Wrap is several years so don't need to deal with it */

    printDebug ("Tick %d.\n", gTimerTickDeciSeconds);
    
    if (pthread_mutex_trylock (&lockLinkedLists) == 0)
    {
        pEntry = pgUsedTimerListHead; /* Need to assign this here as it may have been changed by whoever held the lock */
        for (x = 0; (pEntry != PNULL) && (x < (MAX_NUM_TIMERS)); x++)
        {
            if (pEntry->timer.expiryTimeDeciSeconds <= gTimerTickDeciSeconds)
            {
                Timer * pTimer = &(pEntry->timer);
    
                printDebug ("tickHandler: %d decisecond timer at 0x%08x expired.\n", pEntry->timer.expiryTimeDeciSeconds, &(pEntry->timer));
                
                /* Timer has expired, send a message back */
                sendTimerExpiryMsg (pTimer);
                
                /* Move to the next entry and then free this timer entry */
                pEntry = pEntry->pNextEntry;
                freeTimerUnprotected (pTimer);
                printDebugUsedTimerListUnprotected();
            }
            else
            {
                pEntry = pEntry->pNextEntry;
            }
        }
        
        pthread_mutex_unlock (&lockLinkedLists);
        printDebug ("tickHandler: processing took %d microsecond(s).\n", (getProcessTimeNanoSeconds() - tickProcessingStartNanoSeconds) / 1000);
    }
    else
    {
        printDebug ("tickHandler: lists locked, skipping at tick %d.\n", gTimerTickDeciSeconds);
    }
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
    UInt32 i = 0;
    UInt32 z = 0;
    TimerEntry * pEntry = pgUsedTimerListHead;
    UInt32 sortingStartNanoSeconds;

    sortingStartNanoSeconds = getProcessTimeNanoSeconds();

    printDebug ("sortUsedList: starting...\n");
    for (x = 0; (pEntry != PNULL) && (x < (MAX_NUM_TIMERS * MAX_NUM_TIMERS)); x++)
    {
        if ((pEntry->pNextEntry != PNULL) && (pEntry->timer.expiryTimeDeciSeconds > pEntry->pNextEntry->timer.expiryTimeDeciSeconds))
        {
            TimerEntry * pThisEntry = pEntry;
            TimerEntry * pNextEntry = pEntry->pNextEntry;

            z++;
            printDebug ("sortUsedList: swapping entry %d (%d, 0x%08x) with entry %d (%d, 0x%08x).\n", i, pEntry->timer.expiryTimeDeciSeconds, &(pEntry->timer), i + 1, pNextEntry->timer.expiryTimeDeciSeconds, &(pNextEntry->timer));
            /* If this entry has a later expiry time than the next one, swap them */
            if (pThisEntry->pPrevEntry != PNULL)
            {
                pThisEntry->pPrevEntry->pNextEntry = pNextEntry;
            }
            if (pNextEntry->pNextEntry != PNULL)
            {
                pNextEntry->pNextEntry->pPrevEntry = pThisEntry;
            }
            pThisEntry->pNextEntry = pNextEntry->pNextEntry;
            pNextEntry->pPrevEntry = pThisEntry->pPrevEntry;
            pThisEntry->pPrevEntry = pNextEntry;
            pNextEntry->pNextEntry = pThisEntry;            
            
            /* If we have just changed the head, swap it around too */
            if (pThisEntry == pgUsedTimerListHead)
            {
                pgUsedTimerListHead = pNextEntry;
            }
            
            /* Restart the sort from the beginning */
            pEntry = pgUsedTimerListHead;
            i = 0;
        }
        else
        {
            pEntry = pEntry->pNextEntry;
            i++;
        }
    }
    
    if (x == MAX_NUM_TIMERS * MAX_NUM_TIMERS)
    {
        printDebug ("sortUsedList: WARNING, sorting the timer list hit the buffers (%d iteration(s) (%d swap(s)), %d microsecond(s)).\n", x, z, (getProcessTimeNanoSeconds() - sortingStartNanoSeconds) / 1000);
    }
    else
    {
        printDebug ("sortUsedList: sorting the timer list took %d iteration(s) (%d swap(s)), %d microsecond(s).\n", x, z, (getProcessTimeNanoSeconds() - sortingStartNanoSeconds) / 1000);        
    }
    printDebugUsedTimerListUnprotected();
}

/*
 * Allocate a timer and re-sort the timer list.
 * 
 * periodDeciSeconds  the timer period in deciSeconds
 * sourcePort         the port to send the expiry message to
 * id                 the id for the timer
 * pExpiryMsg         a pointer to the expiry message to send
 * 
 * @return  a pointer to the timer or PNULL if
 *          unable to allocate one.
 */
static Timer * allocTimer (UInt32 periodDeciSeconds, SInt32 sourcePort, TimerId id, ShortMsg * pExpiryMsg)
{
    Timer * pAlloc = PNULL;
    UInt32 x = 0;
    TimerEntry * pEntry;
    TimerEntry * pPrevEntry = PNULL;

    ASSERT_PARAM (pExpiryMsg != PNULL, (unsigned long) pExpiryMsg);
    
    pthread_mutex_lock (&lockLinkedLists);
    pEntry = pgFreeTimerListHead; /* Assign this here in case it has been changed by whoever held the lock */

    printDebug ("allocTimer: finding entry in the free list...\n");
    /* Find the entry at the end of the free list. TODO quicker to take it from the front */
    for (x = 0; (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        pPrevEntry = pEntry;
        pEntry = pEntry->pNextEntry;
    }

    /* If there is one, move it to the used list */
    if (pPrevEntry != PNULL)
    {
        TimerEntry * pWantedEntry = pPrevEntry;
        TimerEntry ** ppEntry = PNULL;

        /* Unlink it from the end of the free list */
        if (pWantedEntry->pPrevEntry != PNULL)
        {
            pWantedEntry->pPrevEntry->pNextEntry = pWantedEntry->pNextEntry;
        }
        if (pWantedEntry == pgFreeTimerListHead) /* Deal with empty free list case */
        {
            pgFreeTimerListHead = PNULL;
        }        

        /* Attach it to the end of the used list */
        ppEntry = &pgUsedTimerListHead;
        pPrevEntry = PNULL;
        for (x = 0; (*ppEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
        {
            pPrevEntry = *ppEntry; 
            ppEntry = &((*ppEntry)->pNextEntry);
        }

        if (*ppEntry == PNULL)
        {
            *ppEntry = pWantedEntry;
            pWantedEntry->pPrevEntry = pPrevEntry;
            pWantedEntry->pNextEntry = PNULL;
            pAlloc = &(pWantedEntry->timer);
            
            /* Copy the data in */
            pAlloc->expiryTimeDeciSeconds = gTimerTickDeciSeconds + periodDeciSeconds;
            pAlloc->id = id;
            pAlloc->sourcePort = sourcePort;
            memcpy (&(pAlloc->expiryMsg), pExpiryMsg, sizeof (pAlloc->expiryMsg));

            printDebug ("allocTimer: timer should expire at tick %d.\n", pAlloc->expiryTimeDeciSeconds);

            /* Now sort the list */
            sortUsedListUnprotected();
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
 * Empty the used timer list
 */
static void freeAllTimers (void)
{
    UInt32 x;
    TimerEntry * pEntry;
    TimerEntry * pNextEntry = PNULL;

    pthread_mutex_lock (&lockLinkedLists);
    pEntry = pgUsedTimerListHead; /* Assign this here in case it has been changed by whoever held the lock */

    printDebug ("freeAllTimers: freeing all timers.\n");
    
    for (x = 0; (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        pNextEntry = pEntry->pNextEntry;
        freeTimerUnprotected (&(pEntry->timer));
        pEntry = pNextEntry;
    }
    pgUsedTimerListHead = PNULL;

    pthread_mutex_unlock (&lockLinkedLists);
}

/*
 * Handle a message that will cause us to start.
 */
static void actionTimerServerStart (void)
{
    UInt32 x;
    TimerEntry ** ppEntry = &pgFreeTimerListHead;
    TimerEntry * pPrevEntry = PNULL;

    /* Create the mutex */
    if (pthread_mutex_init (&lockLinkedLists, NULL) != 0)
    {
        ASSERT_ALWAYS_STRING ("actionTimerServerStart: failed pthread_mutex_init().");
    }
    
    /* Create a malloc()ed free list */
    for (x = 0; x < MAX_NUM_TIMERS; x++)
    {
        *ppEntry = (TimerEntry *) malloc (sizeof (TimerEntry));
        if (*ppEntry != PNULL)
        {
            memset (*ppEntry, 0, sizeof (**ppEntry));
            /* Link it in */
            (*ppEntry)->pPrevEntry = pPrevEntry;
            (*ppEntry)->pNextEntry = PNULL;
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
     * http://man7.org/linux/man-pages/man2/timer_create.2.html
     * but modified to call a handler rather than send a signal
     * 'cos we already use the signalling for our messaging */
    
    /* Create the tick event */
    gSev.sigev_notify = SIGEV_THREAD;
    gSev.sigev_value.sival_ptr = &gTimerId;
    gSev.sigev_notify_function = tickHandlerCallback;
    gSev.sigev_notify_attributes = PNULL;
    if (timer_create (CLOCK_MONOTONIC, &gSev, &gTimerId) != 0)
    {
        ASSERT_ALWAYS_STRING ("actionTimerServerStart: failed timer_create().");        
    }
    
    /* Start the timer */
     gIts.it_value.tv_sec = TIMER_INTERVAL_NANOSECONDS / 1000000000;
     gIts.it_value.tv_nsec = TIMER_INTERVAL_NANOSECONDS % 1000000000;
     gIts.it_interval.tv_sec = gIts.it_value.tv_sec;
     gIts.it_interval.tv_nsec = gIts.it_value.tv_nsec;

     if (timer_settime (gTimerId, 0, &gIts, PNULL) != 0)
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
    TimerEntry * pEntry;

    /* Return used timers to the free list */
    freeAllTimers ();

    pthread_mutex_lock (&lockLinkedLists);
    pEntry = pgFreeTimerListHead; /* Assign this here in case it has been changed by whoever held the lock */
    
    /* Free the free list */
    for (x = 0; (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        TimerEntry * pNextEntry = pEntry->pNextEntry;
        free (pEntry);
        pEntry = pNextEntry;
    }
    pgFreeTimerListHead = PNULL;
    
    if (timer_delete (gTimerId) != 0)
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
    
    printDebug ("actionTimerStart: starting a timer of duration %d 10ths of a second (from port %d, id %d, expiryMsg.msgType 0x%08x).\n",
                pTimerStartReq->expiryDeciSeconds,
                pTimerStartReq->sourcePort,
                pTimerStartReq->id,
                pTimerStartReq->expiryMsg.msgType);

    /* Allocate a timer and fill the data in */
    pTimer = allocTimer (pTimerStartReq->expiryDeciSeconds, pTimerStartReq->sourcePort, pTimerStartReq->id, &(pTimerStartReq->expiryMsg));
    if (pTimer != PNULL)
    {
        printDebug ("actionTimerStart: allocated a timer at 0x%08x.\n", pTimer);
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

    pthread_mutex_lock (&lockLinkedLists);
    pEntry = pgUsedTimerListHead; /* Assign this here in case it has been changed by whoever held the lock */
    
    for (x = 0; !found && (pEntry != PNULL) && (x < MAX_NUM_TIMERS); x++)
    {
        if ((pEntry->timer.id == pTimerStopReq->id) && (pEntry->timer.sourcePort == pTimerStopReq->sourcePort))
        {
            found = true;
            
            freeTimerUnprotected (&(pEntry->timer));
        }
        pEntry = pEntry->pNextEntry;
    }
    
    pthread_mutex_unlock (&lockLinkedLists);
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
