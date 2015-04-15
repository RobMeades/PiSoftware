/*
 *  Public stuff for the Hardware server
 */

/*
 * MANIFEST CONSTANTS
 */

#define HARDWARE_SERVER_EXE "./roboone_hardware_server"
#define HARDWARE_SERVER_PORT_STRING "5234"

/* Maximum length of a string sent to or received back from the Orangutan, AKA Hindbrain */
#define MAX_O_STRING_LENGTH         80
/* The string to ping the Hindbrain with to get an "OK" response if it's there */
#define PING_STRING "!\n"
/* Max length of response to the ping string*/
#define O_RESPONSE_STRING_LENGTH 10
/* Checker for a good response to the ping string, the parameter being of type OResponseString */
#define O_CHECK_OK_STRING(PoUTPUTsTRING) (((PoUTPUTsTRING)->stringLength) >= 2) && (((PoUTPUTsTRING)->string[0] == 'O') && ((PoUTPUTsTRING)->string[1] == 'K') ? true : false)  
/* Suggested startup delay for the Orangutan, AKA Hindbrain, before it can be pinged */
#define O_START_DELAY_US 100000L
/* The maximum remaininc capacity of a battery (to cap any over-optimistic readings from the monitoring chips) */
#define MAX_REMAINING_CAPACITY_MAH 2200

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE TYPES
 */

/* The states of charge that a charger can be in */
typedef enum ChargeStateTag
{
    CHARGE_STATE_NO_POWER = 0,
    CHARGE_STATE_OFF = 1,
    CHARGE_STATE_GREEN = 2,
    CHARGE_STATE_FLASHING_GREEN = 3,
    CHARGE_STATE_RED = 4,
    CHARGE_STATE_FLASHING_RED = 5,
    NUM_CHARGE_STATES,
    CHARGE_STATE_UNKNOWN,
    CHARGE_STATE_NULL,
    CHARGE_STATE_BAD
} ChargeState;

/*
 * TYPES FOR REQ MESSAGES
 */

typedef struct HardwareBatterySwapDataTag
{
    UInt32 systemTime;
    UInt16 remainingCapacity;
} HardwareBatterySwapData;

/*
 * TYPES FOR CNF MESSAGES
 */

/* The chargers in the system.  Don't mess with these as the values are used
 * to index into arrays. */
typedef enum ChargerTag
{
    CHARGER_RIO = 0,
    CHARGER_O1 = 1,
    CHARGER_O2 = 2,
    CHARGER_O3 = 3,
    NUM_CHARGERS,
    CHARGER_NULL
} Charger;

typedef struct HardwareChargeStateTag
{
    Bool flashDetectPossible;
    ChargeState state[NUM_CHARGERS];
} HardwareChargeState;

typedef struct OResponseStringTag
{
    Char   string[MAX_O_STRING_LENGTH];
    UInt32 stringLength; /* including the terminator which MUST be present */
} OResponseString;

typedef struct OInputStringTag
{
    Bool   waitForResponse;
    Char   string[MAX_O_STRING_LENGTH]; /* Must have a null terminator */
} OInputContainer;

#pragma pack(pop) /* End of packing */ 