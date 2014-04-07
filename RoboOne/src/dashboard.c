#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <rob_system.h>
#include <curses.h> /* Has to be ahead of rob_system.h in the list as it fiddles with bool */
#include <ow_bus.h>
#include <menu.h>

/*
 * MANIFEST CONSTANTS
 */

/* The shape of the windows.
 * Note that this is designed for a 55 row long terminal window. */
/* The actual space used by a window doesn't include the heading,
 * that is added on the line above (so that if the window is
 * a scrolling one the heading doesn't disappear with the scroll),
 * so NEVER start a window at 0, only 1 or more */
#define SCR_HEIGHT                55
#define SCR_WIDTH                 80
#define BORDER_WIDTH              1
#define HEADING_HEIGHT            1
#define WIN_RIO_START_ROW         4
#define WIN_RIO_START_COL         2
#define WIN_RIO_HEIGHT            5
#define WIN_RIO_WIDTH             30
#define WIN_O_START_ROW           4
#define WIN_O_START_COL           35
#define WIN_O_HEIGHT              5
#define WIN_O_WIDTH               42
#define WIN_MUX_START_ROW         9
#define WIN_MUX_START_COL         2
#define WIN_MUX_HEIGHT            1
#define WIN_MUX_WIDTH             70
#define WIN_CHG_START_ROW         12
#define WIN_CHG_START_COL         2
#define WIN_CHG_HEIGHT            2
#define WIN_CHG_WIDTH             25
#define WIN_CMD_START_ROW         11 /* Starts a row higher as there is no heading */
#define WIN_CMD_START_COL         28
#define WIN_CMD_HEIGHT            2
#define WIN_CMD_WIDTH             25
#define WIN_OUTPUT_START_ROW      16
#define WIN_OUTPUT_START_COL      2
#define WIN_OUTPUT_HEIGHT         35
#define WIN_OUTPUT_WIDTH          76

/* The places that various things should appear on the background
 * screen, negative meaning to count up from the bottom of the
 * screen */
#define ROW_HEADING                0
#define COL_HEADING                0

/* Cursor manipulation macros */
#define SET_ROW(rOW) ((rOW) >= 0 ? (topRow + rOW) : (botRow + rOW))    /* Only use this macro for things on the background screen, not in a window */
#define SET_COL(cOL) ((cOL) >= 0 ? (leftCol + cOL) : (rightCol + cOL)) /* Only use this macro for things on the background screen, not in a window */
#define NUM_SPACES_TO_CENTRE_STRING(pStr, sCRcOLS) ((scrCols) >= strlen (pStr) ? (((sCRcOLS) - strlen (pStr)) / 2) : 0)

/* The strings */
#define STRING_MAIN_HEADING "RoboOne Dashboard"
#define COMMAND_PROMPT "Command (? for help): "
 
/* Misc stuff */
#define MAX_NUM_CHARS_IN_ROW  SCR_WIDTH
#define SLOW_UPDATE_BACKOFF   10
#define SLOWER_UPDATE_BACKOFF 30

/*
 * STATIC FUNCTION PROTOTYPES
 */
static void initOutputWindow (WINDOW *pWin);
static Bool updateOutputWindow (WINDOW *pWin, UInt8 count);
static void initCmdWindow (WINDOW *pWin);
static Bool updateCmdWindow (WINDOW *pWin, UInt8 count);
static void initRioWindow (WINDOW *pWin);
static Bool updateRioWindow (WINDOW *pWin, UInt8 count);
static void initOWindow (WINDOW *pWin);
static Bool updateOWindow (WINDOW *pWin, UInt8 count);
static void initMuxWindow (WINDOW *pWin);
static Bool updateMuxWindow (WINDOW *pWin, UInt8 count);
static void initChgWindow (WINDOW *pWin);
static Bool updateChgWindow (WINDOW *pWin, UInt8 count);

/*
 * TYPES
 */

/* Struct to hold window information */
typedef struct WindowDimensionsTag
{
    UInt8 height;
    UInt8 width;
    UInt8 startRow;
    UInt8 startCol;
} WindowDimensions;

/* Struct to hold window information*/
typedef struct WindowInfoTag
{
    Char windowHeading[MAX_NUM_CHARS_IN_ROW];
    WindowDimensions dimensions;
    void (*pWinInit) (WINDOW *);
    Bool (*pWinUpdate) (WINDOW *, UInt8);
    WINDOW *pWin;
    Bool enabled;
} WindowInfo;

/*
 * GLOBALS (prefixed with g)
 */

/* Array of windows that are always displayed */
WindowInfo gWindowList[] = {{"Output", {WIN_OUTPUT_HEIGHT, WIN_OUTPUT_WIDTH, WIN_OUTPUT_START_ROW, WIN_OUTPUT_START_COL}, initOutputWindow, updateOutputWindow, PNULL, true}, /* MUST be first in the list for the global pointer just below */
                            {"Pi/Rio", {WIN_RIO_HEIGHT, WIN_RIO_WIDTH, WIN_RIO_START_ROW, WIN_RIO_START_COL}, initRioWindow, updateRioWindow, PNULL, true},
                            {"Hindbrain", {WIN_O_HEIGHT, WIN_O_WIDTH, WIN_O_START_ROW, WIN_O_START_COL}, initOWindow, updateOWindow, PNULL, true},
                            {"Analogue", {WIN_MUX_HEIGHT, WIN_MUX_WIDTH, WIN_MUX_START_ROW, WIN_MUX_START_COL}, initMuxWindow, updateMuxWindow, PNULL, false},
                            {"Chargers", {WIN_CHG_HEIGHT, WIN_CHG_WIDTH, WIN_CHG_START_ROW, WIN_CHG_START_COL}, initChgWindow, updateChgWindow, PNULL, true},
                            {"", {WIN_CMD_HEIGHT, WIN_CMD_WIDTH, WIN_CMD_START_ROW, WIN_CMD_START_COL}, initCmdWindow, updateCmdWindow, PNULL, true}}; /* Should be last in the list so that display updates leave the cursor here */
/* Must be in the same order as the enum ChargeState */
Char * gChargeStrings[] = {" Off ", " Grn ", "*Grn*", " Red ", "*Red*", "  5  ", " Nul ", " Bad "};
WINDOW **gpOutputWindow = &(gWindowList[0].pWin);

/*
 * STATIC FUNCTIONS
 */

/*
 * Init function for Rio window.
 * 
 * pWin    the window where the Rio stuff
 *         is to be displayed.
 */
static void initRioWindow (WINDOW *pWin)
{
}

/*
 * Display the status of the Rio related
 * stuff.
 * 
 * pWin    the window where the Rio stuff
 *         can be displayed.
 * count   the number of times this function
 *         has been called.  This is used to
 *         update some items more often than
 *         others.
 * 
 * @return  always false, meaning "not finished".
 */
static Bool updateRioWindow (WINDOW *pWin, UInt8 count)
{
    SInt16 current = 0;
    UInt16 voltage = 0;
    UInt16 remainingCapacity = 0;
    UInt32 lifetimeCharge = 0;
    UInt32 lifetimeDischarge = 0;
    UInt8 row = 0;
    UInt8 col = 0;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    readRioBattCurrent (&current);
    readRioBattVoltage (&voltage);
    wmove (pWin, row, col);
    wclrtoeol (pWin);
    wprintw (pWin, "%d mA, %u mV", current, voltage);
    row++;

    if (count % SLOW_UPDATE_BACKOFF == 0)
    {
        readRioRemainingCapacity (&remainingCapacity);
        readRioBattLifetimeChargeDischarge (&lifetimeCharge, &lifetimeDischarge);
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "%u mAhr(s) remain", remainingCapacity);
        row++;
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "lifetime -%lu/%lu mAhr", lifetimeDischarge, lifetimeCharge);
        row++;
    }
    else
    {
        row += 2;
    }
    
    wnoutrefresh (pWin);
    
    return false;
}

/*
 * Init function for Orangutan window.
 * 
 * pWin    the window where the Orangutan
 *         stuff is to be displayed.
 */
static void initOWindow (WINDOW *pWin)
{
}

/*
 * Display the status of the Orangutan related
 * stuff.
 * 
 * pWin    the window where the Orangutan stuff
 *         can be displayed.
 * count   the number of times this function
 *         has been called.  This is used to
 *         update some items more often than
 *         others.
 * 
 * @return  always false, meaning "not finished".
 */
static Bool updateOWindow (WINDOW *pWin, UInt8 count)
{
    SInt16 current[3];
    UInt16 voltage[3];
    UInt16 remainingCapacity[3];
    UInt32 lifetimeCharge[3];
    UInt32 lifetimeDischarge[3];
    UInt8 row = 0;
    UInt8 col = 0;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);
    
    readO1BattCurrent (&current[0]);
    readO2BattCurrent (&current[1]);
    readO3BattCurrent (&current[2]);
    readO1BattVoltage (&voltage[0]);
    readO2BattVoltage (&voltage[1]);
    readO3BattVoltage (&voltage[2]);
    wmove (pWin, row, col);
    wclrtoeol (pWin);
    wprintw (pWin, "%d/%d/%d mA, %u/%u/%u mV", current[0], current[1], current[2], voltage[0], voltage[1], voltage[2]);
    row++;
    
    if (count % SLOWER_UPDATE_BACKOFF == 0)
    {
        readO1RemainingCapacity (&remainingCapacity[0]);
        readO2RemainingCapacity (&remainingCapacity[1]);
        readO3RemainingCapacity (&remainingCapacity[2]);
        readO1BattLifetimeChargeDischarge (&lifetimeCharge[0], &lifetimeDischarge[0]);
        readO2BattLifetimeChargeDischarge (&lifetimeCharge[1], &lifetimeDischarge[1]);
        readO3BattLifetimeChargeDischarge (&lifetimeCharge[2], &lifetimeDischarge[2]);
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "%u mAhr(s) remain", remainingCapacity[0] + remainingCapacity[1] + remainingCapacity[2]);
        row++;
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "lifetime -%lu/%lu mAhr", lifetimeDischarge[0] + lifetimeDischarge[1] + lifetimeDischarge[2], lifetimeCharge[0] + lifetimeCharge[1] + lifetimeCharge[2]);
        row++;
    }
    else
    {
        row += 2;
    }
    
    wnoutrefresh (pWin);
    
    return false;
}

/*
 * Init function for analogue mux window.
 * 
 * pWin    the window where the analogue
 *         mux stuff is to be displayed.
 */
static void initMuxWindow (WINDOW *pWin)
{
}

/*
 * Display the status of the analogue mux related
 * stuff.
 * 
 * pWin    the window where the analogue mux stuff
 *         can be displayed.
 * count   the number of times this function
 *         has been called.  This is used to
 *         update some items more often than
 *         others.
 * 
 * @return  always false, meaning "not finished".
 */
static Bool updateMuxWindow (WINDOW *pWin, UInt8 count)
{
    UInt16 voltage = 0;
    UInt8 row = 0;
    UInt8 col = 0;
    UInt8 i;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    if (count % SLOWER_UPDATE_BACKOFF == 0)
    {
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        for (i = 0; i < 8; i++)
        {
            setAnalogueMuxInput (i);
            readAnalogueMux (&voltage);
            wprintw (pWin, "%d: %u ", i, voltage);
        }
        wprintw (pWin, "mV");
        row++;
    }
    else
    {
        row++;
    }

    wnoutrefresh (pWin);
    
    return false;
}

/*
 * Init function for the charger window.
 * 
 * pWin    the window where the charger stuff
 *         is to be displayed.
 */
static void initChgWindow (WINDOW *pWin)
{
}

/*
 * Display the status of the chargers.  This
 * function only updates the menu if it is
 * called at the right interval to pick up
 * the flashing state of the LEDs.
 * 
 * pWin    the window where the charger stuff
 *         can be displayed.
 * count   the number of times this function
 *         has been called.  This is used to
 *         update some items more often than
 *         others.
 * 
 * @return  always false, meaning "not finished".
 */
static Bool updateChgWindow (WINDOW *pWin, UInt8 count)
{
    Bool success;
    ChargeState state[NUM_CHARGERS];
    Bool flashDetectPossible;
#ifndef ROBOONE_1_0
    Bool mains12VPresent;
#endif    
    UInt8 row = 0;
    UInt8 col = 0;
    UInt8 i;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    wmove (pWin, row, col);
    wclrtoeol (pWin);        
    wprintw (pWin, "  Pi   O1   O2   O3");
#ifndef ROBOONE_1_0
    wprintw (pWin, "  12V");
#endif    
    row++;
    success = readChargerState (&state[0], &flashDetectPossible);
    if (success && flashDetectPossible)
    {
        wmove (pWin, row, col);
        wclrtoeol (pWin);        
        for (i = 0; i < NUM_CHARGERS; i++)
        {
            wprintw (pWin, "%s", gChargeStrings[state[i]]);
        }
#ifndef ROBOONE_1_0
        success = readMains12VPin (&mains12VPresent);
        if (success)
        {
            if (mains12VPresent)
            {
                wprintw (pWin, "  Y");
            }
            else
            {
                wprintw (pWin, "  N");                
            }
        }
        else
        {
            wprintw (pWin, "  ?");            
        }
#endif
    }
    row++;
    
    wnoutrefresh (pWin);
    
    return false;
}

/*
 * Init function for the command window.
 * 
 * pWin    the window where the command stuff
 *         is to be displayed.
 */
static void initCmdWindow (WINDOW *pWin)
{
    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);
    
    mvwprintw (pWin, 0, 0, COMMAND_PROMPT);    
    wnoutrefresh (pWin);
}

/*
 * Display the command window.
 * 
 * pWin     the window where the command prompt
 *          can be displayed.
 * count    the number of times this function
 *          has been called.  This is used to
 *          update some items more often than
 *          others.
 * 
 * @return  true if the exit key has been 
 *          pressed, otherwise false.
 */
static Bool updateCmdWindow (WINDOW *pWin, UInt8 count)
{
    Bool commandExecuted;
    Bool exitMenu = false;
    UInt8 key;
    
    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);
    ASSERT_PARAM (*gpOutputWindow != PNULL, (unsigned long) *gpOutputWindow);
    
    /* Check for key presses */
    key = getch();
    commandExecuted = handleUserCmdMenu (pWin, key, *gpOutputWindow, &exitMenu);
    /* Clear out the previous command if done */
    if (commandExecuted)
    {
        wmove (pWin, 0, 0);
        wclrtoeol (pWin);        
        wprintw (pWin, "%s", COMMAND_PROMPT);
    }
    wnoutrefresh (*gpOutputWindow);
    wnoutrefresh (pWin);
    
    return exitMenu;
}

/*
 * Init function for the output window.
 * 
 * pWin    the window where the output stuff
 *         is to be displayed.
 */
static void initOutputWindow (WINDOW *pWin)
{
    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    scrollok (pWin, true);
    wnoutrefresh (pWin);
}

/*
 * Display the output window.  This is a
 * dummy function as, for the output window,
 * things are updated as necessary by the
 * command window.
 * 
 * pWin     the window where the output from
 *          the command prompt can be displayed.
 * count    the number of times this function
 *          has been called.  This is used to
 *          update some items more often than
 *          others.
 * 
 * @return  always false, meaning "not finished".
 */
static Bool updateOutputWindow (WINDOW *pWin, UInt8 count)
{
    return false;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Display a dashboard of useful information.
 *
 * @return  true if successful, otherwise false.
 */
Bool runDashboard (void)
{
    Bool success = true;
    Bool exitDashboard = false;
    UInt8 leftCol;
    UInt8 rightCol;
    UInt8 topRow;
    UInt8 botRow;
    UInt8 scrCols;
    UInt8 i;
    UInt8 x;
    int savedCursor;

    /* Set up curses for unbuffered input with no delay */
    initscr();
    cbreak();
    noecho();
    nodelay (stdscr, TRUE);
    
    /* Set the cursor to invisible, if possible */
    savedCursor = curs_set (0);

    /* Setup the boundaries */
    getbegyx (stdscr, topRow, leftCol);
    getmaxyx (stdscr, botRow, rightCol);
    topRow += BORDER_WIDTH;
    botRow -= BORDER_WIDTH;
    leftCol += BORDER_WIDTH;
    rightCol -= BORDER_WIDTH;
    scrCols = rightCol - leftCol;
    
    /* Bound the main screen */
    mvprintw (SET_ROW (ROW_HEADING), SET_COL (leftCol + NUM_SPACES_TO_CENTRE_STRING (STRING_MAIN_HEADING, scrCols)), STRING_MAIN_HEADING);
    box (stdscr, 0, 0);
    refresh();
    
    /* Start sub-windows for the status reports, print their headings, call their init functions */
    for (i = 0; (i < (sizeof (gWindowList) / sizeof (WindowInfo))) && success; i++)
    {
        if (gWindowList[i].enabled)
        {
            gWindowList[i].pWin = newwin (gWindowList[i].dimensions.height, gWindowList[i].dimensions.width, gWindowList[i].dimensions.startRow, gWindowList[i].dimensions.startCol);
            if (gWindowList[i].pWin)
            {
                if (strlen(gWindowList[i].windowHeading) > 0)
                {
                    mvwprintw (stdscr, gWindowList[i].dimensions.startRow - HEADING_HEIGHT, gWindowList[i].dimensions.startCol, "%s:", gWindowList[i].windowHeading);
                }
                gWindowList[i].pWinInit(gWindowList[i].pWin);
            }
            else
            {
                success = false;
            }
        }
    }
    
    if (success)
    {
        /* Now show stuff in the sub-windows */
        for (i = 0; !exitDashboard; i++) /* i is not meant to be in the condition here */
        {
            for (x = 0; (x < (sizeof (gWindowList) / sizeof (gWindowList[0])) && !exitDashboard); x++)
            {
                if (gWindowList[x].enabled)
                {
                    exitDashboard = gWindowList[x].pWinUpdate (gWindowList[x].pWin, i);
                }
            }
            doupdate();
        }        
    }
    
    /* clean up */
    echo(); /* endwin() doesn't seem to turn echo back on for me, so call this first */
    if (savedCursor != ERR)
    {
        curs_set (savedCursor);
    }
    for (i = 0; i < sizeof (gWindowList) / sizeof (WindowInfo); i++)
    {
        if (gWindowList[i].pWin != PNULL)
        {
            delwin (gWindowList[i].pWin);
            gWindowList[i].pWin = PNULL;
        }
    }
    endwin();
    fflush (stdout);

    return success;    
}