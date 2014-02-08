/*
**  Rob's system stuff
*/

#include <stdbool.h>

#define PNULL (void *) NULL
#define ASSERT_ALWAYS_STRING(sTRING) ((assertFunc (__FUNCTION__, __LINE__, PSTR(sTRING), 0)))
#define ASSERT_ALWAYS_PARAM(pARAM1) ((assertFunc (__FUNCTION__, __LINE__, PNULL, (pARAM1))))
#define ASSERT_STRING(cONDITION,sTRING) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, PSTR(sTRING), 0)))
#define ASSERT_PARAM(cONDITION,pARAM1) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, PNULL, (pARAM1))))

bool assertFunc (const char * pPlace, int line, const char * pText, unsigned long param1);
void endStuff (void);