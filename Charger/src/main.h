/*
 * Global types for Charger.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 

/*
 * MANIFEST CONSTANTS
 */

/* Where the peripherals are connected */
#define GPIO_MOTOR_MICRO_STEP 24 /* There are 1600 micro steps in one revolution */
#define GPIO_MOTOR_DIRECTION  25 /* CLR for anti-clockwise SET for clockwise */
#define GPIO_MOTOR_ENABLE_BAR 18 /* CLR to enable, SET to disable */
#define GPIO_IR_ENABLE        4  /* SET this to enable the IR unit */
/* The IR detect pins: each go _low_ when something is detected */
#define GPIO_IR_DETECT_NORTH  23 /* North is the direction in which the charging socket is mounted */
#define GPIO_IR_DETECT_SOUTH  27
#define GPIO_IR_DETECT_EAST   22
#define GPIO_IR_DETECT_WEST   17
                            
/*
 * TYPES
 */

/*
 *  FUNCTION PROTOTYPES
 */
