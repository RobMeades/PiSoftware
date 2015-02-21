/*
 * Handle the execution of tasks going straight to the Hindbrain.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rob_system.h>
#include <hardware_types.h>
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
 * pHDTaskReq  pointer to the Hindbrain Direct task
 *             to be handled.
 * pHDTaskInd  pointer a place to put the response.
 * 
 * @return     result code from RoboOneHDResult.
 */
RoboOneHDResult handleHDTaskReq (RoboOneHDTaskReq *pHDTaskReq, RoboOneHDTaskInd *pHDTaskInd)
{
    RoboOneHDResult result = HD_RESULT_GENERAL_FAILURE;
    OInputContainer *pInputContainer;
    OResponseString *pResponseString;
    Char displayBuffer[MAX_O_STRING_LENGTH];

    ASSERT_PARAM (pHDTaskReq != PNULL, (unsigned long) pHDTaskReq);
    ASSERT_PARAM (pHDTaskInd != PNULL, (unsigned long) pHDTaskInd);
     
    pHDTaskInd->string[0] = 0; /* Put in a terminator just in case we fail */

    printDebug ("Task Handler: HD Protocol Task received '%s', sending to Hindbrain.\n", removeCtrlCharacters (&(pHDTaskReq->string[0]), &(displayBuffer[0])));
    pInputContainer = malloc (sizeof (*pInputContainer));
    if (pInputContainer != PNULL)
    {
        UInt32 lengthInputString = 0;
        pInputContainer->waitForResponse = true;
        
        /* Stop overruns as we're using different buffer lengths here */
        lengthInputString = strlen (&(pHDTaskReq->string[0])) + 1; /* +1 for terminator */
        if (lengthInputString > sizeof (pInputContainer->string))
        {
            lengthInputString = sizeof (pInputContainer->string) - 1;
            pInputContainer->string[lengthInputString] = 0; /* Make sure that a terminator gets on the end as it would otherwise be chopped off */
        }
        
        memcpy (&(pInputContainer->string[0]), &(pHDTaskReq->string[0]), lengthInputString);
        
        /* Go send the string and wait for an answer */
        pResponseString = malloc (sizeof (*pResponseString));
        if (pResponseString != PNULL)
        {
            pResponseString->stringLength = sizeof (pResponseString->string);
            result = HD_RESULT_SEND_FAILURE;
            
            if (hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputContainer, sizeof (*pInputContainer), pResponseString))
            {
                /* Prevent overruns */
                if (pResponseString->stringLength > sizeof (pHDTaskInd->string))
                {
                    pResponseString->stringLength = sizeof (pHDTaskInd->string) - 1; /* - 1 to ensure a terminator */
                    pResponseString->string[pResponseString->stringLength] = 0;
                }
                
                /* Copy the answer into the indication to send back */
                memcpy (&(pHDTaskInd->string[0]), &(pResponseString->string[0]), pResponseString->stringLength);
                pHDTaskInd->stringLength = pResponseString->stringLength;
                result = HD_RESULT_SUCCESS;
                printDebug ("Task Handler: '%s' response from Hindbrain.\n", removeCtrlCharacters (&(pResponseString->string[0]), &(displayBuffer[0])));
            }
            else
            {            
                printDebug ("Task Handler: send failed.\n");
            }
            free (pResponseString);
        }
        free (pInputContainer);
    }
    
    return result;
}