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

/*
 *  FUNCTION PROTOTYPES
 */

Bool startOneWireBus (void);
void stopOneWireBus (void);
Bool setupDevices (void);
UInt8 findAllDevices (void);

Bool readMains12VPin (Bool *pMains12VIsPresent);
Bool readChargerStatePins (UInt8 *pPinsState);
Bool readChargerState (ChargeState *pState, Bool *pFlashDetectPossible);
Bool toggleOPwr (void);
Bool readOPwr (Bool *pIsOn);
Bool toggleORst (void);
Bool readORst (Bool *pIsOn);
Bool togglePiRst (void);
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
Bool disableOnPCBRelays (void);
Bool enableOnPCBRelays (void);
Bool readOnPCBRelaysEnabled (Bool *pIsOn);
Bool disableExternalRelays (void);
Bool enableExternalRelays (void);
Bool readExternalRelaysEnabled (Bool *pIsOn);
Bool readGeneralPurposeIOs (UInt8 *pPinsState);
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
Bool performCalO2BatteryMonitor (void);
Bool performCalO3BatteryMonitor (void);
Bool swapRioBattery (UInt32 systemTime, UInt16 remainingCapacity);
Bool swapO1Battery (UInt32 systemTime, UInt16 remainingCapacity);
Bool swapO2Battery (UInt32 systemTime, UInt16 remainingCapacity);
Bool swapO3Battery (UInt32 systemTime, UInt16 remainingCapacity);
Bool setAnalogueMuxInput (UInt8 input);
Bool readAnalogueMux (UInt16 *pVoltage);