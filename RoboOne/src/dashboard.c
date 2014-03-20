#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <rob_system.h>
#include <ow_bus.h>
#include <menu.h>
#include <curses.h> /* Has to be ahead of rob_system.h in the list as it fiddles with bool */

/*
 * MANIFEST CONSTANTS
 */

/* The shape of the windows */
#define BORDER_WIDTH              1
#define WIN_RIO_HEIGHT            9
#define WIN_RIO_WIDTH             30
#define WIN_RIO_START_ROW         3
#define WIN_RIO_START_COL         2
#define WIN_O_HEIGHT              8
#define WIN_O_WIDTH               42
#define WIN_O_START_ROW           3
#define WIN_O_START_COL           35
#define WIN_MUX_HEIGHT            4
#define WIN_MUX_WIDTH             70
#define WIN_MUX_START_ROW         10
#define WIN_MUX_START_COL         2
#define WIN_CHG_HEIGHT           4
#define WIN_CHG_WIDTH            70
#define WIN_CHG_START_ROW        15
#define WIN_CHG_START_COL        2

/* The places that various things should appear on the background
 * screen, negative meaning to count up from the bottom of the
 * screen */
#define ROW_HEADING                0
#define COL_HEADING                1
#define ROW_FOOTER                -2

/* Cursor manipulation macros */
#define SET_ROW(rOW) ((rOW) >= 0 ? (topRow + rOW) : (botRow + rOW))    /* Only use this macro for things on the background screen, not in a window */
#define SET_COL(cOL) ((cOL) >= 0 ? (leftCol + cOL) : (rightCol + cOL)) /* Only use this macro for things on the background screen, not in a window */
#define NUM_SPACES_TO_CENTRE_STRING(pStr, sCRcOLS) ((scrCols) >= strlen (pStr) ? (((sCRcOLS) - strlen (pStr)) / 2) : 0)

/* The strings */
#define STRING_MAIN_HEADING "RoboOne Dashboard"
#define STRING_MAIN_FOOTER "Press any key to exit"
 
/* Misc stuff */
#define MAX_NUM_CHARS_IN_ROW      80
#define SLOW_UPDATE_BACKOFF   10
#define SLOWER_UPDATE_BACKOFF 30

/*
 * STATIC FUNCTION PROTOTYPES
 */
static void updateRioWindow (WINDOW * pWin, UInt8 count);
static void updateOWindow (WINDOW * pWin, UInt8 count);
static void updateMuxWindow (WINDOW * pWin, UInt8 count);
static void updateChgWindow (WINDOW * pWin, UInt8 count);

/*
 * TYPES
 */

/* Struct to hold window information*/
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
    UInt8 windowHeading[MAX_NUM_CHARS_IN_ROW];
    WindowDimensions dimensions;
    void (*winUpdatePtr) (WINDOW *, UInt8);
    WINDOW *pWin;
    Bool enabled;
} WindowInfo;

/*
 * GLOBALS (prefixed with g)
 */

/* Array of possible windows */
WindowInfo gWindowList[] = {{"Pi/Rio", {WIN_RIO_HEIGHT, WIN_RIO_WIDTH, WIN_RIO_START_ROW, WIN_RIO_START_COL}, updateRioWindow, PNULL, true},
                            {"Hindbrain", {WIN_O_HEIGHT, WIN_O_WIDTH, WIN_O_START_ROW, WIN_O_START_COL}, updateOWindow, PNULL, true},
                            {"Analogue", {WIN_MUX_HEIGHT, WIN_MUX_WIDTH, WIN_MUX_START_ROW, WIN_MUX_START_COL}, updateMuxWindow, PNULL, false},
                            {"Chargers", {WIN_CHG_HEIGHT, WIN_CHG_WIDTH, WIN_CHG_START_ROW, WIN_CHG_START_COL}, updateChgWindow, PNULL, true}};
/* Must be in the same order as the enum ChargeState */
Char * gChargeStrings[] = {" Off ", " Grn ", "*Grn*", " Red ", "*Red*", "  5  ", " Nul ", " Bad "};

/*
 * STATIC FUNCTIONS
 */

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
 * @return  none.
 */
static void updateRioWindow (WINDOW * pWin, UInt8 count)
{
    SInt16 current = 0;
    UInt16 voltage = 0;
    UInt16 remainingCapacity = 0;
    UInt32 lifetimeCharge = 0;
    UInt32 lifetimeDischarge = 0;
    UInt8 row = 1;
    UInt8 col = 1;

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
 * @return  none.
 */
static void updateOWindow (WINDOW * pWin, UInt8 count)
{
    SInt16 current[3];
    UInt16 voltage[3];
    UInt16 remainingCapacity[3];
    UInt32 lifetimeCharge[3];
    UInt32 lifetimeDischarge[3];
    UInt8 row = 1;
    UInt8 col = 1;

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
 * @return  none.
 */
static void updateMuxWindow (WINDOW * pWin, UInt8 count)
{
    UInt16 voltage = 0;
    UInt8 row = 1;
    UInt8 col = 1;
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
 * @return  none.
 */
static void updateChgWindow (WINDOW * pWin, UInt8 count)
{
    Bool success;
    ChargeState state[NUM_CHARGERS];
    Bool flashDetectPossible;
    UInt8 row = 1;
    UInt8 col = 1;
    UInt8 i;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    wmove (pWin, row, col);
    wclrtoeol (pWin);        
    wprintw (pWin, "  Pi   O1   O2   O3");
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
    }
    row++;
    
    wnoutrefresh (pWin);
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Display a dashboard of useful information.
 *
 * @return  true if successful, otherwise false.
 */
Bool displayDashboard (void)
{
    Bool success = true;
    UInt8 key;
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
    
    /* Print the headers and footers */
    mvprintw (SET_ROW (ROW_HEADING), SET_COL (leftCol + NUM_SPACES_TO_CENTRE_STRING (STRING_MAIN_HEADING, scrCols)), STRING_MAIN_HEADING);
    mvprintw (SET_ROW (ROW_FOOTER), SET_COL (leftCol + NUM_SPACES_TO_CENTRE_STRING (STRING_MAIN_FOOTER, scrCols)), STRING_MAIN_FOOTER);
    box (stdscr, 0, 0);
    refresh();
    
    /* Start sub-windows for the status reports */
    for (i = 0; (i < (sizeof (gWindowList) / sizeof (WindowInfo))) && success; i++)
    {
        if (gWindowList[i].enabled)
        {
            gWindowList[i].pWin = newwin (gWindowList[i].dimensions.height, gWindowList[i].dimensions.width, gWindowList[i].dimensions.startRow, gWindowList[i].dimensions.startCol);
            if (gWindowList[i].pWin)
            {
                mvwprintw (gWindowList[i].pWin, ROW_HEADING, COL_HEADING, "%s:", gWindowList[i].windowHeading);            
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
        for (i = 0; success && ((key = getch()) == (UInt8) ERR); i++) /* i is not meant to be in the condition here */
        {
            for (x = 0; x < (sizeof (gWindowList) / sizeof (gWindowList[0])); x++)
            {
                if (gWindowList[x].enabled)
                {
                    gWindowList[x].winUpdatePtr (gWindowList[x].pWin, i);
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