/*
 * Public stuff to do with the menus.
 */ 

/*
 * MANIFEST CONSTANTS
 */

#define COMMAND_PROMPT "Command (? for help): "
#define GENERIC_FAILURE_MSG "Failed!"    /* Note no newlines on this as it is used in the dashboard where they get in the way */
#define READ_FAILURE_MSG "Read failure." /* Note no newlines on this as it is used in the dashboard where they get in the way */
#define MAX_NUM_CHARS_IN_COMMAND 4       /* includes a null terminator */

/*
 *  FUNCTION PROTOTYPES
 */

Bool getYesInput (WINDOW *pWin, Char *pPrompt);
Bool handleUserInputMenu (WINDOW *pWin, UInt8 key, Bool *pExitMenu, Char *pMatch);
Bool runMenu (void);
