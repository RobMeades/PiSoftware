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
#define ASSERT_ALWAYS_STRING(sTRING) ((assertFunc (__FUNCTION__, __LINE__, sTRING, false, 0, 0, 0)))
#define ASSERT_ALWAYS_PARAM(pARAM1) ((assertFunc (__FUNCTION__, __LINE__, PNULL, true, (pARAM1), 0, 0)))
#define ASSERT_ALWAYS_PARAM2(pARAM1,pARAM2) ((assertFunc (__FUNCTION__, __LINE__, PNULL, true, (pARAM1), (pARAM2), 0)))
#define ASSERT_ALWAYS_PARAM3(pARAM1,pARAM2,pARAM3) ((assertFunc (__FUNCTION__, __LINE__, PNULL, true, (pARAM1), (pARAM2), (pARAM3))))
#define ASSERT_STRING(cONDITION,sTRING) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, sTRING, false, 0)))
#define ASSERT_PARAM(cONDITION,pARAM1) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, PNULL, true, (pARAM1), 0, 0)))
#define ASSERT_PARAM2(cONDITION,pARAM1,pARAM2) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, PNULL, true, (pARAM1), (pARAM2), 0)))
#define ASSERT_PARAM3(cONDITION,pARAM1,pARAM2,pARAM3) ((cONDITION) ? true : (assertFunc (__FUNCTION__, __LINE__, PNULL, true, (pARAM1), (pARAM2), (pARAM3))))
#define BINARY_STRING_BUFFER_SIZE 9
#define UNUSED(x) (void)(x)

bool assertFunc (const Char * pPlace, UInt32 line, const Char * pText, Bool paramPresent, UInt32 param1, UInt32 param2, UInt32 param3);
void setProgressPrintsOn (void);
void setProgressPrintsOff (void);
void setDebugPrintsOn (void);
void setDebugPrintsOnToFile (Char * pFilename);
void setDebugPrintsOnToSyslog (void);
void setDebugPrintsOff (void);
void hexDumpsToSyslogOn (void);
void hexDumpsToSyslogOff (void);
void suspendDebug (void);
void resumeDebug (void);
void printProgress (const Char * pFormat, ...);
void printDebug (const Char * pFormat, ...);
void printHexDump (const void * pMemory, UInt16 size);
Char * binaryString (UInt8 value, Char *pString);
Char * removeCtrlCharacters (const Char *pInput, Char *pOutput);
UInt32 getSystemTicks (void);
