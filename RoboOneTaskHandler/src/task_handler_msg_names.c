/*
 * task_handler_msg_names.c
 * An array of message names for debug purposes.
 */

#include <rob_system.h>
#include <task_handler_msg_macros.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * GLOBALS - prefixed with g
 */
Char *pgTaskHandlerMessageNames[] =
{
#undef TASK_HANDLER_MSG_DEF
#define TASK_HANDLER_MSG_DEF TASK_HANDLER_MSG_DEF_NAME
#include <task_handler_msgs.h>
};
 