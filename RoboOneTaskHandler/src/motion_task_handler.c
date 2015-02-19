/*
 * Handle the execution of tasks that move the robot around.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <hardware_types.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <task_handler_types.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Handle a motion oriented task.
 * 
 * pMTaskReq  pointer to the Motion task
 *            to be handled.
 * pMTaskInd  pointer a place to put the response.
 * 
 * @return    result code from RoboOneMotionResult.
 */
RoboOneMotionResult handleMotionTaskReq (RoboOneMotionTaskReq *pMotionTaskReq, RoboOneMotionTaskInd *pMotionTaskInd)
{
    RoboOneMotionResult result = MOTION_RESULT_GENERAL_FAILURE;

    ASSERT_PARAM (pMotionTaskReq != PNULL, (unsigned long) pMotionTaskReq);
    ASSERT_PARAM (pMotionTaskInd != PNULL, (unsigned long) pMotionTaskInd);
     
    printDebug ("Task Handler: Motion Protocol Task received command %d.\n", pMotionTaskReq->command);
    
    return result;
}