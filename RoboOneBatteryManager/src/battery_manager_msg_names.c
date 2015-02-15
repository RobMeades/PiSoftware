/*
 * battery_manager_msg_names.c
 * An array of message names for debug purposes.
 */

#include <rob_system.h>
#include <battery_manager_msg_macros.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * GLOBALS - prefixed with g
 */
Char *pgBatteryManagerMessageNames[] =
{
#undef BATTERY_MANAGER_MSG_DEF
#define BATTERY_MANAGER_MSG_DEF BATTERY_MANAGER_MSG_DEF_NAME
#include <battery_manager_msgs.h>
};
 