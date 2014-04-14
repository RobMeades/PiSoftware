/*
 * State machine, based on "Patterns in C - Part 2: STATE" by Adam Petersen
 * http://www.adampetersen.se/Patterns%20in%20C%202,%20STATE.pdf
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

#pragma pack(push, 1) /* This structure is used in messages and so HAS to be fully packed */

typedef struct RoboOneContextTag
{
  RoboOneState state;
} RoboOneContext;

#pragma pack(pop) /* End of packing */ 
/*
 * FUNCTION PROTOTYPES
 */