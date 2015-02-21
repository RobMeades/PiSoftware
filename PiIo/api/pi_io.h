/*
 * Global types for Pi IO.
 * Borrows from http://www.pieter-jan.com/node/15
 */ 

/*
 * MANIFEST CONSTANTS
 */

/* GPIO handling macros
 * These can be used directly if openIo() is called first.  If it is not,
 * then the function call interface must be used.
 */

/* Configure a GPIO as an input */
#define GPIO_CONFIG_INPUT(g)  (*(gGpio.pAddress + ((g) / 10)) &= ~(7 << (((g) % 10) * 3)))

/* Configure a GPIO as an output */
#define GPIO_CONFIG_OUTPUT(g) (*(gGpio.pAddress + ((g) / 10)) = (*(gGpio.pAddress + ((g) / 10)) & ~(7 << (((g) % 10) * 3))) |  (1 << (((g) % 10) * 3))) 

/* Configure a GPIO for an alternate function */
#define GPIO_CONFIG_ALT(g, a) (*(gGpio.pAddress + (((g) / 10))) |= (((a) <= 3 ? (a) + 4 : (a) == 4 ? 3 : 2) << (((g) % 10) * 3)))

/*
 * Set a GPIO
 */  
#define GPIO_SET(g)              (*(gGpio.pAddress + 7) = 1 << g)  /* sets   bits which are 1 ignores bits which are 0 */

/*
 * Clear a GPIO
 */  
#define GPIO_CLR(g)              (*(gGpio.pAddress + 10) = 1 << g) /* clears bits which are 1 ignores bits which are 0 */

/* Read a GPIO */
#define GPIO_READ(g)          (*(gGpio.pAddress + 13) &= (1 << (g)))

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
 
/* Variables
 */
extern Bcm2835Peripheral gGpio;

/*
 *  FUNCTION PROTOTYPES
 */
Bool openIo (void);
void closeIo (void);
Bool configGpioInput (UInt8 gpio);
Bool configGpioOutput (UInt8 gpio);
Bool setGpio (UInt8 gpio);
Bool clrGpio (UInt8 gpio);
Bool readGpio (UInt8 gpio, Bool *pState);
