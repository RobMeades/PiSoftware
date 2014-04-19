/*
 *  Public stuff for the OneWire server
 */

/*
 * MANIFEST CONSTANTS
 */

#define HARDWARE_SERVER_EXE "./roboone_hardware_server"
#define HARDWARE_SERVER_PORT_STRING "5234"

/* Maximum length of a string sent to or receied back from the Orangutan, AKA Hindbrain */
#define MAX_O_STRING_LENGTH         80

/*
 * TYPES
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/* The state that a charger can be in */
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
 * GENERAL MESSAGE TYPES
 */

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

/* The chargers in the system */
typedef enum ChargerTag
{
    CHARGER_RIO = 0,
    CHARGER_O1 = 1,
    CHARGER_O2 = 2,
    CHARGER_O3 = 3,
    NUM_CHARGERS,
    CHARGER_NULL
} Charger;

typedef struct HardwareReadChargerStateTag
{
    ChargeState state[NUM_CHARGERS];
    Bool flashDetectPossible;
} HardwareReadChargerState;

typedef struct HardwareChargeDischargeTag
{
    UInt32 charge;
    UInt32 discharge;
} HardwareChargeDischarge;

typedef struct OStringTag
{
    Char   string[MAX_O_STRING_LENGTH];
    UInt32 stringLength; /* including the terminator which MUST be present */
} OString;

typedef struct OInputStringTag
{
    Char   string[MAX_O_STRING_LENGTH]; /* Must have a null terminator */
    Bool   waitForResponse;
} OInputString;

#pragma pack(pop) /* End of packing */ 