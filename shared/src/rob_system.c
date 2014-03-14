/*
 * Shared.c
 * Shared library functions for my robot stuff on the Pi.
 */

#include <stdio.h>
#include <stdarg.h>
#include <rob_system.h>

/*
 * Assert function for debugging (should be called via the macros in rob_system.c).
 */
Bool assertFunc (const Char * pPlace, UInt32 line, const Char * pText, UInt32 param1)
{
    if (pText)
    {
	    printf ("%s#%u:%s %lu", pPlace, (int) line, pText, param1);
    }
    else
    {
	    printf ("%s#%d: %lu", pPlace, (int) line, param1);
    }

    while (1);

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