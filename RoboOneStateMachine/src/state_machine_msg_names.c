/*
 * state_machine_msg_names.c
 * An array of message names for debug purposes.
 */

#include <rob_system.h>
#include <state_machine_msg_macros.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * GLOBALS - prefixed with g
 */
Char *pgStateMachineMessageNames[] =
{
#undef STATE_MACHINE_MSG_DEF
#define STATE_MACHINE_MSG_DEF STATE_MACHINE_MSG_DEF_NAME
#include <state_machine_msgs.h>
};
 