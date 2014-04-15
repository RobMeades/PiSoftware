/*
 *  Public stuff for the state machine server
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE TYPES
 */

/*
 * TYPES FOR REQ MESSAGES
 */
typedef struct RoboOneContextContainerTag
{
    Bool isValid;
    RoboOneContext roboOneContext;
} RoboOneContextContainer;

/*
 * TYPES FOR CNF MESSAGES
 */

#pragma pack(pop) /* End of packing */ 
