#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <rob_system.h>
#include <ow_bus.h>
#include <menu.h>

/*
 * MANIFEST CONSTANTS
 */

#define MAX_NUM_CHARS_IN_COMMAND 4 /* includes a null terminator */
#define KEY_COMMAND_CANCEL '\x1b'   /* the escape key */
#define KEY_BACKSPACE '\x08'
#define SCREEN_BACKSPACE "\x1b[1D" /* ANSI escape sequence for back one space: ESC [ 1 D */
#define COMMAND_PROMPT "\nCommand (? for help) "
#define GENERIC_FAILURE_MSG "Failed!\n"
#define READ_FAILURE_MSG "Read failure.\n"
#define SWAP_BATTERY_PROMPT "Are you sure you want to change the battery (Y/N)?: "
#define SWAP_BATTERY_STATE_PROMPT "Is the new battery fully charged (Y) (N if it is discharged)?: "
#define SWAP_BATTERY_CNF_MSG "Battery data updated.\n"
#define BATTERY_CAPACITY 2200

/*
 * STATIC FUNCTION PROTOTYPES
 */

static Bool commandHelp (void);
static Bool commandExit (void);
static Bool displayCurrents (void);
static Bool displayVoltages (void);
static Bool displayRemainingCapacities (void);
static Bool displayLifetimeChargesDischarges (void);
static Bool swapRioBatteryCnf (void);
static Bool swapO1BatteryCnf (void);
static Bool swapO2BatteryCnf (void);
static Bool swapO3BatteryCnf (void);
static Bool performCalAllBatteryMonitorsCnf ();

/*
 * TYPES
 */

/* Struct to hold a command */
typedef struct CommandTag
{
	UInt8 commandString[MAX_NUM_CHARS_IN_COMMAND];
	Bool (*commandPtr) (void);
	Char *pDescription;
} Command;

/*
 * GLOBALS (prefixed with g)
 */

/* Array of possible commands.  All command strings MUST be unique.
 * Try not to begin any with a Y or N to avoid accidental cross-over with
 * Y/N confirmation prompts. */
Command gCommandList[] = {{"?", &commandHelp, "display command help"},
						  {"X", &commandExit, "exit this program"},
						  {"I", &displayCurrents, "display current readings"},
 						  {"V", &displayVoltages, "display Voltage readings"},
 						  {"A", &displayRemainingCapacities, "display the accumulated remaining capacities"},
 						  {"L", &displayLifetimeChargesDischarges, "display the lifetime charge/discharge accumulators"},
 						  {"Q", &performCalAllBatteryMonitorsCnf, "calibrate battery monitors"},
                          {"SP", &swapRioBatteryCnf, "swap the battery connected to the RIO/Pi"},
                          {"S1", &swapO1BatteryCnf, "swap the battery connected to the O1"},
                          {"S2", &swapO2BatteryCnf, "swap the battery connected to the O2"},
                          {"S3", &swapO3BatteryCnf, "swap the battery connected to the O3"},
 						  {"PR!", &togglePiRst, "toggle reset to the Pi"},  /* Keep gIndexOfFirstSwitchCommand pointed at here and then we will give user feedback of the result */
 						  {"HP!", &toggleOPwr, "toggle power to the hindbrain (AKA Orangutan)"},
 						  {"HR!", &toggleORst, "toggle reset to the hindbrain (AKA Orangutan)"},
 						  {"PB+", &setRioPwrBattOn, "switch RIO/Pi battery power on"},
 						  {"PB-", &setRioPwrBattOff, "switch RIO/Pi battery power off"},
 						  {"PM+", &setRioPwr12VOn, "switch RIO/Pi mains power on"},
 						  {"PM-", &setRioPwr12VOff, "switch RIO/Pi mains power off"},
 						  {"HB+", &setOPwrBattOn, "switch hindbrain (AKA Orangutan) battery power on"},
 						  {"HB-", &setOPwrBattOff, "switch hindbrain (AKA Orangutan) battery power off"},
 						  {"HM+", &setOPwr12VOn, "switch hindbrain (AKA Orangutan) mains power on"},
 						  {"HM-", &setOPwr12VOff, "switch hindbrain (AKA Orangutan) mains power off"},
 						  {"C*+", &setAllBatteryChargersOn, "switch all chargers on"},
 						  {"C*-", &setAllBatteryChargersOff, "switch all chargers off"},
 						  {"CP+", &setRioBatteryChargerOn, "switch RIO/Pi charger on"},
 						  {"CP-", &setRioBatteryChargerOff, "switch RIO/Pi charger off"},
 						  {"CH+", &setAllOChargersOn, "switch all hindbrain chargers on"},
 						  {"CH-", &setAllOChargersOff, "switch all hindbrain chargers off"},
 						  {"C1+", &setO1BatteryChargerOn, "switch hindbrain charger 1 on"},
 						  {"C1-", &setO1BatteryChargerOff, "switch hindbrain charger 1 off"},
 						  {"C2+", &setO2BatteryChargerOn, "switch hindbrain charger 2 on"},
 						  {"C2-", &setO2BatteryChargerOff, "switch hindbrain charger 2 off"},
 						  {"C3+", &setO3BatteryChargerOn, "switch hindbrain charger 3 on"},
 						  {"C3-", &setO3BatteryChargerOff, "switch hindbrain charger 3 off"}};

UInt8 gIndexOfExitCommand = 1;         /* Keep this pointed at the "X" command entry above */
UInt8 gIndexOfFirstSwitchCommand = 7;

/*
 * STATIC FUNCTIONS
 */

/*
 * Print the possible commands
 *
 * @return  true.
 */
static Bool commandHelp (void)
{
	UInt8 i;

	printf ("\n");
	for (i = 0; i < (sizeof (gCommandList) / sizeof (Command)) - 1; i++)
	{
		printf (" %s %s,\n", gCommandList[i].commandString, gCommandList[i].pDescription);
	}
    printf (" %s %s.\n", gCommandList[i].commandString, gCommandList[i].pDescription); /* A full stop on the last one */

	return true;
}

/*
 * A dummy command for exit
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
 * @return  true if successful, otherwise false.
 */
static Bool displayCurrents (void)
{
    Bool success = true;
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
        printf ("Currents (mA): Pi/RIO %d, O* %d, O1 %d, O2 %d, O3 %d (-ve is discharge).\n", rioCurrent, o1Current + o2Current + o3Current, o1Current, o2Current, o3Current);                
    }
    else
    {
        printf (READ_FAILURE_MSG);
    }
        
    return success;    
}

/*
 * Display the Voltage readings from all
 * the batteries.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayVoltages (void)
{
    Bool success = true;
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
        printf ("Voltage (mV): Pi/RIO %u, O1 %u, O2 %u, O3 %4u.\n", rioVoltage, o1Voltage, o2Voltage, o3Voltage);                
    }
    else
    {
        printf (READ_FAILURE_MSG);
    }
        
    return success;    
}

/*
 * Display the accumulated remaining capacity
 * readings from all the battery monitors.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayRemainingCapacities (void)
{
    Bool success = true;
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
        printf ("Remaining capacity (mAhr): Pi/RIO %u, O* %u, O1 %u, O2 %u, O3 %u.\n", rioRemainingCapacity, o1RemainingCapacity + o2RemainingCapacity + o3RemainingCapacity, o1RemainingCapacity, o2RemainingCapacity, o3RemainingCapacity);                
    }
    else
    {
        printf (READ_FAILURE_MSG);
    }
        
    return success;    
}

/*
 * Display the lifetime charge/discharge accumulator
 * readings from all the battery monitors.
 *
 * @return  true if successful, otherwise false.
 */
static Bool displayLifetimeChargesDischarges (void)
{
    Bool success = true;
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
        printf ("Charge (mAhr):    Pi/RIO %lu, O1 %lu, O2 %lu, O3 %lu.\n", rioLifetimeCharge, o1LifetimeCharge, o2LifetimeCharge, o3LifetimeCharge);                
        printf ("Discharge (mAhr): Pi/RIO %lu, O1 %lu, O2 %lu, O3 %lu.\n", rioLifetimeDischarge, o1LifetimeDischarge, o2LifetimeDischarge, o3LifetimeDischarge);                
    }
    else
    {
        printf (READ_FAILURE_MSG);
    }
        
    return success;    
}

/*
 * Perform a calibration on all battery monitors
 * but wait for a confirm key to be pressed first
 * as this is something that the user has to have
 * prepared for and shouldn't be run accidentally.
 *
 * @return  true if successful, otherwise false.
 */
static bool performCalAllBatteryMonitorsCnf (void)
{
    Bool success = true;
    
    /* Prompt for input */
    if (getYesInput ("Are all the current measurements resistors shorted (Y/N)? "))
    {
        success = performCalAllBatteryMonitors();
        if (success)
        {
            printf ("Calibration completed, don't forget to remove the shorts.\n");
        }
        else
        {
            printf (GENERIC_FAILURE_MSG);
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
 * @return  true if successful, otherwise false.
 */
static Bool swapRioBatteryCnf (void)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (SWAP_BATTERY_PROMPT))
        {
            /* Check that the new battery is fully charged */
            if (getYesInput (SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            
            /* Now do the swap */
            success = swapRioBattery (timeTicks, remainingCapacity);
        }
    }
    else
    {
        success = false;
    }

    if (success)
    {
        printf (SWAP_BATTERY_CNF_MSG);
    }
    else
    {
        printf (GENERIC_FAILURE_MSG);
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
 * @return  true if successful, otherwise false.
 */
static Bool swapO1BatteryCnf (void)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (SWAP_BATTERY_PROMPT))
        {
            /* Check that the new battery is fully charged */
            if (getYesInput (SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            
            /* Now do the swap */
            success = swapO1Battery (timeTicks, remainingCapacity);
        }
    }
    else
    {
        success = false;
    }
    
    if (success)
    {
        printf (SWAP_BATTERY_CNF_MSG);
    }
    else
    {
        printf (GENERIC_FAILURE_MSG);
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
 * @return  true if successful, otherwise false.
 */
static Bool swapO2BatteryCnf (void)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (SWAP_BATTERY_PROMPT))
        {
            /* Check that the new battery is fully charged */
            if (getYesInput (SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            
            /* Now do the swap */
            success = swapO2Battery (timeTicks, remainingCapacity);
        }
    }
    else
    {
        success = false;
    }
    
    if (success)
    {
        printf (SWAP_BATTERY_CNF_MSG);
    }
    else
    {
        printf (GENERIC_FAILURE_MSG);
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
 * @return  true if successful, otherwise false.
 */
static Bool swapO3BatteryCnf (void)
{
    Bool success = true;
    UInt16 remainingCapacity = 0;
    UInt32 timeTicks;
    
    timeTicks = getSystemTicks();
    if (timeTicks != 0)
    {
        if (getYesInput (SWAP_BATTERY_PROMPT))
        {
            /* Check that the new battery is fully charged */
            if (getYesInput (SWAP_BATTERY_STATE_PROMPT))
            {
                remainingCapacity = BATTERY_CAPACITY;
            }
            
            /* Now do the swap */
            success = swapO3Battery (timeTicks, remainingCapacity);
        }
    }
    else
    {
        success = false;
    }

    if (success)
    {
        printf (SWAP_BATTERY_CNF_MSG);
    }
    else
    {
        printf (GENERIC_FAILURE_MSG);
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
 * Act on a backspace character
 */
static void actOnBackspace(UInt8 *pEntry, UInt8 *pNumEntries)
{
	*pEntry = 0;
	(*pNumEntries)--;
	printf (SCREEN_BACKSPACE);
	putchar (' ');
    printf (SCREEN_BACKSPACE);
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Get a Yes input.  Note that this does
 * its own terminal settings stuff so that
 * it can be called from anywhere.
 * 
 * pPrompt  a string to print as a prompt,
 *          may be PNULL.
 *
 * @return  true if "Y" was pressed, otherwise false.
 * 
 */
Bool getYesInput (Char *pPrompt)
{
    Bool answerIsYes = false;
    struct termios savedSettings;
    struct termios newSettings;

    /* Check if this is a TTY device */
    if (tcgetattr (STDIN_FILENO, &savedSettings) == 0)
    {
        /* Save current settings and then set us up for no echo, non-canonical (i.e. no need for enter) */
        memcpy (&newSettings, &savedSettings, sizeof (newSettings));
        newSettings.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
        tcsetattr (STDIN_FILENO, TCSANOW, &newSettings);
        
        /* Prompt for input and get the answer */
        if (pPrompt != PNULL)
        {
            printf (pPrompt);
        }
        
        if (toUpper(getchar()) == 'Y')
        {
            answerIsYes = true;
        }

        /* Restore saved settings */
        tcsetattr (STDIN_FILENO, TCSANOW, &savedSettings);
    }
    
    return answerIsYes;
}

/*
 * Run an interactive menu system.
 */
Bool runMenu (void)
{
    Bool success = true;
    Bool done = false;
	UInt8 numEntries = 0;
	UInt8 entry[MAX_NUM_CHARS_IN_COMMAND];
    UInt8 key;
	UInt8 i;
	struct termios savedSettings;
    struct termios newSettings;

    /* Check if this is a TTY device */
    if (tcgetattr (STDIN_FILENO, &savedSettings) == 0)
    {
    	if (success)
    	{
    		/* Save current settings and then set us up for no echo, non-canonical (i.e. no need for enter) */
    		memcpy (&newSettings, &savedSettings, sizeof (newSettings));
    		newSettings.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    		tcsetattr (STDIN_FILENO, TCSANOW, &newSettings);
    
    		/* Prompt for input */
    		printf (COMMAND_PROMPT);
    
    		/* Run the interactive bit */
    		while (!done)
    		{
    			key = getchar();
    			switch (key)
    			{
    				case KEY_COMMAND_CANCEL:
    				{
    	                /* Reset the number of entries and go fully backwards on the screen */
    					for (i = 0; i < numEntries; i++)
    					{
    						actOnBackspace (&entry[i], &numEntries);
    					}
    				}
    				break;
    				case KEY_BACKSPACE:
    				{
    	                /* Set last entry to zero and go back one on the screen */
    					if (numEntries > 0)
    					{
    						actOnBackspace (&entry[numEntries], &numEntries);
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
    						/* Found a possible command so display the character */
    						putchar (entry[numEntries - 1]);
    						if (numFound == 1)
    						{
    							/* There is only one command that this could be so call the function */
    							if (gCommandList[lastCommandMatching].commandPtr)
    							{
    							    printf (", %s...\n", gCommandList[lastCommandMatching].pDescription);
    								success = gCommandList[lastCommandMatching].commandPtr();
    								
                                    /* Give some feedback and prompt for more input, or exit */
    								if (lastCommandMatching != gIndexOfExitCommand)
    								{
                                        numEntries = 0;
                                        if ((lastCommandMatching >= gIndexOfFirstSwitchCommand) && success)
                                        {
                                            printf ("Done.\n");
                                        }
                                        printf (COMMAND_PROMPT);
    								}
    								else
    								{        
                                        done = true;
    								}
    							}
    						}
    					}
    				}
    				break;
    			}
    		}
    
            /* Restore saved settings, hopefully */
            tcsetattr (STDIN_FILENO, TCSANOW, &savedSettings);
    	}
    }
    else
    {
        success = false;      
    }

    return success;
}