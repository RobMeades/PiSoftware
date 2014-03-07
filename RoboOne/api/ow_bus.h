/*
 * Public stuff to do with the One Wire bus.
 */ 

Bool startOneWireBus (void);
void stopOneWireBus (void);
Bool setupDevices (void);
UInt8 findAllDevices (void);

Bool toggleOPwr (void);
Bool toggleORst (void);
Bool togglePiRst (void);
Bool setRioPwr12VOn (void);
Bool setRioPwr12VOff (void);
Bool setRioPwrBattOn (void);
Bool setRioPwrBattOff (void);
Bool setOPwr12VOn (void);
Bool setOPwr12VOff (void);
Bool setOPwrBattOn (void);
Bool setOPwrBattOff (void);
Bool setRioBatteryChargerOn (void);
Bool setRioBatteryChargerOff (void);
Bool setO1BatteryChargerOn (void);
Bool setO1BatteryChargerOff (void);
Bool setO2BatteryChargerOn (void);
Bool setO2BatteryChargerOff (void);
Bool setO3BatteryChargerOn (void);
Bool setO3BatteryChargerOff (void);
Bool setAllBatteryChargersOn (void);
Bool setAllBatteryChargersOff (void);
Bool setAllOChargersOn (void);
Bool setAllOChargersOff (void);
Bool readRioBattCurrent (SInt16 *pCurrent);
Bool readO1BattCurrent (SInt16 *pCurrent);
Bool readO2BattCurrent (SInt16 *pCurrent);
Bool readO3BattCurrent (SInt16 *pCurrent);
Bool performCalAllBatteryMonitors (void);
Bool performCalRioBatteryMonitor (void);
Bool performCalO1BatteryMonitor (void);
Bool performCalO2BattMonitor (void);
Bool performCalO3BattMonitor (void);