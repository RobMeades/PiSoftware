/*
 * Handle Hindbrain Direct protocol tasks.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <task_handler_types.h>

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
    Bool taskPresent;
    RoboOneTaskReq task;
} TaskItem;

/*
 * GLOBALS - prefixed with g
 */

static TaskItem gTaskListRoot;

/*
 * STATIC FUNCTIONS
 */

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
        printDebug ("pT->pNextTask != PNULL.\n");
        pT = pT->pNextTask;
        guardCounter++;
    }
    
    ASSERT_PARAM (guardCounter < MAX_GUARD_COUNTER, guardCounter);
    
    printDebug ("Adding the task onto the end of the list.\n");
    /* Tag the new one on the end */
    pTaskItem->pPreviousTask = pT;
    pT->pNextTask = pTaskItem;
}

/*
 * Remove unused asks from the task list.
 */
static void removeUnusedTasksFromList (void)
{
    UInt16 guardCounter = 0;
    TaskItem *pThis = &gTaskListRoot;
    TaskItem *pPrevious = &gTaskListRoot;
    
    ASSERT_PARAM (pThis->pPreviousTask == PNULL, (unsigned long) pThis->pPreviousTask);

    printDebug ("Removing unused tasks from list.\n");
    
    /* Look for items where the task no longer exists
     * and remove them from the list, obviously avoiding
     * doing this to the root of the list */
    pThis = pThis->pNextTask;
    while ((pThis != PNULL) && (guardCounter < MAX_GUARD_COUNTER))
    {
        printDebug ("pThis != PNULL.\n");
        if (!pThis->taskPresent)
        {
            printDebug ("pThis->taskPresent = false.\n");
            pThis->pNextTask->pPreviousTask = pThis->pPreviousTask;
            pPrevious->pNextTask = pThis->pNextTask;
            printDebug ("Freeing it.\n");
            free (pThis);
            printDebug ("Moving pointer on.\n");
            pThis = pPrevious->pNextTask;
        }
        else
        {
            printDebug ("pThis->taskPresent = true, just moving pointers on.\n");
            pThis = pThis->pNextTask;
            pPrevious = pThis;
        }
        
        guardCounter++;
    }
    
    ASSERT_PARAM (guardCounter < MAX_GUARD_COUNTER, guardCounter);
}

/*
 * Walk the task list and print useful stuff out.
 * 
 * @return  number of tasks in the list;
 * 
 */
static UInt16 walkTaskList (void)
{
    UInt16 guardCounter = 0;
    TaskItem *pT = &gTaskListRoot;
    UInt16 count = 0;
    
    ASSERT_PARAM (pT->pPreviousTask == PNULL, (unsigned long) pT->pPreviousTask);

    printDebug ("Walking task list.\n");
    while ((pT->pNextTask != PNULL) && (guardCounter < MAX_GUARD_COUNTER))
    {
        printDebug ("pT->pNextTask != PNULL.\n");
        pT = pT->pNextTask;
        count++;
        printDebug ("Count %d.\n", count);
        guardCounter++;
    }
    
    ASSERT_PARAM (guardCounter < MAX_GUARD_COUNTER, guardCounter);

    printDebug ("Final count %d.\n", count);
    
    return count;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Initialise the task handler.
 */
void initTaskHandler (void)
{
    gTaskListRoot.pNextTask = PNULL;
    gTaskListRoot.pPreviousTask = PNULL;
    gTaskListRoot.taskPresent = false;
}

/*
 * Clear the task list.
 */
void clearTaskList (void)
{
    UInt16 guardCounter = 0;
    TaskItem *pThis = &gTaskListRoot;
    TaskItem *pRoot = &gTaskListRoot;
    
    ASSERT_PARAM (pThis->pPreviousTask == PNULL, (unsigned long) pThis->pPreviousTask);

    printDebug ("Clearing task list.\n");
    /* Free up everything except the root entry
     * TODO: tell interested parties about this */
    pThis = pThis->pNextTask;
    while ((pThis != PNULL) && (guardCounter < MAX_GUARD_COUNTER))
    {
        printDebug ("pThis != PNULL.\n");
        if (pThis->pNextTask != PNULL)
        {
            pThis->pNextTask->pPreviousTask = pThis->pPreviousTask;
        }
        pRoot->pNextTask = pThis->pNextTask;
        printDebug ("Freeing it.\n");
        free (pThis);
        printDebug ("Move pointer on.\n");
        pThis = pRoot->pNextTask;
        guardCounter++;
    }
    
    ASSERT_PARAM (guardCounter < MAX_GUARD_COUNTER, guardCounter);    
}

/*
 * Handle a new task.
 * 
 * pTaskReq  pointer to the task to be handled.
 * 
 * @return   true if successful, otherwise false.
 */
Bool handleTaskReq (RoboOneTaskReq *pTaskReq)
{
    Bool success = false;
    TaskItem *pTaskItem;
    
    ASSERT_PARAM (pTaskReq != PNULL, (unsigned long) pTaskReq);

    printDebug ("Handling a new task.\n");
    pTaskItem = malloc (sizeof (*pTaskItem));
    if (pTaskItem != PNULL)
    {
        memcpy (&(pTaskItem->task), pTaskReq, sizeof (pTaskItem->task));
        pTaskItem->pNextTask = PNULL;
        pTaskItem->pPreviousTask = PNULL;
        pTaskItem->taskPresent = true;
        addTaskToList (pTaskItem);
        success = true;
    }
    
    return success;
}

/*
 * Tick the task handler over,
 * allowing it to do stuff.
 */
void tickTaskHandler (void)
{
    removeUnusedTasksFromList();
    printDebug ("Task Handler: received tick message, %d tasks in the list.\n", walkTaskList());
    printDebug ("Task Handler: received tick message.\n");
}