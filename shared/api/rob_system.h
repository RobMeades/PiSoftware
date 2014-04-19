/*
 *  Rob's system stuff
 */

#include <stdbool.h>

typedef bool Bool;
typedef char Char;
typedef signed char SInt8;
typedef unsigned char UInt8;
typedef signed short SInt16;
typedef unsigned short UInt16;
typedef signed long SInt32;
typedef unsigned long UInt32;

#define PNULL (void *) NULL
#define ASSERT_ALWAYS_STRING(sTRING) ((assertFunc (__FUNCTION__, __LINE__, sTRING, 0)))
#define ASSERT_ALWAYS_PARAM(pARAM1) ((assertFunc (__FUNCTION__, __LINE__, PNULL, (pARAM1))))
#define ASSERT_STRING(cONDITION,sTRING) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, sTRING, 0)))
#define ASSERT_PARAM(cONDITION,pARAM1) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, PNULL, (pARAM1))))

bool assertFunc (const Char * pPlace, UInt32 line, const Char * pText, UInt32 param1);
void setProgressPrintsOn (void);
void setProgressPrintsOff (void);
void setDebugPrintsOn (void);
void setDebugPrintsOnToFile (Char * pFilename);
void setDebugPrintsOff (void);
void suspendDebug (void);
void resumeDebug (void);
void printProgress (const Char * pFormat, ...);
void printDebug (const Char * pFormat, ...);
void printHexDump (const UInt8 * pMemory, UInt16 size);
UInt32 getSystemTicks (void);