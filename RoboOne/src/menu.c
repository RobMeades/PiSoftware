#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <rob_system.h>
#include <curses.h>
#include <ow_bus.h>
#include <menu.h>
#include <dashboard.h>

/*
 * MANIFEST CONSTANTS
 */

#define KEY_COMMAND_CANCEL '\x1b'   /* the escape key */
#define BACKSPACE_KEY '\x08'
#define SCREEN_BACKSPACE "\x1b[1D" /* ANSI escape sequence for back one space: ESC [ 1 D */
#define GPIO_HEADING_STRING " HPTg  HRTg  Pi12V PiBat H12V  HBat  PiChg H1Chg H2Chg H3Chg "
#define GPIO_ON_STRING "  ON  "
#define GPIO_OFF_STRING " OFF  "
#define GPIO_DONT_KNOW_STRING "  ??  "
#define SWAP_BATTERY_PROMPT "Are you sure you want to change the battery (Y/N)?: "
#define SWAP_BATTERY_STATE_PROMPT "Is the new battery fully charged (Y) (N if it is discharged)?: "
#define SWAP_BATTERY_CNF_MSG "Battery data updated.\n"
#define BATTERY_CAPACITY 2200

/*
 * STATIC FUNCTION PROTOTYPES
 */

static Bool commandHelp (WINDOW *pWin);
static Bool commandExit (void);
static Bool displayCurrents (WINDOW *pWin);
static Bool displayVoltages (WINDOW *pWin);
static Bool displayRemainingCapacities (WINDOW *pWin);
static Bool displayLifetimeChargesDischarges (WINDOW *pWin);
static Bool displayGpioStates (WINDOW *pWin);
static Bool swapRioBatteryCnf (WINDOW *pWin);
static Bool swapO1BatteryCnf (WINDOW *pWin);
static Bool swapO2BatteryCnf (WINDOW *pWin);
static Bool swapO3BatteryCnf (WINDOW *pWin);
static Bool performCalAllBatteryMonitorsCnf (WINDOW *pWin);

/*
 * TYPES
 */
typedef union FunctionTag
{
    Bool    (*commandPtr) ();
    Bool    (*displayPtr) (WINDOW *);   
} Function;


/* Struct to hold a command */
typedef struct CommandTag
{
	UInt8    commandString[MAX_NUM_CHARS_IN_COMMAND];
    Function function;
    Bool     functionCanDisplay;
	Char    *pDescription;
} Command;

/*
 * GLOBALS (prefixed with g)
 */

/* Array of possible commands.  All command strings MUST be unique.
 * Try not to begin any with a Y or N to avoid accidental cross-over with
 * Y/N confirmation prompts.
 * IF YOU MODIFY THIS KEEP THE INDEX ENTRIES UP TO DATE BELOW */
Command gCommandList[] = {{"?", {&commandHelp}, true, "display command help"},
						  {"X", {&commandExit}, false, "exit this program"}, /* Exit must be a non-displaying menu item */
						  {"I", {&displayCurrents}, true, "display current readings"},
 						  {"V", {&displayVoltages}, true, "display Voltage readings"},
 						  {"A", {&displayRemainingCapacities}, true,  "display the accumulated remaining capacities"},
 						  {"G", {&displayGpioStates}, true, "display the state of all the GPIO pins"},
                          {"L", {&displayLifetimeChargesDischarges}, true,  "display the lifetime charge/discharge accumulators"},
 						  {"D", {&runDashboard}, false, "display a dashboard of useful information"},
 						  {"Q", {&performCalAllBatteryMonitorsCnf}, true, "calibrate battery monitors"},
                          {"SP", {&swapRioBatteryCnf}, true, "swap the battery connected to the RIO/Pi"},
                          {"S1", {&swapO1BatteryCnf}, true, "swap the battery connected to the O1"},
                          {"S2", {&swapO2BatteryCnf}, true, "swap the battery connected to the O2"},
                          {"S3", {&swapO3BatteryCnf}, true, "swap the battery connected to the O3"},
                          {"P!", {&togglePiRst}, false, "toggle reset to the Pi [not yet implemented]"},
 						  {"H~", {&toggleOPwr}, false, "toggle power to the hindbrain (AKA Orangutan)"},
 						  {"H!", {&toggleORst}, false, "toggle reset to the hindbrain (AKA Orangutan)"},
 						  {"PB+", {&setRioPwrBattOn}, false, "switch RIO/Pi battery power on"},
 						  {"PB-", {&setRioPwrBattOff}, false, "switch RIO/Pi battery power off"},
 						  {"PM+", {&setRioPwr12VOn}, false, "switch RIO/Pi mains power on"},
 						  {"PM-", {&setRioPwr12VOff}, false, "switch RIO/Pi mains power off"},
 						  {"HB+", {&setOPwrBattOn}, false, "switch hindbrain (AKA Orangutan) battery power on"},
 						  {"HB-", {&setOPwrBattOff}, false, "switch hindbrain (AKA Orangutan) battery power off"},
 						  {"HM+", {&setOPwr12VOn}, false, "switch hindbrain (AKA Orangutan) mains power on"},
 						  {"HM-", {&setOPwr12VOff}, false, "switch hindbrain (AKA Orangutan) mains power off"},
 						  {"CX+", {&setAllBatteryChargersOn}, false, "switch all chargers on"},
 						  {"CX-", {&setAllBatteryChargersOff}, false, "switch all chargers off"},
 						  {"CP+", {&setRioBatteryChargerOn}, false, "switch RIO/Pi charger on"},
 						  {"CP-", {&setRioBatteryChargerOff}, false, "switch RIO/Pi charger off"},
 						  {"CH+", {&setAllOChargersOn}, false, "switch all hindbrain chargers on"},
 						  {"CH-", {&setAllOChargersOff}, false, "switch all hindbrain chargers off"},
 						  {"C1+", {&setO1BatteryChargerOn}, false, "switch hindbrain charger 1 on"},
 						  {"C1-", {&setO1BatteryChargerOff}, false, "switch hindbrain charger 1 off"},
 						  {"C2+", {&setO2BatteryChargerOn}, false, "switch hindbrain charger 2 on"},
 						  {"C2-", {&setO2BatteryChargerOff}, false, "switch hindbrain charger 2 off"},
 						  {"C3+", {&setO3BatteryChargerOn}, false, "switch hindbrain charger 3 on"},
 						  {"C3-", {&setO3BatteryChargerOff}, false, "switch hindbrain charger 3 off"},
                          {"RX+", {&disableAllRelays}, false, "enable power to all relays"},
                          {"RX-", {&enableAllRelays}, false, "disable power to all relays"}};

UInt8 gIndexOfExitCommand = 1;         /* Keep this pointed at the "X" command entry above */

/*
 * STATIC FUNCTIONS
 */

/*
 * Print helper: print to an ncurses window if pWin is not PNULL,
 * otherwise do printf().
 * 
 * pWin     pointer to an ncurses window.
 * pFormat  the format string.
 * ...      the args to go with the format string.
 * 
 * @return  none.
 */
static void printHelper (WINDOW *pWin, const Char * pFormat, ...)
{
    if (pWin != PNULL)
    {
        va_list args;
        va_start (args, pFormat);
        vw_printw (pWin, pFormat, args);
        va_end (args);
    }
    else
    {
        va_list args;
        va_start (args, pFormat);
        vprintf (pFormat, args);
        va_end (args);
    }
}

/*
 * Print the possible commands.
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true.
 */
static Bool commandHelp (WINDOW *pWin)
{
	UInt8 i;

	printHelper (pWin, "\n");
	for (i = 0; i < (sizeof (gCommandList) / sizeof (Command)) - 1; i++)
	{
	    printHelper (pWin, " %s %s,\n", gCommandList[i].commandString, gCommandList[i].pDescription);
	}
	printHelper (pWin, " %s %s.\n", gCommandList[i].commandString, gCommandList[i].pDescription); /* A full stop on the last one */

	return true;
}

/*
 * A dummy command for exit.
 * 
 * @return  true.
 */
static Bool commandExit (void)
{
	return true;
}

/*
 * Display the current readings from all
 * the batteries.
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayCurrents (WINDOW *pWin)
{
    Bool success;
    SInt16 rioCurrent;
    SInt16 o1Current;
    SInt16 o2Current;
    SInt16 o3Current;            
    
    success = readRioBattCurrent (&rioCurrent);
    if (success)
    {
        success = readO1BattCurrent (&o1Current);
        if (success)
        {
            success = readO2BattCurrent (&o2Current);
            if (success)
            {
                success = readO3BattCurrent (&o3Current);
            }
        }
    }
    
    if (success)
    {
        printHelper (pWin, "Currents (mA): Pi/RIO %d, Ox %d, O1 %d, O2 %d, O3 %d (-ve is discharge).\n", rioCurrent, o1Current + o2Current + o3Current, o1Current, o2Current, o3Current);                
    }
    else
    {
        printHelper (pWin, "%s\n", READ_FAILURE_MSG);
    }
        
    return success;    
}

/*
 * Display the Voltage readings from all
 * the batteries.
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayVoltages (WINDOW *pWin)
{
    Bool success;
    UInt16 rioVoltage;
    UInt16 o1Voltage;
    UInt16 o2Voltage;
    UInt16 o3Voltage;            
    
    success = readRioBattVoltage (&rioVoltage);
    if (success)
    {
        success = readO1BattVoltage (&o1Voltage);
        if (success)
        {
            success = readO2BattVoltage (&o2Voltage);
            if (success)
            {
                success = readO3BattVoltage (&o3Voltage);
            }
        }
    }
    
    if (success)
    {
        printHelper (pWin, "Voltage (mV): Pi/RIO %u, O1 %u, O2 %u, O3 %4u.\n", rioVoltage, o1Voltage, o2Voltage, o3Voltage);                
    }
    else
    {
        printHelper (pWin, "%s\n", READ_FAILURE_MSG);
    }
        
    return success;    
}

/*
 * Display the accumulated remaining capacity
 * readings from all the battery monitors.
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayRemainingCapacities (WINDOW *pWin)
{
    Bool success;
    UInt16 rioRemainingCapacity;
    UInt16 o1RemainingCapacity;
    UInt16 o2RemainingCapacity;
    UInt16 o3RemainingCapacity;            
    
    success = readRioRemainingCapacity (&rioRemainingCapacity);
    if (success)
    {
        success = readO1RemainingCapacity (&o1RemainingCapacity);
        if (success)
        {
            success = readO2RemainingCapacity (&o2RemainingCapacity);
            if (success)
            {
                success = readO3RemainingCapacity (&o3RemainingCapacity);
            }
        }
    }
    
    if (success)
    {
        printHelper (pWin, "Remaining capacity (mAhr): Pi/RIO %u, Ox %u, O1 %u, O2 %u, O3 %u.\n", rioRemainingCapacity, o1RemainingCapacity + o2RemainingCapacity + o3RemainingCapacity, o1RemainingCapacity, o2RemainingCapacity, o3RemainingCapacity);                
    }
    else
    {
        printHelper (pWin, "%s\n", READ_FAILURE_MSG);
    }
        
    return success;    
}

/*
 * Display the lifetime charge/discharge accumulator
 * readings from all the battery monitors.
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayLifetimeChargesDischarges (WINDOW *pWin)
{
    Bool success;
    UInt32 rioLifetimeCharge;
    UInt32 o1LifetimeCharge;
    UInt32 o2LifetimeCharge;
    UInt32 o3LifetimeCharge;            
    UInt32 rioLifetimeDischarge;
    UInt32 o1LifetimeDischarge;
    UInt32 o2LifetimeDischarge;
    UInt32 o3LifetimeDischarge;            
    
    success = readRioBattLifetimeChargeDischarge (&rioLifetimeCharge, &rioLifetimeDischarge);
    if (success)
    {
        success = readO1BattLifetimeChargeDischarge (&o1LifetimeCharge, &o1LifetimeDischarge);
        if (success)
        {
            success = readO2BattLifetimeChargeDischarge (&o2LifetimeCharge, &o2LifetimeDischarge);
            if (success)
            {
                success = readO2BattLifetimeChargeDischarge (&o3LifetimeCharge, &o3LifetimeDischarge);
            }
        }
    }
    
    if (success)
    {
        printHelper (pWin, "Charge (mAhr):    Pi/RIO %lu, O1 %lu, O2 %lu, O3 %lu.\n", rioLifetimeCharge, o1LifetimeCharge, o2LifetimeCharge, o3LifetimeCharge);                
        printHelper (pWin, "Discharge (mAhr): Pi/RIO %lu, O1 %lu, O2 %lu, O3 %lu.\n", rioLifetimeDischarge, o1LifetimeDischarge, o2LifetimeDischarge, o3LifetimeDischarge);                
    }
    else
    {
        printHelper (pWin, "%s\n", READ_FAILURE_MSG);
    }
        
    return success;    
}

/* Little helper function for the GPIO states
 * display function below
 * 
 * pWin     a window to send output to, may
 *          be PNULL.
 * isKnown   whether the state of the GPIO is known
 *           or not.
 * isEnabled whether there is power to the relay.
 * isOn      whether the state of the GPIO is ON or OFF.
 * 
 * @return  none.
 */
static void displayGpioStatesHelper (WINDOW *pWin, Bool isKnown, Bool isEnabled, Bool isOn)
{
    if (isKnown)
    {
        if (isOn)
        {
            if (isEnabled)
            {
                printHelper (pWin, "  ON  ");                
            }
            else
            {
                printHelper (pWin, "  on  ");
            }
        }
        else
        {
            if (isEnabled)
            {
                printHelper (pWin, " OFF  ");
            }
            else
            {
                printHelper (pWin, " off  ");
            }
        }
    }
    else
    {
        printHelper (pWin, "  ??  ");
    }    
}

/*
 * Display the state of all the GPIO pins
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayGpioStates (WINDOW *pWin)
{
    Bool success;
    Bool isOn;
    Bool relaysEnabled;
 
    printHelper (pWin, "%s\n", GPIO_HEADING_STRING);
    
    success = readRelaysEnabled (&relaysEnabled);
    if (!relaysEnabled)
    {
        printHelper (pWin, "[");
    }
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readORst (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readRioPwr12V (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readRioPwrBatt (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readOPwr12V (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readOPwrBatt (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readRioBatteryCharger (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readO1BatteryCharger (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readO2BatteryCharger (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = readO3BatteryCharger (&isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    if (!relaysEnabled)
    {
        printHelper (pWin, "]");
    }
    
    printHelper (pWin, "\n");
    
    return true;
}

/*
 * Perform a calibration on all battery monitors
 * but wait for a confirm key to be pressed first
 * as this is something that the user has to have
 * prepared for and shouldn't be run accidentally.
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static bool performCalAllBatteryMonitorsCnf (WINDOW *pWin)
{
    Bool success = true;
    
    /* Prompt for input */
    if (getYesInput (pWin, "Are all the current measurements resistors shorted (Y/N)? "))
    {
        success = performCalAllBatteryMonitors();
        if (success)
        {
            printHelper (pWin, "Calibration completed, don't forget to remove the shorts.\n");
        }
        else
        {
            printHelper (pWin, "%s\n", GENERIC_FAILURE_MSG);
        }
    }
    
    return success;
}

/*
 * Swap the battery connected to the RIO/Pi/5v.
 * This involves zeroing the accumulators, setting
 * the remaining capacity to something sensible
 * and update the elapsed time (this latter because
 * that's the way the DS2438 chip is built).
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool swapRioBatteryCnf (WINDOW *pWin)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = swapRioBattery (timeTicks, remainingCapacity);
            
            if (success)
            {
                printHelper (pWin, SWAP_BATTERY_CNF_MSG);
            }
            else
            {
                printHelper (pWin, "%s\n", GENERIC_FAILURE_MSG);
            }            
        }
    }
    else
    {
        success = false;
    }

    return success;    
}

/*
 * Swap the battery connected to the O1.
 * This involves zeroing the accumulators, setting
 * the remaining capacity to something sensible
 * and update the elapsed time (this latter because
 * that's the way the DS2438 chip is built).
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool swapO1BatteryCnf (WINDOW *pWin)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = swapO1Battery (timeTicks, remainingCapacity);
            
            if (success)
            {
                printHelper (pWin, SWAP_BATTERY_CNF_MSG);
            }
            else
            {
                printHelper (pWin, "%s\n", GENERIC_FAILURE_MSG);
            }            
        }
    }
    else
    {
        success = false;
    }

    return success;    
}

/*
 * Swap the battery connected to the O2.
 * This involves zeroing the accumulators, setting
 * the remaining capacity to something sensible
 * and update the elapsed time (this latter because
 * that's the way the DS2438 chip is built).
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool swapO2BatteryCnf (WINDOW *pWin)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = swapO2Battery (timeTicks, remainingCapacity);
            
            if (success)
            {
                printHelper (pWin, SWAP_BATTERY_CNF_MSG);
            }
            else
            {
                printHelper (pWin, "%s\n", GENERIC_FAILURE_MSG);
            }            
        }
    }
    else
    {
        success = false;
    }

    return success;    
}

/*
 * Swap the battery connected to the O3.
 * This involves zeroing the accumulators, setting
 * the remaining capacity to something sensible
 * and update the elapsed time (this latter because
 * that's the way the DS2438 chip is built).
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool swapO3BatteryCnf (WINDOW *pWin)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = swapO3Battery (timeTicks, remainingCapacity);
            
            if (success)
            {
                printHelper (pWin, SWAP_BATTERY_CNF_MSG);
            }
            else
            {
                printHelper (pWin, "%s\n", GENERIC_FAILURE_MSG);
            }            
        }
    }
    else
    {
        success = false;
    }
   
    return success;    
}

/*
 * Simple conversion to upper case
 */
static UInt8 toUpper (UInt8 digit)
{
	if (digit >= 'a' && digit <= 'z')
	{
		digit &= ~0x20;
	}

	return digit;
}

/*
 * Act on a backspace character.
 * Outputs to the screen only if pWin
 * is PNULL (if curses is active
 * it will need to do its own screen
 * things)
 */
static void actOnBackspace (WINDOW *pWin, UInt8 *pEntry, UInt8 *pNumEntries)
{
	*pEntry = 0;
	(*pNumEntries)--;
	if (pWin == PNULL)
	{
	    printf (SCREEN_BACKSPACE);
	    putchar (' ');
	    printf (SCREEN_BACKSPACE);
	}
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Get a Yes input.  Note that this does
 * its own terminal settings stuff so that
 * it can be called from anywhere.
 * 
 * pWin     a window to send output to, may
 *          be PNULL.
 * pPrompt  a string to print as a prompt,
 *          may be PNULL.
 *
 * @return  true if "Y" was pressed, otherwise false.
 * 
 */
Bool getYesInput (WINDOW *pWin, Char *pPrompt)
{
    Bool answerIsYes = false;
    struct termios savedSettings;
    struct termios newSettings;

    /* Check if this is a TTY device */
    if (tcgetattr (STDIN_FILENO, &savedSettings) == 0)
    {
        /* Save current settings and then set us up for no echo, non-canonical (i.e. no need for enter) */
        memcpy (&newSettings, &savedSettings, sizeof (newSettings));
        newSettings.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHONL | ECHOKE | ICRNL);
        tcsetattr (STDIN_FILENO, TCSANOW, &newSettings);
        
        /* Prompt for input and get the answer */
        if (pPrompt != PNULL)
        {
            printHelper (pWin, pPrompt);
        }
        
        if (toUpper (getchar()) == 'Y')
        {
            answerIsYes = true;
        }

        /* Restore saved settings */
        tcsetattr (STDIN_FILENO, TCSANOW, &savedSettings);
    }
    
    return answerIsYes;
}

/*
 * Act on user input.  This function
 * outputs to the screen directly when
 * run as part of the menu outside the
 * dashboard.  When accessed from
 * inside the dashboard it instead
 * returns the characters matched so far
 * in pMatch.
 *
 * pWin      a window to send output
 *           to, may be PNULL.
 * key       the character entered.
 * pExitMenu pointer to a Bool that will
 *           be set to true if the user
 *           has chosen to exit the
 *           menu, or otherwise set to false.
 * pMatch    pointer to a place to store
 *           the matched charactters
 *           (MAX_NUM_CHARS_IN_COMMAND characters
 *           required).  A null terminator
 *           is included.  May be PNULL if
 *           pWin is PNULL.
 *
 * @return  true if a command has been executed
 *          otherwise false.
 */
Bool handleUserInputMenu (WINDOW *pWin, UInt8 key, Bool *pExitMenu, Char *pMatch)
{
    Bool success;
    Bool commandExecuted = false;
    static UInt8 numEntries = 0;
    static UInt8 entry[MAX_NUM_CHARS_IN_COMMAND];
    UInt8 i;

    ASSERT_PARAM (pExitMenu != PNULL, (unsigned long) pExitMenu);
    *pExitMenu = false;
    if (pMatch != PNULL)
    {
        *pMatch = 0; /* Put a null terminator in to start with */
    }

    switch (key)
    {
        case KEY_COMMAND_CANCEL:
        {
            /* Reset the number of entries and go fully backwards on the screen */
            for (i = 0; i < numEntries; i++)
            {
                actOnBackspace (pWin, &entry[i], &numEntries);
            }
        }
        break;
        case BACKSPACE_KEY:
        {
            /* Set last entry to zero and go back one on the screen */
            if (numEntries > 0)
            {
                actOnBackspace (pWin, &entry[numEntries], &numEntries);
            }
        }
        break;
        /* Handle all the other commands */
        default:
        {
            /* Go through the characters entered so far and find out
             * which command they refer to.  When only one is found,
             * run the command function. */
            UInt8 numFound;
            UInt8 lastCommandMatching = 0;
            Bool matching[sizeof (gCommandList) / sizeof (Command)];
            UInt8 entryPos = 0;

            entry[numEntries] = toUpper (key);
            numEntries++;
            memset (&matching[0], true, sizeof (matching));
            do
            {
                numFound = 0;
                lastCommandMatching = 0;
                for (i = 0; i < sizeof (matching); i++)
                {
                    if (matching[i])
                    {
                        if (gCommandList[i].commandString[entryPos] == entry[entryPos])
                        {
                            numFound++;
                            lastCommandMatching = i;
                        }
                        else
                        {
                            matching[i] = false;
                        }
                    }
                }
                if (numFound > 0)
                {
                    entryPos++;
                }
            } while ((entryPos < numEntries) && (numFound > 0));

            if (numFound == 0)
            {
                /* None found, delete the most recent key entered as it won't get us anywhere */
                numEntries--;
            }
            else
            {
                /* Found a possible command so display the new character (if not in curses
                 * mode) */
                if (pWin == PNULL)
                {
                    putchar (entry[numEntries - 1]);                    
                }
                
                /* If there is only one command then call the function */
                if (numFound == 1)
                {
                    if (gCommandList[lastCommandMatching].functionCanDisplay)
                    {
                        if (gCommandList[lastCommandMatching].function.displayPtr)
                        {
                            printHelper (pWin, "Cmd: %s...\n", gCommandList[lastCommandMatching].pDescription);
                            success = gCommandList[lastCommandMatching].function.displayPtr (pWin);                            
                        }
                    }
                    else
                    {
                        if (gCommandList[lastCommandMatching].function.commandPtr)
                        {
                            printHelper (pWin, "Cmd: %s...\n", gCommandList[lastCommandMatching].pDescription);
                            success = gCommandList[lastCommandMatching].function.commandPtr ();                            

                            /* Check if we're exiting and give some feedback for these functions that
                             * otherwise don't give any */
                            if (lastCommandMatching != gIndexOfExitCommand)
                            {
                                if (success)
                                {
                                    printHelper (pWin, "Done.\n");
                                }
                            }
                            else
                            {        
                                *pExitMenu = true;
                            }
                        }                        
                    }

                    commandExecuted = true;
                }
            }
        }
        break;
    } /* end of switch() */
    
    /* Copy the matches so far to the pMatch string (for curses mode) */                    
    if (pMatch != PNULL)
    {
        memcpy (pMatch, &entry[0], numEntries);
        *(pMatch + numEntries) = 0;
    }

    /* Reset for next time */
    if (commandExecuted)
    {
        numEntries = 0;  
    }
    
    return commandExecuted;
}

/*
 * Run an interactive menu system.
 * This doesn't use curses or run the
 * state machine, if you want to do that
 * you need to run it from the dashboard.

 * @return  true if the menu ran OK, otherwiss
 *          false.
 */
Bool runMenu (void)
{
    Bool success = false;
    Bool exitMenu = false;
    Bool commandExecuted = true;
    UInt8 key;
    struct termios savedSettings;
    struct termios newSettings;

    /* Check if this is a TTY device */
    if (tcgetattr (STDIN_FILENO, &savedSettings) == 0)
    {
        success = true;
        /* Save current settings and then set us up for no echo, non-canonical (i.e. no need for enter) */
        memcpy (&newSettings, &savedSettings, sizeof (newSettings));
        newSettings.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHONL | ECHOKE | ICRNL);
        tcsetattr (STDIN_FILENO, TCSANOW, &newSettings);

        /* Run the interactive bit */
        while (!exitMenu)
        {
            if (commandExecuted)
            {
                /* Prompt for (more) commands */
                printf ("\n%s", COMMAND_PROMPT);                
            }
            key = getchar();
            commandExecuted = handleUserInputMenu (PNULL, key, &exitMenu, PNULL);
        }

        /* Restore saved settings, hopefully */
        tcsetattr (STDIN_FILENO, TCSANOW, &savedSettings);
    }

    return success;
}