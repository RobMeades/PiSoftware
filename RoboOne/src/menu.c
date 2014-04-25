#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <rob_system.h>
#include <curses.h>
#include <menu.h>
#include <local_server.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <task_handler_types.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <main.h>

/*
 * MANIFEST CONSTANTS
 */

#define KEY_COMMAND_CANCEL '\x1b'   /* the escape key */
#define BACKSPACE_KEY '\x08'
#define SCREEN_BACKSPACE "\x1b[1D" /* ANSI escape sequence for back one space: ESC [ 1 D */
#define RELAY_HEADING_STRING " HPTg  HRTg  Pi12V PiBat H12V  HBat  PiChg H1Chg H2Chg H3Chg"
#define GPIO_HEADING_STRING  "  0     1     2     3     4     5     6     7"
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
static Bool displayRelayStates (WINDOW *pWin);
static Bool displayGpios (WINDOW *pWin);
static Bool swapRioBatteryCnf (WINDOW *pWin);
static Bool swapO1BatteryCnf (WINDOW *pWin);
static Bool swapO2BatteryCnf (WINDOW *pWin);
static Bool swapO3BatteryCnf (WINDOW *pWin);
static Bool performCalAllBatteryMonitorsCnf (WINDOW *pWin);
static Bool togglePiRst (void);
static Bool toggleOPwr (void);
static Bool toggleORst (void);
static Bool setRioPwrBattOn (void);
static Bool setRioPwrBattOff (void);
static Bool setRioPwr12VOn (void);
static Bool setRioPwr12VOff (void);
static Bool setOPwrBattOn (void);
static Bool setOPwrBattOff (void);
static Bool setOPwr12VOn (void);
static Bool setOPwr12VOff (void);
static Bool setAllBatteryChargersOn (void);
static Bool setAllBatteryChargersOff (void);
static Bool setRioBatteryChargerOn (void);
static Bool setRioBatteryChargerOff (void);
static Bool setAllOChargersOn (void);
static Bool setAllOChargersOff (void);
static Bool setO1BatteryChargerOn (void);
static Bool setO1BatteryChargerOff (void);
static Bool setO2BatteryChargerOn (void);
static Bool setO2BatteryChargerOff (void);
static Bool setO3BatteryChargerOn (void);
static Bool setO3BatteryChargerOff (void);
static Bool enableAllRelays (void);
static Bool disableAllRelays (void);
static Bool sendOString (WINDOW *pWin);
static Bool sendOTask (WINDOW *pWin);

/*
 * TYPES
 */
typedef union FunctionTag
{
    Bool    (*pCommand) ();
    Bool    (*pDisplay) (WINDOW *);   
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
 * EXTERNS
 */
extern RoboOneGlobals gRoboOneGlobals;

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
                          {"L", {&displayLifetimeChargesDischarges}, true,  "display the lifetime charge/discharge accumulators"},
                          {"G", {&displayGpios}, true, "display GPIO pin states"},
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
                          {"HS", {&sendOString}, true, "send hindbrain (AKA Orangutan) a string"},
                          {"T", {&sendOTask}, true, "send a task to hindbrain (AKA Orangutan)"},
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
                          {"R?", {&displayRelayStates}, true, "display the state of all relays"},
                          {"RX+", {&enableAllRelays}, false, "enable power to all relays"},
                          {"RX-", {&disableAllRelays}, false, "disable power to all relays"}};

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
 * Things to do on exit.
 * 
 * @return  true.
 */
static Bool commandExit (void)
{
	return stateMachineServerSendReceive (STATE_MACHINE_EVENT_SHUTDOWN, PNULL, 0, PNULL, PNULL);
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
    
    success = hardwareServerSendReceive (HARDWARE_READ_RIO_BATT_CURRENT, PNULL, 0, &rioCurrent);
    if (success)
    {
        success = hardwareServerSendReceive (HARDWARE_READ_O1_BATT_CURRENT, PNULL, 0, &o1Current);
        if (success)
        {
            success = hardwareServerSendReceive (HARDWARE_READ_O2_BATT_CURRENT, PNULL, 0, &o2Current);
            if (success)
            {
                success = hardwareServerSendReceive (HARDWARE_READ_O3_BATT_CURRENT, PNULL, 0, &o3Current);
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
    
    success = hardwareServerSendReceive (HARDWARE_READ_RIO_BATT_VOLTAGE, PNULL, 0, &rioVoltage);
    if (success)
    {
        success = hardwareServerSendReceive (HARDWARE_READ_O1_BATT_VOLTAGE, PNULL, 0, &o1Voltage);
        if (success)
        {
            success = hardwareServerSendReceive (HARDWARE_READ_O2_BATT_VOLTAGE, PNULL, 0, &o2Voltage);
            if (success)
            {
                success = hardwareServerSendReceive (HARDWARE_READ_O3_BATT_VOLTAGE, PNULL, 0, &o3Voltage);
            }
        }
    }
    
    if (success)
    {
        printHelper (pWin, "Voltage (mV): Pi/RIO %u, O1 %u, O2 %u, O3 %u.\n", rioVoltage, o1Voltage, o2Voltage, o3Voltage);                
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
    
    success = hardwareServerSendReceive (HARDWARE_READ_RIO_REMAINING_CAPACITY, PNULL, 0, &rioRemainingCapacity);
    if (success)
    {
        success = hardwareServerSendReceive (HARDWARE_READ_O1_REMAINING_CAPACITY, PNULL, 0, &o1RemainingCapacity);
        if (success)
        {
            success = hardwareServerSendReceive (HARDWARE_READ_O2_REMAINING_CAPACITY, PNULL, 0, &o2RemainingCapacity);
            if (success)
            {
                success = hardwareServerSendReceive (HARDWARE_READ_O3_REMAINING_CAPACITY, PNULL, 0, &o3RemainingCapacity);
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
    HardwareChargeDischarge  rioLifetime;
    HardwareChargeDischarge  o1Lifetime;
    HardwareChargeDischarge  o2Lifetime;
    HardwareChargeDischarge  o3Lifetime;
    
    success = hardwareServerSendReceive (HARDWARE_READ_RIO_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &rioLifetime);
    if (success)
    {
        success = hardwareServerSendReceive (HARDWARE_READ_O1_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &o1Lifetime);
        if (success)
        {
            success = hardwareServerSendReceive (HARDWARE_READ_O2_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &o2Lifetime);
            if (success)
            {
                success = hardwareServerSendReceive (HARDWARE_READ_O3_BATT_LIFETIME_CHARGE_DISCHARGE, PNULL, 0, &o3Lifetime);
            }
        }
    }
    
    if (success)
    {
        printHelper (pWin, "Charge (mAhr):    Pi/RIO %lu, O1 %lu, O2 %lu, O3 %lu.\n", rioLifetime.charge, o1Lifetime.charge, o2Lifetime.charge, o3Lifetime.charge);                
        printHelper (pWin, "Discharge (mAhr): Pi/RIO %lu, O1 %lu, O2 %lu, O3 %lu.\n", rioLifetime.discharge, o1Lifetime.discharge, o2Lifetime.discharge, o3Lifetime.discharge);                
    }
    else
    {
        printHelper (pWin, "%s\n", READ_FAILURE_MSG);
    }
        
    return success;    
}

/* Little helper function for the GPIO/Relay states
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
 * Display the state of all the relay
 * control pins
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayRelayStates (WINDOW *pWin)
{
    Bool success;
    Bool isOn;
    Bool relaysEnabled;
 
    printHelper (pWin, "%s\n", RELAY_HEADING_STRING);
    
    hardwareServerSendReceive (HARDWARE_READ_ON_PCB_RELAYS_ENABLED, PNULL, 0, &relaysEnabled);
    success = hardwareServerSendReceive (HARDWARE_READ_O_PWR, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_O_RST, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_RIO_PWR_12V, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_RIO_PWR_BATT, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    hardwareServerSendReceive (HARDWARE_READ_EXTERNAL_RELAYS_ENABLED, PNULL, 0, &relaysEnabled);
    success = hardwareServerSendReceive (HARDWARE_READ_O_PWR_12V, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_O_PWR_BATT, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_RIO_BATTERY_CHARGER, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_O1_BATTERY_CHARGER, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_O2_BATTERY_CHARGER, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    success = hardwareServerSendReceive (HARDWARE_READ_O3_BATTERY_CHARGER, PNULL, 0, &isOn);
    displayGpioStatesHelper (pWin, success, relaysEnabled, isOn);
    
    printHelper (pWin, "\n");
    
    return true;
}

/*
 * Display the state of all GPIO pins
 *
 * pWin     a window to send output to, may
 *          be PNULL.
 *
 * @return  always true.
 */
static Bool displayGpios (WINDOW *pWin)
{
    Bool success;
    UInt8 pinsState;
    UInt8 i;
    Bool isOn;
    UInt8 mask;
 
    printHelper (pWin, "%s\n", GPIO_HEADING_STRING);
    
    success = hardwareServerSendReceive (HARDWARE_READ_GENERAL_PURPOSE_IOS, PNULL, 0, &pinsState);
    for (i = 0; i < 8; i++)
    {
        mask = 1 << i;
        isOn = false;
        if (mask & pinsState)
        {
            isOn = true;
        }
        displayGpioStatesHelper (pWin, success, true, isOn);
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
static Bool performCalAllBatteryMonitorsCnf (WINDOW *pWin)
{
    Bool success = true;
    
    /* Prompt for input */
    if (getYesInput (pWin, "Are all the current measurements resistors shorted (Y/N)? "))
    {
        success = hardwareServerSendReceive (HARDWARE_PERFORM_CAL_ALL_BATTERY_MONITORS, PNULL, 0, PNULL);
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
 * Toggle Pi reset. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool togglePiRst (void)
{
    return hardwareServerSendReceive (HARDWARE_TOGGLE_PI_RST, PNULL, 0, PNULL);
}

/*
 * Toggle power to the Orangutan, AKA Hindbrain. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool toggleOPwr (void)
{
    return hardwareServerSendReceive (HARDWARE_TOGGLE_O_PWR, PNULL, 0, PNULL);
}

/*
 * Toggle reset to the Orangutan, AKA Hindbrain. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool toggleORst (void)
{
    return hardwareServerSendReceive (HARDWARE_TOGGLE_O_RST, PNULL, 0, PNULL);
}

/*
 * Switch RIO/Pi battery power on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwrBattOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_BATT_ON, PNULL, 0, PNULL);
}

/*
 * Switch RIO/Pi battery power off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwrBattOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_BATT_OFF, PNULL, 0, PNULL);
}

/*
 * Switch RIO/Pi 12V/mains power on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwr12VOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_12V_ON, PNULL, 0, PNULL);
}

/*
 * Switch RIO/Pi 12V/mains power off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioPwr12VOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_RIO_PWR_12V_OFF, PNULL, 0, PNULL);
}

/*
 * Switch Orangutan, AKA Hindbrain, battery power on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwrBattOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O_PWR_BATT_ON, PNULL, 0, PNULL);
}

/*
 * Switch Orangutan, AKA Hindbrain, battery power off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwrBattOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O_PWR_BATT_OFF, PNULL, 0, PNULL);
}

/*
 * Switch Orangutan, AKA Hindbrain, mains/12V power on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwr12VOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O_PWR_12V_ON, PNULL, 0, PNULL);
}

/*
 * Switch Orangutan, AKA Hindbrain, mains/12V  power off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setOPwr12VOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O_PWR_12V_OFF, PNULL, 0, PNULL);
}

/*
 * Switch all battery chargers on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setAllBatteryChargersOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_ALL_BATTERY_CHARGERS_ON, PNULL, 0, PNULL);
}

/*
 * Switch all battery chargers off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setAllBatteryChargersOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_ALL_BATTERY_CHARGERS_OFF, PNULL, 0, PNULL);
}

/*
 * Switch RIO/Pi battery charger on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioBatteryChargerOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_RIO_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
}

/*
 * Switch RIO/Pi battery charger off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setRioBatteryChargerOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_RIO_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);
}

/*
 * Switch all the Orangutan/Hindbrain battery
 * chargers on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setAllOChargersOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_ALL_O_BATTERY_CHARGERS_ON, PNULL, 0, PNULL);
}

/*
 * Switch all the Orangutan/Hindbrain battery
 * chargers off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setAllOChargersOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_ALL_O_BATTERY_CHARGERS_OFF, PNULL, 0, PNULL);
}

/*
 * Switch O1 battery charger on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO1BatteryChargerOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O1_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
}

/*
 * Switch O1 battery charger off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO1BatteryChargerOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O1_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);
}

/*
 * Switch O2 battery charger on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO2BatteryChargerOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O2_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
}

/*
 * Switch O2 battery charger off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO2BatteryChargerOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O2_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);
}

/*
 * Switch O3 battery charger on. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO3BatteryChargerOn (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O3_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
}

/*
 * Switch O3 battery charger off. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool setO3BatteryChargerOff (void)
{
    return hardwareServerSendReceive (HARDWARE_SET_O3_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);
}

/*
 * Enable power to all relays. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool enableAllRelays (void)
{
    Bool success;
    
    success = hardwareServerSendReceive (HARDWARE_ENABLE_ON_PCB_RELAYS, PNULL, 0, PNULL);
    if (success)
    {
        success = hardwareServerSendReceive (HARDWARE_ENABLE_EXTERNAL_RELAYS, PNULL, 0, PNULL);
    }
    
    return success;
}

/*
 * Disable power to all relays. 
 *
 * @return  true if successful, otherwise false.
 */
static Bool disableAllRelays (void)
{
    Bool success;
    
    success = hardwareServerSendReceive (HARDWARE_DISABLE_ON_PCB_RELAYS, PNULL, 0, PNULL);
    if (success)
    {
        success = hardwareServerSendReceive (HARDWARE_DISABLE_EXTERNAL_RELAYS, PNULL, 0, PNULL);
    }
    
    return success;
}

/*
 * Send a string to the Orangutan.
 *
 * pWin     a window to send output to,
 *          may be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool sendOString (WINDOW *pWin)
{
    Bool success = false;
    OInputContainer *pInputContainer;
    OResponseString *pResponseString;
    Char * pDisplayBuffer;
     
    pInputContainer = malloc (sizeof (*pInputContainer));
    if (pInputContainer != PNULL)
    {
        pInputContainer->waitForResponse = true;
        
        pResponseString = malloc (sizeof (*pResponseString));
        if (pResponseString != PNULL)
        {
            pResponseString->stringLength = sizeof (pResponseString->string);
            
            pDisplayBuffer = malloc (MAX_O_STRING_LENGTH);
            if (pDisplayBuffer != PNULL)
            {
                if (getStringInput (pWin, "String: ", &(pInputContainer->string[0]), sizeof (pInputContainer->string)) != PNULL)
                {
                    removeCtrlCharacters (&(pInputContainer->string[0]), pDisplayBuffer);
                    
                    /* Send the string and look for a response */
                    success = hardwareServerSendReceive (HARDWARE_SEND_O_STRING, pInputContainer, sizeof (*pInputContainer), pResponseString);
                    if (success)
                    {
                        printHelper (pWin, "\nSent '%s' successfully", pDisplayBuffer);
                        if (pResponseString->stringLength > 1) /* There will always be a terminator, hence the 1 */
                        {
                            removeCtrlCharacters (&(pResponseString->string[0]), pDisplayBuffer);
                            printHelper (pWin, ", response: '%s'.\n", pDisplayBuffer);                
                        }
                        else
                        {
                            printHelper (pWin, ", no response.\n");                                
                        }
                    }
                    else
                    {
                        printHelper (pWin, "\nSend failed.\n");
                    }
                }
                free (pDisplayBuffer);
            }
            free (pResponseString);
        }
        free (pInputContainer);
    }
     
    return success;
}

/*
 * Send a task to the Orangutan.  This is
 * pretty much the same as sending a string
 * but goes via the state machine so that we
 * shift into mobile state to do it.
 *
 * pWin     a window to send output to,
 *          may be PNULL.
 *
 * @return  true if successful, otherwise false.
 */
static Bool sendOTask (WINDOW *pWin)
{
    Bool success = false;
    RoboOneTaskReq *pTaskReq;

    pTaskReq = malloc (sizeof (*pTaskReq));
    if (pTaskReq != PNULL)
    {
        /* Put in a header so that the task handler knows where to send indications of progress */
        pTaskReq->headerPresent = true;
        pTaskReq->header.sourceServerPort = LOCAL_SERVER_PORT;
        pTaskReq->header.sourceServerIpAddressStringPresent = false;
        pTaskReq->header.handle = gRoboOneGlobals.roboOneTaskInfo.taskCounter;
        pTaskReq->body.protocol = TASK_PROTOCOL_HD;

        if (getStringInput (pWin, "Task string: ", &(pTaskReq->body.detail.hdReq.string[0]), sizeof (pTaskReq->body.detail.hdReq.string)) != PNULL)
        {
            removeCtrlCharacters (&(pTaskReq->body.detail.hdReq.string[0]), &(gRoboOneGlobals.roboOneTaskInfo.lastTaskSent[0]));
            gRoboOneGlobals.roboOneTaskInfo.lastResultReceivedIsValid = false;
            gRoboOneGlobals.roboOneTaskInfo.lastIndString[0] = 0;
            
            /* Send the task via the state machine */
            success = stateMachineServerSendReceive (STATE_MACHINE_EVENT_TASKS_AVAILABLE, pTaskReq, sizeof (*pTaskReq), PNULL, PNULL);
            if (success)
            {
                removeCtrlCharacters (&(pTaskReq->body.detail.hdReq.string[0]), &(gRoboOneGlobals.roboOneTaskInfo.lastTaskSent[0]));
                gRoboOneGlobals.roboOneTaskInfo.taskCounter++;
                printHelper (pWin, "\nSent task '%s'.\n", &(gRoboOneGlobals.roboOneTaskInfo.lastTaskSent[0]));
            }
            else
            {
                gRoboOneGlobals.roboOneTaskInfo.lastTaskSent[0] = 0;
                printHelper (pWin, "\nSend failed.\n");
            }
        }
        free (pTaskReq);
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
    HardwareBatterySwapData batterySwapData;
    
    batterySwapData.remainingCapacity = 0;
    batterySwapData.systemTime = getSystemTicks();
    if (batterySwapData.systemTime != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                batterySwapData.remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = hardwareServerSendReceive (HARDWARE_SWAP_RIO_BATTERY, &batterySwapData, sizeof (batterySwapData), PNULL);
            
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
    HardwareBatterySwapData batterySwapData;
    
    batterySwapData.remainingCapacity = 0;
    batterySwapData.systemTime = getSystemTicks();
    if (batterySwapData.systemTime != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                batterySwapData.remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = hardwareServerSendReceive (HARDWARE_SWAP_O1_BATTERY, &batterySwapData, sizeof (batterySwapData), PNULL);
            
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
    HardwareBatterySwapData batterySwapData;
    
    batterySwapData.remainingCapacity = 0;
    batterySwapData.systemTime = getSystemTicks();
    if (batterySwapData.systemTime != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                batterySwapData.remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = hardwareServerSendReceive (HARDWARE_SWAP_O2_BATTERY, &batterySwapData, sizeof (batterySwapData), PNULL);
            
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
    HardwareBatterySwapData batterySwapData;
    
    batterySwapData.remainingCapacity = 0;
    batterySwapData.systemTime = getSystemTicks();
    if (batterySwapData.systemTime != 0)
    {
        if (getYesInput (pWin, SWAP_BATTERY_PROMPT))
        {
            printHelper (pWin, "\n");
            /* Check that the new battery is fully charged */
            if (getYesInput (pWin, SWAP_BATTERY_STATE_PROMPT))
            {
                batterySwapData.remainingCapacity = BATTERY_CAPACITY;
            }
            printHelper (pWin, "\n");
            
            /* Now do the swap */
            success = hardwareServerSendReceive (HARDWARE_SWAP_O3_BATTERY, &batterySwapData, sizeof (batterySwapData), PNULL);
            
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
 *
 * pWin        a window to send output to, may
 *             be PNULL.
 * pEntry      pointer to the current character
 *             in the input buffer.
 * pNumEntries pointer to the number of characters
 *             in the input buffer.
 */
static void actOnBackspace (WINDOW *pWin, UInt8 *pEntry, UInt8 *pNumEntries)
{
    UInt8 row = 0;
    UInt8 col = 0;
    
	*pEntry = 0;
	(*pNumEntries)--;
	if (pWin == PNULL)
	{
	    printf (SCREEN_BACKSPACE);
	    putchar (' ');
	    printf (SCREEN_BACKSPACE);
	}
	else
	{
	    getyx (pWin, row, col);
	    wmove (pWin, row, col - 1);
        wdelch (pWin);
        wnoutrefresh (pWin); /* give user feedback immediately */
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
 * pWin     a window to get input from,
 *          may be PNULL.
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
            if (pWin != PNULL)
            {
                wnoutrefresh (pWin);
                doupdate ();
            }
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
 * Get a string of input.  This does its own
 * terminal settings stuff so that
 * it can be called from anywhere.
 * 
 * pWin      a window to get input from,
 *           may be PNULL.
 * pPrompt   a string to print as a prompt,
 *           may be PNULL.
 * pString   a pointer to a place to store
 *           the string.  It will be null
 *           terminated.
 * stringLen the maximum length of the
 *           returned string (including
 *           terminator). 
 *
 * @return  a pointer to the string.
 * 
 */
Char * getStringInput (WINDOW *pWin, Char *pPrompt, Char *pString, UInt32 stringLen)
{
    Char * pReturnValue = PNULL;
    struct termios savedSettings;
    struct termios newSettings;

    ASSERT_PARAM (pString != PNULL, (unsigned long) pString);
    ASSERT_PARAM (stringLen > 0, stringLen);

    /* Check if this is a TTY device */
    if (tcgetattr (STDIN_FILENO, &savedSettings) == 0)
    {
        /* Save current settings and then set us up for canonical input (i.e. needs enter to complete) */
        memcpy (&newSettings, &savedSettings, sizeof (newSettings));
        newSettings.c_iflag |= ICRNL;
        newSettings.c_lflag |= ICANON | ECHO | ECHOE | ECHOK | ECHOKE;
        tcsetattr (STDIN_FILENO, TCSANOW, &newSettings);
        
        tcflush (STDIN_FILENO, TCIFLUSH);
        /* Prompt for input */
        if (pPrompt != PNULL)
        {
            printHelper (pWin, pPrompt);
            if (pWin != PNULL)
            {
                wnoutrefresh (pWin);
                doupdate ();
            }
        }
        
        /* Now get the string. fgets() will include the LF that terminates
         * the input and will add a null terminator to the string also */
        pReturnValue = fgets (pString, stringLen, stdin);
        
        /* Restore saved settings */
        tcsetattr (STDIN_FILENO, TCSANOW, &savedSettings);
    }
    
    return pReturnValue;
}

/*
 * Act on user input.
 *
 * pCMdWin    the command window to
 *            get user feedback from.
 * key        the character entered.
 * pOutputWin the window to send user
 *            information to.
 * pExitMenu  pointer to a location that
 *            will be set to true if
 *            the user chose to exit,
 *            otherwise false.
 *
 * @return    true if a command has
 *            been executed, otherwise
 *            false.
 */
Bool handleUserCmdMenu (WINDOW *pCmdWin, UInt8 key, WINDOW *pOutputWin, Bool *pExitMenu)
{
    Bool success;
    Bool commandExecuted = false;
    static UInt8 numEntries = 0;
    static UInt8 entry[MAX_NUM_CHARS_IN_COMMAND];
    UInt8 i;

    ASSERT_PARAM (pCmdWin != PNULL, (unsigned long) pCmdWin);
    ASSERT_PARAM (pOutputWin != PNULL, (unsigned long) pOutputWin);
    ASSERT_PARAM (pExitMenu != PNULL, (unsigned long) pExitMenu);
    
    *pExitMenu = false;

    switch (key)
    {
        case KEY_COMMAND_CANCEL:
        {
            /* Reset the number of entries and go fully backwards on the screen */
            for (i = 0; i < numEntries; i++)
            {
                actOnBackspace (pCmdWin, &entry[i], &numEntries);
            }
        }
        break;
        case BACKSPACE_KEY:
        {
            /* Set last entry to zero and go back one on the screen */
            if (numEntries > 0)
            {
                actOnBackspace (pCmdWin, &entry[numEntries], &numEntries);
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
                /* Found a possible command so display the new character straight away */
                wprintw (pCmdWin, "%c", entry[numEntries - 1]);                    
                wnoutrefresh (pCmdWin);
                doupdate ();

                /* If there is only one command then call the function */
                if (numFound == 1)
                {
                    if (gCommandList[lastCommandMatching].functionCanDisplay)
                    {
                        if (gCommandList[lastCommandMatching].function.pDisplay)
                        {
                            wprintw (pOutputWin, "Cmd: %s...\n", gCommandList[lastCommandMatching].pDescription);
                            success = gCommandList[lastCommandMatching].function.pDisplay (pOutputWin);                            
                        }
                    }
                    else
                    {
                        if (gCommandList[lastCommandMatching].function.pCommand)
                        {
                            wprintw (pOutputWin, "Cmd: %s...\n", gCommandList[lastCommandMatching].pDescription);
                            success = gCommandList[lastCommandMatching].function.pCommand ();                            

                            /* Check if we're exiting and give some feedback for these functions that
                             * otherwise don't give any */
                            if (lastCommandMatching != gIndexOfExitCommand)
                            {
                                if (success)
                                {
                                    wprintw (pOutputWin, "Done.\n");
                                }
                            }
                            else
                            {        
                                *pExitMenu = true;
                            }
                        }                        
                    }

                    commandExecuted = true;
                    numEntries = 0;
                }
            }
        }
        break;
    } /* end of switch() */
    
    return commandExecuted;
}