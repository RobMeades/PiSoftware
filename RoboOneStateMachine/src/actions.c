/*
 * Actions taken by the state machine that are used in more than one state.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <state_machine_interface.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS:
 */

/*
 * Switch on the Hindbrain (AKA Orangutan).
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchOnHindbrain (void)
{
    Bool success = false;
    
    /* Toggle the power */
    /* Check for a response */
    /* Initialise sensors */
    
    return success;
}

/*
 * Switch off the Hindbrain (AKA Orangutan).
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchOffHindbrain (void)
{
    Bool success = false;
    
    /* Toggle the power */
    /* Check for lack of a response */

    return success;
}

/*
 * Switch the Pi/RIO to mains/12V power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchRioToMainsPower (void)
{
    Bool success = false;

    /* Switch 12V/mains power on to Rio/Pi */
    /* Switch battery power off to Rio/Pi */
    
    return success;
}

/*
 * Switch Pi/RIO to battery power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchRioToBatteryPower (void)
{
    Bool success = false;
    
    /* Switch battery power on to Rio/Pi */
    /* Switch 12V/mains power off to Rio/Pi */

    return success;
}

/*
 * Switch the Hindbrain (AKA Orangutan)
 * to mains/12V power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchHindbrainToMainsPower (void)
{
    Bool success = false;

    /* Switch 12V/mains power on to Hindbrain */
    /* Switch battery power off to Hindbrain */
    /* Check for a response */
    
    return success;
}

/*
 * Switch the Hindbrain (AKA Orangutan)
 * to battery power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchHindbrainToBatteryPower (void)
{
    Bool success = false;

    /* Switch battery power on to Hindbrain */
    /* Switch 12V/mains power off to Hindbrain */
    /* Check for a response */
    
    return success;
}

/*
 * Start the timer.
 * 
 * @return  true if successful, otherwise false.
 */
Bool startTimer (void)
{
    Bool success = false;
    
    return success;
}

/*
 * Stop the timer.
 * 
 * @return  true if successful, otherwise false.
 */
Bool stopTimer (void)
{
    Bool success = false;
    
    return success;
}