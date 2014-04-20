/*
 * hardware_msg_names.c
 * An array of message names for debug purposes.
 */

#include <rob_system.h>
#include <one_wire_msg_macros.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * GLOBALS - prefixed with g
 */
Char *pgOneWireMessageNames[] =
{
#undef ONE_WIRE_MSG_DEF
#define ONE_WIRE_MSG_DEF ONE_WIRE_MSG_DEF_NAME
#include <one_wire_msgs.h>
};
 