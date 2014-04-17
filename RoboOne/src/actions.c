/*
 * Actions taken by the state machine that are used in more than one state.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <ow_bus.h>
#include <orangutan.h>

/*
 * MANIFEST CONSTANTS
 */
#define PING_STRING "PiHello"
#define O_RESPONSE_STRING_LENGTH 10
#define O_CHECK_OK_STRING(bUFFER) ((sizeof (buffer) >= 2) && ((bUFFER)[0] == 'O') && ((bUFFER)[1] == 'K') ? true : false)  

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
    Char buffer[O_RESPONSE_STRING_LENGTH];
    UInt32 bufferLength = sizeof (buffer); 
    UInt8 i;
    
    /* Do this twice in case the Hindbrain is already on and the first toggle switches it off */
    for (i = 0; !success && (i < 2); i++)
    {
        memset (&(buffer[0]), 0, sizeof (buffer));
        /* Toggle the power */
        success = toggleOPwr();
        if (success)
        {
            /* Send the ping string and check for an OK response */
            success = sendStringToOrangutan (PING_STRING, &(buffer[0]), &bufferLength);
            if (success)
            {
               if (O_CHECK_OK_STRING (buffer))
               {
                   /* TODO: Initialise sensors */
               }
               else
               {
                   success = false;
               }
            }
        }
    }
    
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
    UInt8 i;
    
    /* Do this twice in case the Hindbrain is already off and the first toggle switches it on */
    for (i = 0; !success && (i < 2); i++)
    {
        /* Toggle the power */
        success = toggleOPwr();
        if (success)
        {
            /* Send the ping string - it should fail to send */
            if (sendStringToOrangutan (PING_STRING, PNULL, PNULL))
            {
                success = false;
            }
        }
    }
        
    return success;
}

/*
 * Switch the Pi/RIO to mains/12V power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchPiRioTo12VMainsPower (void)
{
    Bool success;

    /* Switch 12V/mains power on to Rio/Pi */
    success = setRioPwr12VOn();
    if (success)
    {
        /* Switch battery power off to Rio/Pi */
        success = setRioPwrBattOff();
    }
    
    return success;
}

/*
 * Switch Pi/RIO to battery power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchPiRioToBatteryPower (void)
{
    Bool success;
    
    /* Switch battery power on to Rio/Pi */
    success = setRioPwrBattOn();
    if (success)
    {
        /* Switch 12V/mains power off to Rio/Pi */
        success = setRioPwr12VOff();
    }

    return success;
}

/*
 * Switch the Hindbrain (AKA Orangutan)
 * to mains/12V power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool switchHindbrainTo12VMainsPower (void)
{
    Bool success;
    Char buffer[O_RESPONSE_STRING_LENGTH];
    UInt32 bufferLength = sizeof (buffer); 

    memset (&(buffer[0]), 0, sizeof (buffer));

    /* Switch 12V/mains power on to the Hindbrain */
    success = setOPwr12VOn();
    if (success)
    {
        /* Switch battery power off to the Hindbrain */
        success = setOPwrBattOff();
        if (success)
        {
            /* Check for a response */
            success = sendStringToOrangutan (PING_STRING, &(buffer[0]), &bufferLength);
            if (success)
            {
               if (!O_CHECK_OK_STRING (buffer))
               {
                   success = false;
               }
            }            
        }
    }
    
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
    Bool success;
    Char buffer[O_RESPONSE_STRING_LENGTH];
    UInt32 bufferLength = sizeof (buffer); 

    memset (&(buffer[0]), 0, sizeof (buffer));

    /* Switch battery power on to the Hindbrain */
    success = setOPwrBattOn();
    if (success)
    {
        /* Switch 12V/mains power off to the Hindbrain */
        success = setOPwr12VOff();
        if (success)
        {
            /* Check for a response */
            success = sendStringToOrangutan (PING_STRING, &(buffer[0]), &bufferLength);
            if (success)
            {
               if (!O_CHECK_OK_STRING (buffer))
               {
                   success = false;
               }
            }            
        }
    }
    
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