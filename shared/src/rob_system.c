/*
** Shared.c
** Shared library functions for my robot stuff on the Pi.
*/

#include <stdio.h>
#include <rob_system.h>

/*
** Assert function for debugging (should be called via the macros in rob_system.c).
*/
bool assertFunc (const char * pPlace, int line, const char * pText, unsigned long param1)
{
    if (pText)
    {
	    printf ("%s#%u:%s %lu", pPlace, line, pText, param1);
    }
    else
    {
	    printf ("%s#%d: %lu", pPlace, line, param1);
    }

    while (1);

    return false;
}

/*
** Progress printing function (that can be stubbed out afterwards)
*/
void printProgress (const char * pText)
{
    printf ("%s", pText);
}