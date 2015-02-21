/*
 *  Messages that go from/to the task handler server.
 */

/* This file is included in the task_handler_msg_auto header and
 * defines the message structures for the messages sent to
 * and from the server.  The items in the list are:
 * 
 * - the message type enum,
 * - the message typedef struct (without a Cnf, Req or Ind on the end),
 * - the message variable name for use in the message union
 *   (again, without a Cnf, Req or Ind on the end),
 * - the message member that is needed in the Req message structure
 *   beyond the mandatory msgHeader (which is added automagically),
 * - the message member that is needed in the Ind message structure.
 * 
 * NOTE: the Cnf message structure has fixed contents and so is not
 * considered here.
 */

/*
 * Definitions
 */
TASK_HANDLER_MSG_DEF (TASK_HANDLER_SERVER_START, TaskHandlerServerStart, taskHandlerServerStart, TASK_HANDLER_EMPTY, TASK_HANDLER_EMPTY)
TASK_HANDLER_MSG_DEF (TASK_HANDLER_SERVER_STOP, TaskHandlerServerStop, taskHandlerServerStop, TASK_HANDLER_EMPTY, TASK_HANDLER_EMPTY)
TASK_HANDLER_MSG_DEF (TASK_HANDLER_TICK, TaskHandlerTick, taskHandlerTick, TASK_HANDLER_EMPTY, TASK_HANDLER_EMPTY)
TASK_HANDLER_MSG_DEF (TASK_HANDLER_NEW_TASK, TaskHandlerTask, taskHandlerTask, RoboOneTaskReq taskReq, TASK_HANDLER_EMPTY)
TASK_HANDLER_MSG_DEF (TASK_HANDLER_TASK_IND, TaskHandlerTaskInd, taskHandlerTaskInd, RoboOneTaskInd taskInd, TASK_HANDLER_EMPTY)
