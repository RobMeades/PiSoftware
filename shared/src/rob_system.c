/*
 * Shared.c
 * Shared library functions for my robot stuff on the Pi.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <rob_system.h>

/*
 * GLOBALS - prefixed with g
 */

static Bool gDebugPrintsAreOn = false;
static Bool gProgressPrintsAreOn = true;

/*
 * Assert function for debugging (should be called via the macros in rob_system.c).
 */
Bool assertFunc (const Char * pPlace, UInt32 line, const Char * pText, UInt32 param1)
{
    if (pText)
    {
	    printf ("\n!!! ASSERT: %s#%u:%s %lu!!!\n", pPlace, (int) line, pText, param1);
    }
    else
    {
	    printf ("\n!!! ASSERT: %s#%u: %lu!!!\n", pPlace, (int) line, param1);
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
}
/*
 * Progress printing function (that can be stubbed out if necessary)
 */
void printProgress (const Char * pFormat, ...)
{
    if (gProgressPrintsAreOn)
    {
        va_list args;
        va_start (args, pFormat);
        vprintf (pFormat, args);
        va_end (args);
    }
}

/*
 * Print debug
 */
void printDebug (const Char * pFormat, ...)
{
    if (gDebugPrintsAreOn)
    {
        va_list args;
        va_start (args, pFormat);
        vprintf (pFormat, args);
        va_end (args);
    }
}

/*
 * Dump hex from memory
 */
void printHexDump (const UInt8 * pMemory, UInt16 size)
{
    UInt8 i;
    
    if (gDebugPrintsAreOn)
    {
        for (i = 0; i < size; i +=8)
        {
            printf ("0x%.8lx: 0x%.2x 0x%.2x 0x%.2x 0x%.2x : 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", (unsigned long) pMemory, *pMemory, *(pMemory + 1), *(pMemory + 2), *(pMemory + 3), *(pMemory + 4), *(pMemory + 5), *(pMemory + 6), *(pMemory + 7));
            pMemory +=8;
        }
    }
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