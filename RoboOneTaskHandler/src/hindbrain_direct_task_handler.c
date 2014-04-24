/*
 * Handle the execution of tasks going straight to the Hindbrain.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <task_handler_types.h>

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
 * PUBLIC FUNCTIONS
 */

/*
 * Handle a task directed at the Hindbrain.
 * 
 * pTaskReq  pointer to the Hindbrain Direct task
 *           to be handled.
 * 
 * @return   true if successful, otherwise false.
 */
Bool handleHindbrainDirectTaskReq (RoboOneHindbrainDirectTaskReq *pTaskReq)
{
    Bool success = false;
    OInputContainer *pInputContainer;
    OResponseString *pResponseString;
    Char displayBuffer[MAX_O_STRING_LENGTH];

    ASSERT_PARAM (pTaskReq != PNULL, (unsigned long) pTaskReq);
     
    printDebug ("Task Handler: HD Protocol Task received '%s', sending to Hindbrain.\n", removeCtrlCharacters (&(pTaskReq->string[0]), &(displayBuffer[0])));
    pInputContainer = malloc (sizeof (*pInputContainer));
    if (pInputContainer != PNULL)
    {
        UInt32 lengthInputString = 0;
        pInputContainer->waitForResponse = true;
        
        /* Stop overruns as we're using different buffer lengths here */
        lengthInputString = strlen (&(pTaskReq->string[0])) + 1; /* +1 for terminator */
        if (lengthInputString > sizeof (pInputContainer->string))
        {
            lengthInputString = sizeof (pInputContainer->string) - 1;
            pInputContainer->string[lengthInputString] = 0; /* Make sure that a terminator gets on the end as it would otherwise be chopped off */
        }
        
        memcpy (&(pInputContainer->string[0]), &(pTaskReq->string[0]), lengthInputString);
        
        /* Go send the string and wait for an "OK" answer */
        pResponseString = malloc (sizeof (*pResponseString));
        if (pResponseString != PNULL)
        {
            pResponseString->stringLength = sizeof (pResponseString->string);
            success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputContainer, sizeof (*pInputContainer), pResponseString);
            if (success)
            {
                printDebug ("Task Handler: '%s' response from Hindbrain.\n", removeCtrlCharacters (&(pResponseString->string[0]), &(displayBuffer[0])));
                if (!O_CHECK_OK_STRING (pResponseString))
                {
                    printDebug ("Task Handler: response indicated failure.\n");
                    success = false;
                }
            }
            else
            {            
                printDebug ("Task Handler: send failed.\n");
            }
            free (pResponseString);
        }
        free (pInputContainer);
    }
    
    return success;
}