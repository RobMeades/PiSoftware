/*
 * hardware_msg_names.c
 * An array of message names for debug purposes.
 */

#include <rob_system.h>
#include <hardware_msg_macros.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * GLOBALS - prefixed with g
 */
Char *pgHardwareMessageNames[] =
{
#undef HARDWARE_MSG_DEF
#define HARDWARE_MSG_DEF HARDWARE_MSG_DEF_NAME
#include <hardware_msgs.h>
};
 