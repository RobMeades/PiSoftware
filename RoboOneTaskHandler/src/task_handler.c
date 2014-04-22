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
    TaskItem *pT = &gTaskListRoot;
    
    ASSERT_PARAM (pT->pPreviousTask == PNULL, (unsigned long) pT->pPreviousTask);
    ASSERT_PARAM (pTaskItem->pPreviousTask == PNULL, (unsigned long) pTaskItem->pPreviousTask);
    ASSERT_PARAM (pTaskItem->pNextTask == PNULL, (unsigned long) pTaskItem->pNextTask);

    /* Find the item at the end of the list */
    while (pT->pNextTask != PNULL)
    {
        pT = pT->pNextTask;
    }
    
    /* Tag the new one on the end */
    pTaskItem->pPreviousTask = pT;
    pT->pNextTask = pTaskItem;
}

/*
 * Remove unused asks from the task list.
 */
static void removeUnusedTasksFromList (void)
{
    TaskItem *pThis = &gTaskListRoot;
    TaskItem *pPrevious = &gTaskListRoot;
    
    ASSERT_PARAM (pThis->pPreviousTask == PNULL, (unsigned long) pThis->pPreviousTask);

    /* Look for items where the task no longer exists
     * and remove them from the list, obviously avoiding
     * doing this to the root of the list */
    pThis = pPrevious->pNextTask;
    while (pThis != PNULL)
    {
        if (!pThis->taskPresent)
        {
            pThis->pNextTask->pPreviousTask = pThis->pPreviousTask;
            pPrevious->pNextTask = pThis->pNextTask;
            free (pThis);
        }
        pThis = pPrevious->pNextTask;
        pPrevious = pThis->pPreviousTask;
    }
}

/*
 * Walk the task list and print useful stuff out.
 * 
 * @return  number of tasks in the list;
 * 
 */
static UInt16 walkTaskList (void)
{
    TaskItem *pT = &gTaskListRoot;
    UInt16 count = 0;
    
    ASSERT_PARAM (pT->pPreviousTask == PNULL, (unsigned long) pT->pPreviousTask);

    while (pT->pNextTask != PNULL)
    {
        pT = pT->pNextTask;
        count++;
    }
    
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
    TaskItem *pThis = &gTaskListRoot;
    TaskItem *pPrevious = &gTaskListRoot;
    
    ASSERT_PARAM (pThis->pPreviousTask == PNULL, (unsigned long) pThis->pPreviousTask);

    /* Free up everything except the root entry
     * TODO: tell interested parties about this */
    pThis = pPrevious->pNextTask;
    while (pThis != PNULL)
    {
        pThis->pNextTask->pPreviousTask = pThis->pPreviousTask;
        pPrevious->pNextTask = pThis->pNextTask;
        free (pThis);
        pThis = pPrevious->pNextTask;
        pPrevious = pThis->pPreviousTask;
    }
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
}