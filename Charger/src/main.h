/*
 * Global types for Charger.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 

/*
 * MANIFEST CONSTANTS
 */

/* Where the peripherals are connected */
#define GPIO_RELAY_1_ENABLE   2  /* Set to 1 to switch relay 1 on */
#define GPIO_RELAY_2_ENABLE   3  /* Set to 1 to switch relay 2 on */
#define GPIO_MOTOR_MICRO_STEP 24 /* There are 1600 micro steps in one revolution */
#define GPIO_MOTOR_DIRECTION  25 /* CLR for anti-clockwise SET for clockwise */
#define GPIO_MOTOR_ENABLE_BAR 18 /* CLR to enable, SET to disable */
#define GPIO_IR_ENABLE        4  /* SET this to enable the IR unit */

/* The IR detect pins connected to the OneWire IO chip. */
/* Each goes _low_ when something is detected */
#define OW_PIN_IR_DETECT_NORTH 0x01  /* North is the direction in which the charging socket is mounted */
#define OW_PIN_IR_DETECT_EAST  0x02
#define OW_PIN_IR_DETECT_SOUTH 0x04
#define OW_PIN_IR_DETECT_WEST  0x08

/*
 * TYPES
 */

/*
 *  FUNCTION PROTOTYPES
 */
