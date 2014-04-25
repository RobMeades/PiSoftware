/*
 * Handle Hindbrain Direct protocol tasks.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <task_handler_types.h>
#include <task_handler_server.h>
#include <task_handler_msg_auto.h>
#include <task_handler_responder.h>
#include <hindbrain_direct_task_handler.h>

/*
 * MANIFEST CONSTANTS
 */
#define MAX_GUARD_COUNTER 150

/*
 * TYPES
 */
typedef struct TaskItemTag
{
    struct TaskItemTag *pPreviousTask;
    struct TaskItemTag *pNextTask;
    Bool taskCompleted;
    RoboOneTaskReq task;
    RoboOneTaskInd *pResult;
} TaskItem;

/*
 * GLOBALS - prefixed with g
 */

static TaskItem gTaskListRoot;

/*
 * STATIC FUNCTIONS
 */

/*
 * Initialise and entry in the list
 * 
 * pTaskItem  the item to initialise.
 */
static void initTaskItem (TaskItem *pTaskItem)
{
    pTaskItem->pNextTask = PNULL;
    pTaskItem->pPreviousTask = PNULL;
    pTaskItem->taskCompleted = false;
    pTaskItem->pResult = PNULL;
}

/*
 * Add a task to the task list.
 * 
 * pTaskItem  the task to add, should
 *            be with both next and previous
 *            pointers set to PNULL.
 */
static void addTaskToList (TaskItem *pTaskItem)
{
    UInt16 guardCounter = 0;
    TaskItem *pT = &gTaskListRoot;
    
    ASSERT_PARAM (pT->pPreviousTask == PNULL, (unsigned long) pT->pPreviousTask);
    ASSERT_PARAM (pTaskItem->pPreviousTask == PNULL, (unsigned long) pTaskItem->pPreviousTask);
    ASSERT_PARAM (pTaskItem->pNextTask == PNULL, (unsigned long) pTaskItem->pNextTask);

    printDebug ("Adding a task to the list.\n");
    /* Find the item at the end of the list */
    while ((pT->pNextTask != PNULL) && (guardCounter < MAX_GUARD_COUNTER))
    {
        printDebug ("Moving past used entry %d.\n", guardCounter);
        pT = pT->pNextTask;
        guardCounter++;
    }
    
    ASSERT_PARAM (guardCounter < MAX_GUARD_COUNTER, guardCounter);
    
    printDebug ("Adding the task onto the end of the list at entry %d.\n", guardCounter);
    /* Tag the new one on the end */
    pTaskItem->pPreviousTask = pT;
    pT->pNextTask = pTaskItem;
}

/*
 * Walk the task list, doing several possible things.
 * 
 * removeUnusedTasks  if true, remove the any unused
 *                    tasks, freeing their memory.
 *                    An unused task is one that is
 *                    completed and has a null result
 *                    pointer
 * clearAllTasks      if true, remove all tasks,
 *                    freeing memory.
 * 
 * @return  number of tasks left in the list;
 */
static UInt16 walkTaskList (Bool removeUnusedTasks, Bool clearAllTasks)
{
    UInt16 guardCounter = 0;
    TaskItem *pThis = &gTaskListRoot;
    TaskItem *pPrevious = &gTaskListRoot;
    UInt16 count = 0;
    
    ASSERT_PARAM (pThis->pPreviousTask == PNULL, (unsigned long) pThis->pPreviousTask);

    /* Look for items where the task no longer exists
     * and remove them from the list, obviously avoiding
     * doing this to the root of the list */
    pThis = pThis->pNextTask;
    while ((pThis != PNULL) && (guardCounter < MAX_GUARD_COUNTER))
    {
        printDebug ("Entry %d, task completed %d.\n", guardCounter, pThis->taskCompleted);
        if ((pThis->taskCompleted  && (pThis->pResult == PNULL) && removeUnusedTasks) || clearAllTasks)
        {
            if (pThis->pNextTask != PNULL)
            {
                pThis->pNextTask->pPreviousTask = pThis->pPreviousTask;
            }
            pPrevious->pNextTask = pThis->pNextTask;
            if (pThis->pResult != PNULL)
            {
                printDebug (" freeing result memory.\n");
                free (pThis->pResult);                
            }
            printDebug (" freeing task memory.\n");
            free (pThis);
            printDebug (" moving pointer on.\n");
            pThis = pPrevious->pNextTask;
        }
        else
        {
            printDebug (" moving pointers on.\n");
            pThis = pThis->pNextTask;
            pPrevious = pThis;
            count++;
        }
        
        guardCounter++;
    }
    
    ASSERT_PARAM (guardCounter < MAX_GUARD_COUNTER, guardCounter);
    
    return count;
}

/*
 * Do a Hindbrain Direct protocol task.
 * 
 * pTaskItem  pointer to the task in the linked
 *            list.
 * 
 * @return    true if the task was completed.
 */
static Bool doHDTask (TaskItem *pTaskItem)
{
    Bool success = false;
    ASSERT_PARAM (pTaskItem != PNULL, (unsigned long) pTaskItem);
    ASSERT_PARAM (pTaskItem->pResult == PNULL, (unsigned long) pTaskItem->pResult);

    pTaskItem->pResult = malloc (sizeof (*pTaskItem->pResult));
    if (pTaskItem->pResult != PNULL)
    {
        pTaskItem->pResult->body.protocol = TASK_PROTOCOL_HD;
        pTaskItem->pResult->body.detail.hdInd.result = handleHDTaskReq (&pTaskItem->task.body.detail.hdReq, &pTaskItem->pResult->body.detail.hdInd);
        success = true;
    }
    
    return success;
}

/*
 * Send confirmations of task completeness.
 * 
 * pTaskItem  pointer to the task in the linked
 *            list.
 * 
 * @return    always true.
 */
static Bool doTaskCompleted (TaskItem *pTaskItem)
{
    /* Handle the confirmation if one was requested and is available */
    if (pTaskItem->task.headerPresent && (pTaskItem->pResult != PNULL))
    {
        pTaskItem->pResult->handle = pTaskItem->task.header.handle;
        taskHandlerResponder (&(pTaskItem->task.header), TASK_HANDLER_TASK_IND, pTaskItem->pResult, sizeof (*pTaskItem->pResult));
        free (pTaskItem->pResult);
        pTaskItem->pResult = PNULL;
    }
    
    return true;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Initialise the task list.
 * 
 * @return   true if successful, otherwise false.
 */
Bool initTaskList (void)
{
    initTaskItem (&gTaskListRoot);
    
    return true;
}

/*
 * Empty the task list.
 * 
 * @return   true if successful, otherwise false.
 */
Bool clearTaskList(void)
{
    walkTaskList (false, true);
    
    return true;
}

/*
 * Handle a new task.
 * 
 * pTaskReq  pointer to the task to be handled.
 * 
 * @return   true if successful, otherwise false.
 */
Bool handleNewTaskReq (RoboOneTaskReq *pTaskReq)
{
    Bool success = false;
    TaskItem *pTaskItem;
    
    ASSERT_PARAM (pTaskReq != PNULL, (unsigned long) pTaskReq);

    printDebug ("Handling a new task.\n");
    pTaskItem = malloc (sizeof (*pTaskItem));
    if (pTaskItem != PNULL)
    {
        memcpy (&(pTaskItem->task), pTaskReq, sizeof (pTaskItem->task));
        initTaskItem (pTaskItem);
        addTaskToList (pTaskItem);
        success = true;
    }
    
    return success;
}

/*
 * Tick the task handler over,
 * allowing it to do stuff.
 * 
 * @return   true if successful, otherwise false.
 */
Bool tickTaskHandler (void)
{
    Bool success = true;
    UInt16 guardCounter = 0;
    TaskItem *pT = &gTaskListRoot;
        
    ASSERT_PARAM (pT->pPreviousTask == PNULL, (unsigned long) pT->pPreviousTask);

    printDebug ("Task Handler: received tick message, %d task(s) in the list.\n", walkTaskList (false, false));
    
    while ((pT->pNextTask != PNULL) && (guardCounter < MAX_GUARD_COUNTER))
    {        
        pT = pT->pNextTask;
        if (!pT->taskCompleted)
        {
            switch (pT->task.body.protocol)
            {
                case TASK_PROTOCOL_HD:
                {
                    pT->taskCompleted = doHDTask (pT);
                }
                break;
                default:
                {
                    ASSERT_ALWAYS_PARAM (pT->task.body.protocol);   
                    success = false;
                }
                break;
            }
        }
        
        /* Handle any indications of completion that need doing */
        doTaskCompleted (pT);   

        guardCounter++;
    }
    
    ASSERT_PARAM (guardCounter < MAX_GUARD_COUNTER, guardCounter);

    /* Remove any completed tasks */
    walkTaskList (true, false);

    return success;
}