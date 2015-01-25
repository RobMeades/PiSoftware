/*
 * Global types for Charger.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 

/*
 * MANIFEST CONSTANTS
 */

/* GPIO memory mapping */
#define BCM2708_PERIPHERALS_BASE 0x20000000
#define GPIO_BASE                (BCM2708_PERIPHERALS_BASE + 0x200000)  /* GPIO controller */ 
 
#define BLOCK_SIZE      (4*1024)
 
/* Where the peripherals are connected */
#define GPIO_MOTOR_STEP       24
#define GPIO_MOTOR_DIRECTION  25
#define GPIO_IR_ENABLE        4
#define GPIO_IR_DETECT_NORTH  23
#define GPIO_IR_DETECT_SOUTH  27
#define GPIO_IR_DETECT_EAST   22
#define GPIO_IR_DETECT_WEST   17
                            
/*
 * TYPES
 */

/* IO Access */
typedef struct Bcm2835Peripheral_Tag
{
    unsigned long addressP;
    int memFd;
    void *pMap;
    volatile unsigned int *pAddress;
} Bcm2835Peripheral;
 
/*
 *  FUNCTION PROTOTYPES
 */
