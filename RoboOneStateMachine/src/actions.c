/*
 * Actions taken by the state machine that are used in more than one state.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>

/*
 * MANIFEST CONSTANTS
 */
#define PING_STRING "PiHello"
#define O_RESPONSE_STRING_LENGTH 10
#define O_CHECK_OK_STRING(PoUTPUTsTRING) (((PoUTPUTsTRING)->stringLength) >= 2) && (((PoUTPUTsTRING)->string[0] == 'O') && ((PoUTPUTsTRING)->string[1] == 'K') ? true : false)  

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
    OInputString *pInputString;
    OString *pOutputString;
    UInt8 i;
    
    pInputString = malloc (sizeof (OInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->stringLength = strlen (PING_STRING) + 1;
        pInputString->waitForResponse = true;
        
        pOutputString = malloc (sizeof (OString));
        if (pOutputString != PNULL)
        {
            pOutputString->stringLength = 0;
            /* Do this twice in case the Hindbrain is already on and the first toggle switches it off */
            for (i = 0; !success && (i < 2); i++)
            {
                /* Toggle the power */
                success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_12V_OFF, PNULL, 0, PNULL);
                if (success)
                {
                    /* Send the ping string and check for an OK response */
                    success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, sizeof (OInputString), pOutputString);
                    if (success)
                    {
                       if (O_CHECK_OK_STRING (pOutputString))
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
            free (pOutputString);
        }
        free (pInputString);
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
    OInputString *pInputString;
    UInt8 i;
    
    pInputString = malloc (sizeof (OInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->stringLength = strlen (PING_STRING) + 1;
        pInputString->waitForResponse = false;
        
        /* Do this twice in case the Hindbrain is already off and the first toggle switches it on */
        for (i = 0; !success && (i < 2); i++)
        {
            /* Toggle the power */
            success = hardwareServerSendReceive (HARDWARE_TOGGLE_O_PWR, PNULL, 0, PNULL);
            if (success)
            {
                /* Send the ping string - it should fail to send */
                if (hardwareServerSendReceive (HARDWARE_TOGGLE_O_PWR, pInputString, sizeof (OInputString), PNULL));
                {
                    success = false;
                }
            }
        }
        free (pInputString);
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
    success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_12V_ON, PNULL, 0, PNULL);
    if (success)
    {
        /* Switch battery power off to Rio/Pi */
        success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_BATT_OFF, PNULL, 0, PNULL);
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
    success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_BATT_ON, PNULL, 0, PNULL);
    if (success)
    {
        /* Switch 12V/mains power off to Rio/Pi */
        success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_12V_OFF, PNULL, 0, PNULL);
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
    Bool success = false;
    OInputString *pInputString;
    OString *pOutputString;

    pInputString = malloc (sizeof (OInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->stringLength = strlen (PING_STRING) + 1;
        pInputString->waitForResponse = true;
        
        pOutputString = malloc (sizeof (OString));
        if (pOutputString != PNULL)
        {
            pOutputString->stringLength = 0;
            /* Switch 12V/mains power on to the Hindbrain */
            success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_12V_ON, PNULL, 0, PNULL);
            if (success)
            {
                /* Switch battery power off to the Hindbrain */
                success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_BATT_OFF, PNULL, 0, PNULL);    
                if (success)
                {
                    /* Send the ping string and check for an OK response */
                    success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, sizeof (OInputString), pOutputString);
                    if (success)
                    {
                       if (!O_CHECK_OK_STRING (pOutputString))
                       {
                           success = false;
                       }
                    }
                }
            }
            free (pOutputString);
        }
        free (pInputString);
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
    Bool success = false;
    OInputString *pInputString;
    OString *pOutputString;

    pInputString = malloc (sizeof (OInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->stringLength = strlen (PING_STRING) + 1;
        pInputString->waitForResponse = true;
        
        pOutputString = malloc (sizeof (OString));
        if (pOutputString != PNULL)
        {
            pOutputString->stringLength = 0;
            /* Switch battery power on to the Hindbrain */
            success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_BATT_ON, PNULL, 0, PNULL);
            if (success)
            {
                /* Switch 12V/mains power off to the Hindbrain */
                success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_12V_OFF, PNULL, 0, PNULL);    
                if (success)
                {
                    /* Send the ping string and check for an OK response */
                    success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, sizeof (OInputString), pOutputString);
                    if (success)
                    {
                        if (!O_CHECK_OK_STRING (pOutputString))
                        {
                            success = false;
                        }
                    }
                }
            }
            free (pOutputString);
        }
        free (pInputString);
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