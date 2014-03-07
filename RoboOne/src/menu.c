#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <rob_system.h>
#include <ow_bus.h>


/*
 * MANIFEST CONSTANTS
 */

#define MAX_NUM_CHARS_IN_COMMAND 4 /* including a null terminator */
#define KEY_COMMAND_CANCEL '\x1b'   /* the escape key */
#define KEY_BACKSPACE '\x08'
#define COMMAND_PROMPT "Enter a command (? for help): "

/*
 * STATIC FUNCTION PROTOTYPES
 */

static Bool commandHelp (void);
static Bool commandExit (void);
static Bool commandNull (void);

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

/* Array of possible commands.  All commands must be unique */
Command gCommandList[] = {{"", &commandNull, "Null entry."},
	                      {"?", &commandHelp, "Display command help."},
						  {"X", &commandExit, "Exit this program."},
						  {"I", &commandNull, "Display the current reading for all batteries."},
 						  {"V", &commandNull, "Display the Voltage reading for all batteries."},
 						  {"A", &commandNull, "Read the integrated current accumulators for all batteries."},
 						  {"L", &commandNull, "Read the lifetime charge and discharge accumulators for all batteries."},
 						  {"QC!", &performCalAllBatteryMonitors, "Calibrate all battery monitors (ALL batteries must be shorted!)."},
 						  {"PR!", &togglePiRst, "Toggle reset to the Pi."},
 						  {"HP!", &toggleOPwr, "Toggle power to the hindbrain (AKA Orangutan)."},
 						  {"HR!", &toggleORst, "Toggle reset to the hindbrain (AKA Orangutan)."},
 						  {"PB+", &setRioPwrBattOn, "Switch RIO/Pi battery power on."},
 						  {"PB-", &setRioPwrBattOff, "Switch RIO/Pi battery power off."},
 						  {"PM+", &setRioPwr12VOn, "Switch RIO/Pi mains power on."},
 						  {"PM-", &setRioPwr12VOff, "Switch RIO/Pi mains power off."},
 						  {"HB+", &setOPwrBattOn, "Switch hindbrain (AKA Orangutan) battery power on."},
 						  {"HB-", &setOPwrBattOff, "Switch hindbrain (AKA Orangutan) battery power off."},
 						  {"HM+", &setOPwr12VOn, "Switch hindbrain (AKA Orangutan) mains power on."},
 						  {"HM-", &setOPwr12VOff, "Switch hindbrain (AKA Orangutan) mains power off."},
 						  {"C*+", &setAllBatteryChargersOn, "Switch all chargers on."},
 						  {"C*-", &setAllBatteryChargersOff, "Switch all chargers off."},
 						  {"CP+", &setRioBatteryChargerOn, "Switch RIO/Pi charger on."},
 						  {"CP-", &setRioBatteryChargerOff, "Switch RIO/Pi charger off."},
 						  {"CH+", &setAllOChargersOn, "Switch all hindbrain chargers on."},
 						  {"CH-", &setAllOChargersOff, "Switch all hindbrain chargers off."},
 						  {"C1+", &setO1BatteryChargerOn, "Switch hindbrain charger 1 on."},
 						  {"C1-", &setO1BatteryChargerOff, "Switch hindbrain charger 1 off."},
 						  {"C2+", &setO2BatteryChargerOn, "Switch hindbrain charger 2 on."},
 						  {"C2-", &setO2BatteryChargerOff, "Switch hindbrain charger 2 off."},
 						  {"C3+", &setO3BatteryChargerOn, "Switch hindbrain charger 3 on."},
 						  {"C3-", &setO3BatteryChargerOff, "Switch hindbrain charger 3 off."}};

/*
 * STATIC FUNCTIONS
 */

/*
 * A Null command as the pedantic compiler setting doesn't
 * allow me to use PNULL for some reason
 */
static Bool commandNull (void)
{
    printf ("Not implemented.\n");
    return true;
}

/*
 * Print the possible commands
 */
static Bool commandHelp (void)
{
	UInt8 i;

	for (i = 0; i < sizeof (gCommandList) / sizeof (Command); i++)
	{
		printf (" %s: %s\n", gCommandList[i].commandString, gCommandList[i].pDescription);
	}
	printf (COMMAND_PROMPT);

	return true;
}

/*
 * Just return false to exit
 */
static Bool commandExit (void)
{
	return false;
}

/*
 * Simple conversion to upper case
 */
static UInt8 toUpper (UInt8 digit)
{
	if (digit >= 'a' && digit <= 'z')
	{
		digit |= 0x20;
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
	putchar (KEY_BACKSPACE);
	putchar (' ');
	putchar (KEY_BACKSPACE);
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Run an interactive menu system.
 */
Bool runMenu (void)
{
    Bool success = true;
	UInt8 numEntries = 0;
	UInt8 entry[MAX_NUM_CHARS_IN_COMMAND];
    UInt8 key;
	UInt8 i;
	struct termios savedSettings;
    struct termios newSettings;

    success = tcgetattr (STDIN_FILENO, &savedSettings);
	if (success)
	{
		/* Save current settings and then set us up for no echo, non-canonical (i.e. no need for enter) */
		memcpy (&newSettings, &savedSettings, sizeof (newSettings));
		newSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
		tcsetattr (STDIN_FILENO, TCSANOW, &newSettings);

		/* Prompt for input */
		printf (COMMAND_PROMPT);

		/* Run the interactive bit */
		while (success)
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
					UInt8 lastCommandFound = 0;
					UInt8 entryPos = 0;

					entry[numEntries] = toUpper (key);
					numEntries++;
					do
					{
						numFound = 0;
					    lastCommandFound = 0;
						for (i = 0; i < (sizeof (gCommandList) / sizeof (Command)); i++)
						{
							if (gCommandList[i].commandString[entryPos] == entry[entryPos])
							{
								numFound++;
								lastCommandFound = i;
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
						entry[numEntries] = 0;
						numEntries--;
					}
					else
					{
						/* Found a possible command so display the character */
						putchar (entry[numEntries]);
						if (numFound == 1)
						{
							/* There is only one command that this could be so call the function */
							if (gCommandList[lastCommandFound].commandPtr)
							{
								success = gCommandList[lastCommandFound].commandPtr();

								/* Remove the input line and await the next command */
								for (i = 0; i < numEntries; i++)
								{
									actOnBackspace (&entry[i], &numEntries);
								}
							}
						}
					}
				}
				break;
			}
		}

		/* Restore saved settings */
	    success = tcsetattr (STDIN_FILENO, TCSANOW, &savedSettings);
	}

    return success;
}