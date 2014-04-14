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

typedef struct RoboOneContextTag
{
  RoboOneState state;
} RoboOneContext;

/*
 * FUNCTION PROTOTYPES
 */