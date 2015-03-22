/*
 *  Public stuff for the timer server
 */

/*
 * MANIFEST CONSTANTS
 */

#define TIMER_SERVER_EXE "./timer_server"
#define TIMER_SERVER_PORT_STRING "5235"

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE TYPES
 */

typedef UInt8 TimerId;

/*
 * TYPES FOR REQ MESSAGES
 */

/*
 * TYPES FOR IND MESSAGES
 */

#pragma pack(pop) /* End of packing */ 