/*
 * Stuff to do with the One Wire bus.
 */ 

/*
 * MANIFEST CONSTANTS
 */

#define PIO_MAX_BYTES_TO_READ        32   /* Should be the same as DS2408_MAX_BYTES_IN_CHANNEL_ACCESS */

/*
 * TYPES
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

/* The state that a charger can be in */
typedef enum ChargeStateTag
{
    CHARGE_STATE_OFF = 0,
    CHARGE_STATE_GREEN = 1,
    CHARGE_STATE_FLASHING_GREEN = 2,
    CHARGE_STATE_RED = 3,
    CHARGE_STATE_FLASHING_RED = 4,
    NUM_CHARGE_STATES,
    CHARGE_STATE_NULL,
    CHARGE_STATE_BAD
} ChargeState;

/*
 *  FUNCTION PROTOTYPES
 */

Bool startOneWireBus (void);
void stopOneWireBus (void);
Bool setupDevices (void);
UInt8 findAllDevices (void);

Bool readChargerStatePins (UInt8 *pPinsState);
Bool readChargerState (ChargeState *pState, Bool *pFlashDetectPossible);
Bool toggleOPwr (void);
Bool readOPwr (Bool *pIsOn);
Bool toggleORst (void);
Bool readORst (Bool *pIsOn);
Bool togglePiRst (void);
Bool readPiRst (Bool *pIsOn);
Bool setRioPwr12VOn (void);
Bool setRioPwr12VOff (void);
Bool readRioPwr12V (Bool *pIsOn);
Bool setRioPwrBattOn (void);
Bool setRioPwrBattOff (void);
Bool readRioPwrBatt (Bool *pIsOn);
Bool setOPwr12VOn (void);
Bool setOPwr12VOff (void);
Bool readOPwr12V (Bool *pIsOn);
Bool setOPwrBattOn (void);
Bool setOPwrBattOff (void);
Bool readOPwrBatt (Bool *pIsOn);
Bool setRioBatteryChargerOn (void);
Bool setRioBatteryChargerOff (void);
Bool readRioBatteryCharger (Bool *pIsOn);
Bool setO1BatteryChargerOn (void);
Bool setO1BatteryChargerOff (void);
Bool readO1BatteryCharger (Bool *pIsOn);
Bool setO2BatteryChargerOn (void);
Bool setO2BatteryChargerOff (void);
Bool readO2BatteryCharger (Bool *pIsOn);
Bool setO3BatteryChargerOn (void);
Bool setO3BatteryChargerOff (void);
Bool readO3BatteryCharger (Bool *pIsOn);
Bool setAllBatteryChargersOn (void);
Bool setAllBatteryChargersOff (void);
Bool setAllOChargersOn (void);
Bool setAllOChargersOff (void);
Bool disableAllRelays (void);
Bool enableAllRelays (void);
Bool readRelaysEnabled (Bool *pIsOn);
Bool readRioBattCurrent (SInt16 *pCurrent);
Bool readO1BattCurrent (SInt16 *pCurrent);
Bool readO2BattCurrent (SInt16 *pCurrent);
Bool readO3BattCurrent (SInt16 *pCurrent);
Bool readRioBattVoltage (UInt16 *pVoltage);
Bool readO1BattVoltage (UInt16 *pVoltage);
Bool readO2BattVoltage (UInt16 *pVoltage);
Bool readO3BattVoltage (UInt16 *pVoltage);
Bool readRioRemainingCapacity (UInt16 *pRemainingCapacity);
Bool readO1RemainingCapacity (UInt16 *pRemainingCapacity);
Bool readO2RemainingCapacity (UInt16 *pRemainingCapacity);
Bool readO3RemainingCapacity (UInt16 *pRemainingCapacity);
Bool readRioBattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge);
Bool readO1BattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge);
Bool readO2BattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge);
Bool readO3BattLifetimeChargeDischarge (UInt32 *pCharge, UInt32 *pDischarge);
Bool performCalAllBatteryMonitors (void);
Bool performCalRioBatteryMonitor (void);
Bool performCalO1BatteryMonitor (void);
Bool performCalO2BattMonitor (void);
Bool performCalO3BattMonitor (void);
Bool swapRioBattery (UInt32 systemTime, UInt16 remainingCapacity);
Bool swapO1Battery (UInt32 systemTime, UInt16 remainingCapacity);
Bool swapO2Battery (UInt32 systemTime, UInt16 remainingCapacity);
Bool swapO3Battery (UInt32 systemTime, UInt16 remainingCapacity);
Bool setAnalogueMuxInput (UInt8 input);
Bool readAnalogueMux (UInt16 *pVoltage);