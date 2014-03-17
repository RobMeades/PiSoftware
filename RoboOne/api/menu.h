/*
 * Public stuff to do with the menus.
 */ 

/*
 * MANIFEST CONSTANTS
 */

#define GENERIC_FAILURE_MSG "Failed!"    /* Note no newlines on this as it is used in the dashboard where they get in the way */
#define READ_FAILURE_MSG "Read failure." /* Note no newlines on this as it is used in the dashboard where they get in the way */

/*
 *  FUNCTION PROTOTYPES
 */

Bool getYesInput (Char *pPrompt);
Bool runMenu (void);
