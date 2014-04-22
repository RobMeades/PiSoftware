/*
 * Handle the execution of tasks going straight to the Hindbrain.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
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
 * Handle a task directed at the Hindbrain.
 * 
 * pTaskReq  pointer to the Hindbrain Direct task
 *           to be handled.
 * pTaskCnf  pointer to a place to put the
 *           confirmation of the Hindbrain Direct
 *           task having been handled (or not).
 */
void handleHindbrainDirectTaskReq (RoboOneHindbrainDirectTaskReq *pTaskReq, RoboOneHindbrainDirectTaskCnf *pTaskCnf)
{
    ASSERT_PARAM (pTaskReq != PNULL, (unsigned long) pTaskReq);
    ASSERT_PARAM (pTaskCnf != PNULL, (unsigned long) pTaskCnf);

}