/*
 * Stuff to do with the menus.
 */ 

/*
 * MANIFEST CONSTANTS
 */

#define GENERIC_FAILURE_MSG "Failed!"    /* Note no newlines on this as it is used in the dashboard where they get in the way */
#define READ_FAILURE_MSG "Read failure." /* Note no newlines on this as it is used in the dashboard where they get in the way */
#define MAX_NUM_CHARS_IN_COMMAND 4       /* includes a null terminator */

/*
 *  FUNCTION PROTOTYPES
 */

Bool getYesInput (WINDOW *pWin, Char *pPrompt);
Char * getStringInput (WINDOW *pWin, Char *pPrompt, Char * pString, UInt32 stringLen);
Bool handleUserCmdMenu (WINDOW *pCmdWin, UInt8 key, WINDOW *pOutputWin, Bool *pExitMenu);
Bool runMenu (void);
