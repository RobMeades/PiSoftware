/*
 * Shared.c
 * Shared library functions for my robot stuff on the Pi.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <rob_system.h>

/*
 * GLOBALS - prefixed with g
 */

static Bool gDebugPrintsAreOn = false;
static FILE *pgDebugPrintsStream = PNULL;
static Bool gProgressPrintsAreOn = true;
static Bool gSuspendDebug = false;
/* Mutex to ensure no loss of printf()s if called from multiple threads */
pthread_mutex_t lockPrintf;

/*
 * Assert function for debugging (should be called via the macros in rob_system.c).
 */
Bool assertFunc (const Char * pPlace, UInt32 line, const Char * pText, UInt32 param1)
{
    Char * pFormat = PNULL;
    
    if (pText)
    {
        pFormat = "\n!!! ASSERT: %s#%u:%s %lu!!!\n";
	    printf (pFormat, pPlace, (int) line, pText, param1);
    }
    else
    {
        pFormat = "\n!!! ASSERT: %s#%u: %lu!!!\n";
	    printf (pFormat, pPlace, (int) line, param1);
    }
    
    /* If debug is printing to file, put the assert there also */
    if (gDebugPrintsAreOn && (pgDebugPrintsStream != PNULL))
    {
        pthread_mutex_lock (&lockPrintf);
        /* But remove any initial line feeds for neatness */
        if (*pFormat == '\n')
        {
            pFormat++;
        }
        fprintf (pgDebugPrintsStream, "%.10lu: ", getSystemTicks());
        
        if (pText)
        {
            fprintf (pgDebugPrintsStream, pFormat, pPlace, (int) line, pText, param1);
        }
        else
        {
            fprintf (pgDebugPrintsStream, pFormat, pPlace, (int) line, param1);
        }
        fflush (pgDebugPrintsStream);
        fclose (pgDebugPrintsStream);
        
        pthread_mutex_unlock (&lockPrintf);
        pthread_mutex_destroy (&lockPrintf);

    }

    fflush (stdout);
    exit (127);

    return false;
}

/*
 * Set progress prints on
 */
void setProgressPrintsOn (void)
{
    gProgressPrintsAreOn = true;
}

/*
 * Set progress prints on or off
 */
void setProgressPrintsOff (void)
{
    gProgressPrintsAreOn = false;
}

/*
 * Set debug prints on
 */
void setDebugPrintsOn (void)
{
    gDebugPrintsAreOn = true;
}

/*
 * Set debug prints off
 */
void setDebugPrintsOff (void)
{
    gDebugPrintsAreOn = false;
    if (pgDebugPrintsStream != PNULL)
    {
        time_t timeNow = time (NULL);
        printDebug ("Stopped debug prints on %s", asctime (localtime (&timeNow))); 
        fclose (pgDebugPrintsStream);
        pgDebugPrintsStream = PNULL;
        pthread_mutex_destroy (&lockPrintf);
    }
}

/*
 * Suspend debug
 */
void suspendDebug (void)
{
    gSuspendDebug = true;
}

/*
 * Resume debug
 */
void resumeDebug (void)
{
    gSuspendDebug = false;
}

/*
 * Set debug prints on and to file
 */
void setDebugPrintsOnToFile (Char * pFilename)
{
    ASSERT_PARAM (pFilename != PNULL, (unsigned long) pFilename);
    
    if (pgDebugPrintsStream != PNULL)
    {
        fclose (pgDebugPrintsStream);        
    }
    
    pgDebugPrintsStream = fopen (pFilename, "w"); 
  
    if (pgDebugPrintsStream != PNULL)
    {
        time_t timeNow = time (NULL);
        
        /* Create the mutex */
        if (pthread_mutex_init (&lockPrintf, NULL) != 0)
        {
            ASSERT_ALWAYS_STRING ("setDebugPrintsOnToFile: failed pthread_mutex_init().");
        }
        
        gDebugPrintsAreOn = true;
        printDebug ("Started debug prints on %s", asctime (localtime (&timeNow))); 
    }
}

/*
 * Progress printing function
 */
void printProgress (const Char * pFormat, ...)
{
    if (gProgressPrintsAreOn)
    {
        va_list args;
        
        pthread_mutex_lock (&lockPrintf);

        va_start (args, pFormat);
        vprintf (pFormat, args);
        
        /* If debug is printing to file, put the progress prints there also */
        if (gDebugPrintsAreOn && !gSuspendDebug && (pgDebugPrintsStream != PNULL))
        {
            /* But remove any initial line feeds for neatness */
            if (*pFormat == '\n')
            {
                pFormat++;
            }
            fprintf (pgDebugPrintsStream, "%.10lu: ", getSystemTicks());
            vfprintf (pgDebugPrintsStream, pFormat, args);
        }
        va_end (args);
        fflush (stdout);
        
        pthread_mutex_unlock (&lockPrintf);
    }
}

/*
 * Print debug
 */
void printDebug (const Char * pFormat, ...)
{
    FILE *pStream = stdout;
    
    if (gDebugPrintsAreOn && !gSuspendDebug)
    {
        va_list args;

        pthread_mutex_lock (&lockPrintf);
        
        if (pgDebugPrintsStream != PNULL)
        {
            pStream = pgDebugPrintsStream;
        }
        va_start (args, pFormat);
        fprintf (pStream, "%.10lu: ", getSystemTicks());
        vfprintf (pStream, pFormat, args);
        va_end (args);
        fflush (pStream);
        
        pthread_mutex_unlock (&lockPrintf);
    }
}

/*
 * Dump hex from memory
 */
void printHexDump (const void * pMemory, UInt16 size)
{
    UInt8 i;
    UInt32 time;
    FILE *pStream = stdout;
    const UInt8 *pPrint = pMemory;
    
    if (gDebugPrintsAreOn && !gSuspendDebug)
    {
        pthread_mutex_lock (&lockPrintf);

        time = getSystemTicks();
        if (pgDebugPrintsStream != PNULL)
        {
            pStream = pgDebugPrintsStream;
        }

        fprintf (pStream, "%.10lu: ", time);
        fprintf (pStream, "Printing at least %d bytes:\n", size);
        for (i = 0; i < size; i +=8)
        {
            fprintf (pStream, "%.10lu: 0x%.8lx: 0x%.2x 0x%.2x 0x%.2x 0x%.2x : 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", time, (unsigned long) pPrint, *pPrint, *(pPrint + 1), *(pPrint + 2), *(pPrint + 3), *(pPrint + 4), *(pPrint + 5), *(pPrint + 6), *(pPrint + 7));
            pPrint +=8;
        }
        fflush (pStream);

        pthread_mutex_unlock (&lockPrintf);
    }
}

/*
 * Return a binary string representation of
 * a value.
 * 
 * value   the value to be represented as binary.
 * pString storage for BINARY_STRING_BUFFER_SIZE
 *         of null terminated string.
 * 
 * return  pString.
 */
Char * binaryString (UInt8 value, Char *pString)
{
    UInt8 i;
    UInt8 mask = 0x80;
    
    ASSERT_PARAM (pString != PNULL, (unsigned long) pString);
    
    for (i = 0; i < 8; i++)
    {
        *(pString + i) = '0';
        if ((value & mask) > 0)
        {
            *(pString + i) = '1';
        }
        mask >>= 1;
    }
    
    *(pString + i) = 0; /* Add the terminator */
    
    return pString;
}

/*
 * Remove control characters from a null
 * terminated string.
 * 
 * pInput   the null terminated input string.
 * pOutput  pointer to somewhere to store the
 *          filtered string, needs to be
 *          at least as large as pInput. The
 *          null terminator is copied.
 * 
 * @return  pOutput.
 */
Char * removeCtrlCharacters (const Char *pInput, Char *pOutput)
{
    UInt32 i;
    UInt32 inputLength;
    Char *pDest;
    const Char *pSrc;
    
    ASSERT_PARAM (pInput != PNULL, (unsigned long) pInput);
    ASSERT_PARAM (pOutput != PNULL, (unsigned long) pOutput);
    
    inputLength = strlen (pInput);
    
    pSrc = pInput;
    pDest = pOutput;
    for (i = 0; i < inputLength; i++)
    {
        if (isprint (*pSrc))
        {
            *pDest = *pSrc;
            pDest++;
        }
        pSrc++;
    }
    *pDest = 0; /* Add the null terminator */
    
    return pOutput;
}

/*
 * Get the system time in ticks and in a
 * UInt32.
 * 
 * @return  the system ticks as a UInt32,
 *          which will be zero if the
 *          call fails.
 */
UInt32 getSystemTicks (void)
{
    struct timespec time;

    if (clock_gettime (CLOCK_REALTIME, &time) != 0)
    {
        /* If the call fails, set the time returned to zero */
        time.tv_sec = 0;
    }
    
    return (UInt32) time.tv_sec;
}