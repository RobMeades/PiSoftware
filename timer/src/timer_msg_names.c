/*
 * timer_msg_names.c
 * An array of message names for debug purposes.
 */

#include <rob_system.h>
#include <timer_msg_macros.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * GLOBALS - prefixed with g
 */
Char *pgTimerMessageNames[] =
{
#undef TIMER_MSG_DEF
#define TIMER_MSG_DEF TIMER_MSG_DEF_NAME
#include <timer_msgs.h>
};
 