/*
 * Actions taken by the state machine that are used in more than one state.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <rob_system.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>

/*
 * MANIFEST CONSTANTS
 */
#define PING_STRING "!\n"
#define O_RESPONSE_STRING_LENGTH 10
#define O_CHECK_OK_STRING(PoUTPUTsTRING) (((PoUTPUTsTRING)->stringLength) >= 2) && (((PoUTPUTsTRING)->string[0] == 'O') && ((PoUTPUTsTRING)->string[1] == 'K') ? true : false)  
#define O_START_DELAY_US 100000L

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
 * Enable all relays
 * 
 * @return  true if successful, otherwise false.
 */
Bool actionEnableAllRelays (void)
{
    return hardwareServerSendReceive (HARDWARE_ENABLE_ALL_RELAYS, PNULL, 0, PNULL);
}

/*
 * Disable all relays
 * 
 * @return  true if successful, otherwise false.
 */
Bool actionDisableAllRelays (void)
{
    return hardwareServerSendReceive (HARDWARE_DISABLE_ALL_RELAYS, PNULL, 0, PNULL);
}

/*
 * Check if 12V/mains is available
 * 
 * @return  true if it is, otherwise false.
 */
Bool actionIsMains12VAvailable (void)
{
    Bool isOn = false;
    
    hardwareServerSendReceive (HARDWARE_READ_MAINS_12V, PNULL, 0, &isOn);
    
    return isOn; 
}

/*
 * Switch on the Hindbrain (AKA Orangutan).
 * 
 * @return  true if successful, otherwise false.
 */
Bool actionSwitchOnHindbrain (void)
{
    Bool success = false;
    OInputString *pInputString;
    OResponseString *pResponseString;
    UInt8 i;
    
    pInputString = malloc (sizeof (*pInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->waitForResponse = true;
        
        pResponseString = malloc (sizeof (*pResponseString));
        if (pResponseString != PNULL)
        {
            pResponseString->stringLength = sizeof (pResponseString->string);
            /* Do this twice in case the Hindbrain is already on and the first toggle switches it off */
            for (i = 0; !success && (i < 2); i++)
            {
                /* Toggle the power */
                success = hardwareServerSendReceive (HARDWARE_TOGGLE_O_PWR, PNULL, 0, PNULL);
                if (success)
                {
                    usleep (O_START_DELAY_US);
                    /* Send the ping string and check for an OK response */
                    success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, strlen (&(pInputString->string[0])) + 1, pResponseString);
                    if (success)
                    {
                       if (O_CHECK_OK_STRING (pResponseString))
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
            free (pResponseString);
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
Bool actionSwitchOffHindbrain (void)
{
    Bool success = false;
    OInputString *pInputString;
    UInt8 i;
    
    pInputString = malloc (sizeof (*pInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->waitForResponse = false;
        
        /* Do this twice in case the Hindbrain is already off and the first toggle switches it on */
        for (i = 0; !success && (i < 2); i++)
        {
            /* Toggle the power */
            success = hardwareServerSendReceive (HARDWARE_TOGGLE_O_PWR, PNULL, 0, PNULL);
            if (success)
            {
                /* Send the ping string - it should *fail* to send */
                if (hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, strlen (&(pInputString->string[0])) + 1, PNULL));
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
Bool actionSwitchPiRioTo12VMainsPower (void)
{
    Bool success;

    /* Switch 12V/mains power on to RIO/Pi */
    printDebug ("ACTION: switching on 12V/mains power to RIO/Pi.\n");
    success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_12V_ON, PNULL, 0, PNULL);
    if (success)
    {
        /* Switch battery power off to RIO/Pi */
        printDebug ("ACTION: switching off battery power to RIO/Pi.\n");
        success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_BATT_OFF, PNULL, 0, PNULL);
    }
    
    return success;
}

/*
 * Switch Pi/RIO to battery power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool actionSwitchPiRioToBatteryPower (void)
{
    Bool success;
    
    /* Switch battery power on to Rio/Pi */
    printDebug ("ACTION: switching on battery power to RIO/Pi.\n");
    success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_BATT_ON, PNULL, 0, PNULL);
    if (success)
    {
        /* Switch 12V/mains power off to Rio/Pi */
        printDebug ("ACTION: switching off 12V/mains power to RIO/Pi.\n");
        success = hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_12V_OFF, PNULL, 0, PNULL);
    }

    return success;
}

/*
 * Switch the Hindbrain (AKA Orangutan) to
 * mains/12V power.
 * 
 * @return  true if successful, otherwise false.
 */
Bool actionSwitchHindbrainTo12VMainsPower (void)
{
    Bool success = false;
    OInputString *pInputString;
    OResponseString *pResponseString;

    pInputString = malloc (sizeof (*pInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->waitForResponse = true;
        
        pResponseString = malloc (sizeof (*pResponseString));
        if (pResponseString != PNULL)
        {
            pResponseString->stringLength = sizeof (pResponseString->string);
            /* Switch 12V/mains power on to the Hindbrain */
            printDebug ("ACTION: switching on 12V/mains power to Hindbrain.\n");
            success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_12V_ON, PNULL, 0, PNULL);
            if (success)
            {
                /* Switch battery power off to the Hindbrain */
                printDebug ("ACTION: switching off battery power to Hindbrain.\n");
                success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_BATT_OFF, PNULL, 0, PNULL);    
                if (success)
                {
                    /* Send the ping string and check for an OK response */
                    printDebug ("ACTION: Pinging Hindbrain.\n");
                    success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, strlen (&(pInputString->string[0])) + 1, pResponseString);
                    if (success && !O_CHECK_OK_STRING (pResponseString))
                    {
                        success = false;
                    }
                }
            }
            free (pResponseString);
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
Bool actionSwitchHindbrainToBatteryPower (void)
{
    Bool success = false;
    OInputString *pInputString;
    OResponseString *pResponseString;

    pInputString = malloc (sizeof (*pInputString));
    if (pInputString != PNULL)
    {
        memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
        pInputString->waitForResponse = true;
        
        pResponseString = malloc (sizeof (*pResponseString));
        if (pResponseString != PNULL)
        {
            pResponseString->stringLength = sizeof (pResponseString->string);
            /* Switch battery power on to the Hindbrain */
            printDebug ("ACTION: switching on battery power to Hindbrain.\n");
            success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_BATT_ON, PNULL, 0, PNULL);
            if (success)
            {
                /* Switch 12V/mains power off to the Hindbrain */
                printDebug ("ACTION: switching off 12V/mains power to Hindbrain.\n");
                success = hardwareServerSendReceive (HARDWARE_SET_O_PWR_12V_OFF, PNULL, 0, PNULL);    
                if (success)
                {
                    /* Send the ping string and check for an OK response */
                    printDebug ("ACTION: Pinging Hindbrain.\n");
                    success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, strlen (&(pInputString->string[0])) + 1, pResponseString);
                    if (success && !O_CHECK_OK_STRING (pResponseString))
                    {
                        success = false;
                    }
                }
            }
            free (pResponseString);
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
Bool actionStartTimer (void)
{
    Bool success = false;
    
    return success;
}

/*
 * Stop the timer.
 * 
 * @return  true if successful, otherwise false.
 */
Bool actionStopTimer (void)
{
    Bool success = false;
    
    return success;
}