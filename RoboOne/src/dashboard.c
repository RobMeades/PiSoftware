/*
 * Dashboard and event generator for RoboOne.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <rob_system.h>
#include <curses.h> /* Has to be ahead of rob_system.h in the list as it fiddles with bool */
#include <menu.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>

/*
 * MANIFEST CONSTANTS
 */

/* The shape of the dashboard windows.
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
#define WIN_POWER_START_ROW       9 /* sits on top of the mux window so move this if you ever want them both on */
#define WIN_POWER_START_COL       2
#define WIN_POWER_HEIGHT          3
#define WIN_POWER_WIDTH           35
#define WIN_CHG_START_ROW         13
#define WIN_CHG_START_COL         2
#define WIN_CHG_HEIGHT            2
#define WIN_CHG_WIDTH             25
#define WIN_CMD_START_ROW         12 /* Starts a row higher as there is no heading */
#define WIN_CMD_START_COL         28
#define WIN_CMD_HEIGHT            2
#define WIN_CMD_WIDTH             25
#define WIN_OUTPUT_START_ROW      17
#define WIN_OUTPUT_START_COL      2
#define WIN_OUTPUT_HEIGHT         35
#define WIN_OUTPUT_WIDTH          76
#define WIN_STATE_START_ROW       9 /* sits on top of the mux window so move this if you ever want them both on */
#define WIN_STATE_START_COL       35
#define WIN_STATE_HEIGHT          3
#define WIN_STATE_WIDTH           28

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
#define STRING_POWER_HEADING "  Mains     Pi/RIO  Hindbrain"
 
/* Misc stuff */
#define MAX_NUM_CHARS_IN_ROW  SCR_WIDTH
#define SLOW_UPDATE_BACKOFF   10
#define SLOWER_UPDATE_BACKOFF 20

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
static void initPowerWindow (WINDOW *pWin);
static Bool updatePowerWindow (WINDOW *pWin, UInt8 count);
static void initMuxWindow (WINDOW *pWin);
static Bool updateMuxWindow (WINDOW *pWin, UInt8 count);
static void initChgWindow (WINDOW *pWin);
static Bool updateChgWindow (WINDOW *pWin, UInt8 count);
static void initStateWindow (WINDOW *pWin);
static Bool updateStateWindow (WINDOW *pWin, UInt8 count);

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

/* Array of windows that can be displayed */
WindowInfo gWindowList[] = {{"Output", {WIN_OUTPUT_HEIGHT, WIN_OUTPUT_WIDTH, WIN_OUTPUT_START_ROW, WIN_OUTPUT_START_COL}, initOutputWindow, updateOutputWindow, PNULL, true}, /* MUST be first in the list for the global pointer just below */
                            {"Pi/Rio", {WIN_RIO_HEIGHT, WIN_RIO_WIDTH, WIN_RIO_START_ROW, WIN_RIO_START_COL}, initRioWindow, updateRioWindow, PNULL, true},
                            {"Hindbrain", {WIN_O_HEIGHT, WIN_O_WIDTH, WIN_O_START_ROW, WIN_O_START_COL}, initOWindow, updateOWindow, PNULL, true},
                            {"Power", {WIN_POWER_HEIGHT, WIN_POWER_WIDTH, WIN_POWER_START_ROW, WIN_POWER_START_COL}, initPowerWindow, updatePowerWindow, PNULL, true},
                            {"Analogue", {WIN_MUX_HEIGHT, WIN_MUX_WIDTH, WIN_MUX_START_ROW, WIN_MUX_START_COL}, initMuxWindow, updateMuxWindow, PNULL, false},
                            {"Chargers", {WIN_CHG_HEIGHT, WIN_CHG_WIDTH, WIN_CHG_START_ROW, WIN_CHG_START_COL}, initChgWindow, updateChgWindow, PNULL, true},
                            {"State", {WIN_STATE_HEIGHT, WIN_STATE_WIDTH, WIN_STATE_START_ROW, WIN_STATE_START_COL}, initStateWindow, updateStateWindow, PNULL, true},
                            {"", {WIN_CMD_HEIGHT, WIN_CMD_WIDTH, WIN_CMD_START_ROW, WIN_CMD_START_COL}, initCmdWindow, updateCmdWindow, PNULL, true}}; /* Should be last in the list so that display updates leave the cursor here */
/* Must be in the same order as the enum ChargeState */
Char * gChargeStrings[] = {" --- ", " Off ", " Grn ", "*Grn*", " Red ", "*Red*", "  6  ", " ??? ", " Nul ", " Bad "};
WINDOW **gpOutputWindow = &(gWindowList[0].pWin);

/*
 * STATIC FUNCTIONS
 */

/* Helper function for the power display window
 * Note: the wiring on RoboOne is such that unless
 * the relays are powered and the relay for that device
 * is deliberately switched to 12V/mains power
 * then the device is battery powered.
 * 
 * pWin      window to send output to.
 * isKnown   whether the state of the GPIO is known
 *           or not.
 * isEnabled whether there is power to the relays.
 * is12V     whether the 12V supply is on.
 * isBatt    whether the battery supply is on.
 * 
 * @return  none.
 */

static void displayPowerStatesHelper (WINDOW *pWin, Bool isKnown, Bool isEnabled, Bool is12V, Bool isBatt)
{
    if (isKnown)
    {
        if ((is12V && isEnabled) && isBatt)
        {            
            wprintw (pWin, "  !BOTH!  ");
        }
        else
        {
            if (is12V && isEnabled)
            {
                wprintw (pWin, "  mains   ");
            }
            else
            {
                if (isBatt || !isEnabled)
                {
                    wprintw (pWin, "   batt   ");
                }
                else
                {
                    wprintw (pWin, "   ----   ");                    
                }
            }
        }
    }
    else
    {
        wprintw (pWin, "    ??    ");            
    }
}
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
    HardwareChargeDischarge chargeDischarge;
    UInt8 row = 0;
    UInt8 col = 0;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    chargeDischarge.charge = 0;
    chargeDischarge.discharge = 0;

    hardwareServerSendReceive (HARDWARE_READ_RIO_BATT_CURRENT, PNULL, 0, &current);
    hardwareServerSendReceive (HARDWARE_READ_RIO_BATT_VOLTAGE, PNULL, 0, &voltage);
    wmove (pWin, row, col);
    wclrtoeol (pWin);
    wprintw (pWin, "%d mA, %u mV", current, voltage);
    row++;

    if (count % SLOW_UPDATE_BACKOFF == 0)
    {
        hardwareServerSendReceive (HARDWARE_READ_RIO_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity);
        hardwareServerSendReceive (HARDWARE_READ_RIO_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &(chargeDischarge.discharge));
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "%u mAhr(s) remain", remainingCapacity);
        row++;
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "lifetime -%lu/%lu mAhr", chargeDischarge.discharge, chargeDischarge.charge);
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
    HardwareChargeDischarge chargeDischarge[3];
    UInt8 row = 0;
    UInt8 col = 0;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);
   
    memset (&current, 0, sizeof (current));
    memset (&voltage, 0, sizeof (voltage));
    memset (&remainingCapacity, 0, sizeof (remainingCapacity));
    memset (&chargeDischarge, 0, sizeof (chargeDischarge));

    hardwareServerSendReceive (HARDWARE_READ_O1_BATT_CURRENT, PNULL, 0, &current[0]);
    hardwareServerSendReceive (HARDWARE_READ_O2_BATT_CURRENT, PNULL, 0, &current[1]);
    hardwareServerSendReceive (HARDWARE_READ_O3_BATT_CURRENT, PNULL, 0, &current[2]);
    hardwareServerSendReceive (HARDWARE_READ_O1_BATT_VOLTAGE, PNULL, 0, &voltage[0]);
    hardwareServerSendReceive (HARDWARE_READ_O2_BATT_VOLTAGE, PNULL, 0, &voltage[1]);
    hardwareServerSendReceive (HARDWARE_READ_O3_BATT_VOLTAGE, PNULL, 0, &voltage[2]);
    wmove (pWin, row, col);
    wclrtoeol (pWin);
    wprintw (pWin, "%d/%d/%d mA, %u/%u/%u mV", current[0], current[1], current[2], voltage[0], voltage[1], voltage[2]);
    row++;
    
    if (count % SLOWER_UPDATE_BACKOFF == 0)
    {
        hardwareServerSendReceive (HARDWARE_READ_O1_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity[0]);
        hardwareServerSendReceive (HARDWARE_READ_O2_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity[1]);
        hardwareServerSendReceive (HARDWARE_READ_O3_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity[2]);
        hardwareServerSendReceive (HARDWARE_READ_O1_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &chargeDischarge[0]);
        hardwareServerSendReceive (HARDWARE_READ_O2_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &chargeDischarge[1]);
        hardwareServerSendReceive (HARDWARE_READ_O3_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &chargeDischarge[2]);
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "%u mAhr(s) remain", remainingCapacity[0] + remainingCapacity[1] + remainingCapacity[2]);
        row++;
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        wprintw (pWin, "lifetime -%lu/%lu mAhr", chargeDischarge[0].discharge + chargeDischarge[1].discharge + chargeDischarge[2].discharge, chargeDischarge[0].charge + chargeDischarge[1].charge + chargeDischarge[2].charge);
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
 * Init function for the power window.
 * 
 * pWin    the window where the power
 *         stuff is to be displayed.
 */
static void initPowerWindow (WINDOW *pWin)
{
}

/*
 * Display the state of power to things various.
 * 
 * pWin    the window where the power stuff
 *         can be displayed.
 * count   the number of times this function
 *         has been called.  This is used to
 *         update some items more often than
 *         others.
 * 
 * @return  always false, meaning "not finished".
 */
static Bool updatePowerWindow (WINDOW *pWin, UInt8 count)
{
    UInt8 row = 0;
    UInt8 col = 0;
    Bool success;
    Bool relaysEnabled;
    Bool mains12VPresent;
    static Bool previousMains12VPresent = false;
    Bool is12V;
    Bool isBatt;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    if (count % SLOW_UPDATE_BACKOFF == 0)
    {
        wmove (pWin, row, col);
        wclrtoeol (pWin);     
        wprintw (pWin, "%s", STRING_POWER_HEADING);
        row++;
        
        wmove (pWin, row, col);
        wclrtoeol (pWin);
        
        /* First print the state of 12V power presence */
        success = hardwareServerSendReceive (HARDWARE_READ_MAINS_12V, PNULL, 0, &mains12VPresent);
        if (success)
        {
            if (mains12VPresent)
            {
                wprintw (pWin, " present  ");
                if (!previousMains12VPresent)
                {
                    stateMachineServerSendReceive (STATE_MACHINE_EVENT_MAINS_POWER_AVAILABLE, PNULL, 0, PNULL, PNULL);
                }
            }
            else
            {
                wprintw (pWin, "   ---    ");
            }
            previousMains12VPresent = mains12VPresent;
        }
        else
        {
            wprintw (pWin, "    ??    ");            
        }
        
        /* Then print how each of the Pi and the Hindbrain are powered. */
        success = hardwareServerSendReceive (HARDWARE_READ_ON_PCB_RELAYS_ENABLED, PNULL, 0, &relaysEnabled);
        if (success)
        {
            success = hardwareServerSendReceive (HARDWARE_READ_RIO_PWR_12V, PNULL, 0, &is12V);
            if (success)
            {
                success = hardwareServerSendReceive (HARDWARE_READ_RIO_PWR_BATT, PNULL, 0, &isBatt);
            }
        }
        displayPowerStatesHelper (pWin, success, relaysEnabled, is12V, isBatt);
            
        success = hardwareServerSendReceive (HARDWARE_READ_EXTERNAL_RELAYS_ENABLED, PNULL, 0, &relaysEnabled);
        if (success)
        {
            success = hardwareServerSendReceive (HARDWARE_READ_O_PWR_12V, PNULL, 0, &is12V);
            if (success)
            {
                success = hardwareServerSendReceive (HARDWARE_READ_O_PWR_BATT, PNULL, 0, &isBatt);
            }
        }
        displayPowerStatesHelper (pWin, success, relaysEnabled, is12V, isBatt);
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
            hardwareServerSendReceive (HARDWARE_SET_ANALOGUE_MUX_INPUT, &i, sizeof (i), PNULL);
            hardwareServerSendReceive (HARDWARE_READ_ANALOGUE_MUX, PNULL, 0, &voltage);
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
    HardwareChargeState chargeState;
    UInt8 row = 0;
    UInt8 col = 0;
    UInt8 i;

    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);

    memset (&chargeState.state, 0, sizeof (chargeState.state));
    chargeState.flashDetectPossible = false;
    
    wmove (pWin, row, col);
    wclrtoeol (pWin);        
    wprintw (pWin, "  Pi   O1   O2   O3");
    row++;
    
    success = hardwareServerSendReceive (HARDWARE_READ_CHARGER_STATE, PNULL, 0, &chargeState);
    
    if (success && chargeState.flashDetectPossible)
    {
        wmove (pWin, row, col);
        wclrtoeol (pWin);        
        for (i = 0; i < NUM_CHARGERS; i++)
        {
            ASSERT_PARAM (chargeState.state[i] < (sizeof (gChargeStrings)/sizeof (Char *)), chargeState.state[i]);
            wprintw (pWin, "%s", gChargeStrings[chargeState.state[i]]);
        }
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
 * Init function for the State window.
 * 
 * pWin    the window where the state stuff
 *         is to be displayed.
 */
static void initStateWindow (WINDOW *pWin)
{
}

/*
 * Display the state window.
 * 
 * pWin     the window where state information
 *          can be diesplayed.
 * count    the number of times this function
 *          has been called.  This is used to
 *          update some items more often than
 *          others.
 * 
 * @return  true if the exit key has been 
 *          pressed, otherwise false.
 */
static Bool updateStateWindow (WINDOW *pWin, UInt8 count)
{
    Bool success;
    StateMachineMsgType receivedMsgType = STATE_MACHINE_SERVER_NULL;
    StateMachineServerGetContextCnf *pReceivedMsgBody;
    OInputString *pInputString;
    
    ASSERT_PARAM (pWin != PNULL, (unsigned long) pWin);
    
    wmove (pWin, 0, 0);
    wclrtoeol (pWin);
    
    pReceivedMsgBody = malloc (sizeof (StateMachineServerGetContextCnf));
    
    if (pReceivedMsgBody != PNULL)
    {
        /* Display the state */
        pReceivedMsgBody->roboOneContextContainer.isValid = false;
        success = stateMachineServerSendReceive (STATE_MACHINE_SERVER_GET_CONTEXT, PNULL, 0, &receivedMsgType, (void *) pReceivedMsgBody);
        if (success)
        {
            if ((receivedMsgType == STATE_MACHINE_SERVER_GET_CONTEXT) && pReceivedMsgBody->roboOneContextContainer.isValid)
            {
                wprintw (pWin, "%s", &(pReceivedMsgBody->roboOneContextContainer.roboOneContext.state.name[0]));
            }
            else
            {
                wprintw (pWin, "Invalid response");            
            }
        }
        else
        {
            wprintw (pWin, "Error");
        }
        
        /* Also display whether the Hindbrain is on or off */
        pInputString = malloc (sizeof (*pInputString));
        if (pInputString != PNULL)
        {
            memcpy (&(pInputString->string[0]), PING_STRING, strlen (PING_STRING) + 1); /* +1 to copy the terminator */
            pInputString->waitForResponse = false;
            
            /* Send the ping string but don't bother waiting for a response (if a send succeeds the Hindbrain is there) */
            success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputString, strlen (&(pInputString->string[0])) + 1, PNULL);
            if (success)
            {
               wprintw (pWin, " [Hindbrain ON]");            
            }
            else
            {
                wprintw (pWin, " [Hindbrain OFF]");                                
            }
            free (pInputString);
        }
        
        wnoutrefresh (pWin);
        
        free (pReceivedMsgBody);
    }
    
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