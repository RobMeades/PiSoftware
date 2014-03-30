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
 * Progress printing function (that can be stubbed out afterwards)
 */
void printProgress (const Char * pFormat, ...)
{
    va_list args;
    va_start (args, pFormat);
    vprintf (pFormat, args);
    va_end (args);
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