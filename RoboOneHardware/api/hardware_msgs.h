/*
 *  Messages that go from/to the hardware server.
 */

/* This file is included in the hardware_msg_auto header and
 * defines the message structures for the messages sent to
 * and from the server.  The items in the list are:
 * 
 * - the message type enum,
 * - the message typedef struct (without a Cnf or Req on the end),
 * - the message variable name for use in the message union
 *   (again, without a Cnf or Req on the end),
 * - the message member that is needed in the Req message structure
 *   beyond the mandatory msgHeader (which is added automatagically),
 * - the message member that is needed in the Cnf message structure.
 */

/*
 * Definitions
 */
HARDWARE_MSG_DEF (HARDWARE_SERVER_START, HardwareServerStart, hardwareServerStart, Bool batteriesOnly, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SERVER_STOP, HardwareServerStop, hardwareServerStop, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_MAINS_12V, HardwareReadMains12V, hardwareReadMains12V, HARDWARE_EMPTY, Bool mains12VIsPresent)
HARDWARE_MSG_DEF (HARDWARE_READ_CHARGER_STATE_PINS, HardwareReadChargerStatePins, hardwareReadChargerStatePins, HARDWARE_EMPTY, UInt8 pinsState)
HARDWARE_MSG_DEF (HARDWARE_READ_CHARGER_STATE, HardwareReadChargerState, hardwareReadChargerState, HARDWARE_EMPTY, HardwareChargeState chargeState)
HARDWARE_MSG_DEF (HARDWARE_TOGGLE_O_PWR, HardwareToggleOPwr, hardwareToggleOPwr, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_O_PWR, HardwareReadOPwr, hardwareReadOPwr, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_TOGGLE_O_RST, HardwareToggleORst, hardwareToggleORst, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_O_RST, HardwareReadORst, hardwareReadORst, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_TOGGLE_PI_RST, HardwareTogglePiRst, hardwareTogglePiRst, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_RIO_PWR_12V_ON, HardwareSetRioPwr12VOn, hardwareSetRioPwr12VOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_RIO_PWR_12V_OFF, HardwareSetRioPwr12VOff, hardwareSetRioPwr12VOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_PWR_12V, HardwareReadRioPwr12V, hardwareReadRioPwr12V, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_RIO_PWR_BATT_ON, HardwareSetRioPwrBattOn, hardwareSetRioPwrBattOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_RIO_PWR_BATT_OFF, HardwareSetRioPwrBattOff, hardwareSetRioPwrBattOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_PWR_BATT, HardwareReadRioPwrBatt, hardwareReadRioPwrBatt, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_O_PWR_12V_ON, HardwareSetOPwr12VOn, hardwareSetOPwr12VOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_O_PWR_12V_OFF, HardwareSetOPwr12VOff, hardwareSetOPwr12VOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_O_PWR_12V, HardwareReadOPwr12V, hardwareReadOPwr12V, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_O_PWR_BATT_ON, HardwareSetOPwrBattOn, hardwareSetOPwrBattOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_O_PWR_BATT_OFF, HardwareSetOPwrBattOff, hardwareSetOPwrBattOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_O_PWR_BATT, HardwareReadOPwrBatt, hardwareReadOPwrBatt, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_RIO_BATTERY_CHARGER_ON, HardwareSetRioBatteryChargerOn, hardwareSetRioBatteryChargerOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_RIO_BATTERY_CHARGER_OFF, HardwareSetRioBatteryChargerOff, hardwareSetRioBatteryChargerOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_BATTERY_CHARGER, HardwareReadRioBatteryCharger, hardwareReadRioBatteryCharger, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_O1_BATTERY_CHARGER_ON, HardwareSetO1BatteryChargerOn, hardwareSetO1BatteryChargerOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_O1_BATTERY_CHARGER_OFF, HardwareSetO1BatteryChargerOff, hardwareSetO1RioBatteryChargerOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_O1_BATTERY_CHARGER, HardwareReadO1BatteryCharger, hardwareReadO1BatteryCharger, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_O2_BATTERY_CHARGER_ON, HardwareSetO2BatteryChargerOn, hardwareSetO2BatteryChargerOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_O2_BATTERY_CHARGER_OFF, HardwareSetO2BatteryChargerOff, hardwareSetO2RioBatteryChargerOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_O2_BATTERY_CHARGER, HardwareReadO2BatteryCharger, hardwareReadO2BatteryCharger, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_O3_BATTERY_CHARGER_ON, HardwareSetO3BatteryChargerOn, hardwareSetO3BatteryChargerOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_O3_BATTERY_CHARGER_OFF, HardwareSetO3BatteryChargerOff, hardwareSetO3RioBatteryChargerOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_O3_BATTERY_CHARGER, HardwareReadO3BatteryCharger, hardwareReadO3BatteryCharger, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_SET_ALL_BATTERY_CHARGERS_ON, HardwareSetAllBatteryChargersOn, hardwareSetAllBatteryChargersOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_ALL_BATTERY_CHARGERS_OFF, HardwareSetAllBatteryChargersOff, hardwareSetAllBatteryChargersOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_ALL_O_BATTERY_CHARGERS_ON, HardwareSetAllOBatteryChargersOn, hardwareSetAllOBatteryChargersOn, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SET_ALL_O_BATTERY_CHARGERS_OFF, HardwareSetAllOBatteryChargersOff, hardwareSetAllOBatteryChargersOff, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_DISABLE_ON_PCB_RELAYS, HardwareDisableOnPCBRelays, hardwareDisableOnPCBRelays, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_ENABLE_ON_PCB_RELAYS, HardwareEnableOnPCBRelays, hardwareEnableOnPCBRelays, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_DISABLE_EXTERNAL_RELAYS, HardwareDisableExternalRelays, hardwareDisableExternalRelays, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_ENABLE_EXTERNAL_RELAYS, HardwareEnableExternalRelays, hardwareEnableExternalRelays, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_EXTERNAL_RELAYS_ENABLED, HardwareReadExternalRelaysEnabled, hardwareReadExternalRelaysEnabled, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_READ_ON_PCB_RELAYS_ENABLED, HardwareReadOnPCBRelaysEnabled, hardwareReadOnPCBRelaysEnabled, HARDWARE_EMPTY, Bool isOn)
HARDWARE_MSG_DEF (HARDWARE_READ_GENERAL_PURPOSE_IOS, HardwareReadGeneralPurposeIOs, hardwareReadGeneralPurposeIOs, HARDWARE_EMPTY, UInt8 pinsState)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_BATT_CURRENT, HardwareReadRioBattCurrent, hardwareReadRioBattCurrent, HARDWARE_EMPTY, SInt16 current)
HARDWARE_MSG_DEF (HARDWARE_READ_O1_BATT_CURRENT, HardwareReadO1BattCurrent, hardwareReadO1BattCurrent, HARDWARE_EMPTY, SInt16 current)
HARDWARE_MSG_DEF (HARDWARE_READ_O2_BATT_CURRENT, HardwareReadO2BattCurrent, hardwareReadO2BattCurrent, HARDWARE_EMPTY, SInt16 current)
HARDWARE_MSG_DEF (HARDWARE_READ_O3_BATT_CURRENT, HardwareReadO3BattCurrent, hardwareReadO3BattCurrent, HARDWARE_EMPTY, SInt16 current)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_BATT_VOLTAGE, HardwareReadRioBattVoltage, hardwareReadRioBattVoltage, HARDWARE_EMPTY, UInt16 voltage)
HARDWARE_MSG_DEF (HARDWARE_READ_O1_BATT_VOLTAGE, HardwareReadO1BattVoltage, hardwareReadO1BattVoltage, HARDWARE_EMPTY, UInt16 voltage)
HARDWARE_MSG_DEF (HARDWARE_READ_O2_BATT_VOLTAGE, HardwareReadO2BattVoltage, hardwareReadO2BattVoltage, HARDWARE_EMPTY, UInt16 voltage)
HARDWARE_MSG_DEF (HARDWARE_READ_O3_BATT_VOLTAGE, HardwareReadO3BattVoltage, hardwareReadO3BattVoltage, HARDWARE_EMPTY, UInt16 voltage)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_REMAINING_CAPACITY, HardwareReadRioRemainingCapacity, hardwareReadRioRemainingCapacity, HARDWARE_EMPTY, UInt16 remainingCapacity)
HARDWARE_MSG_DEF (HARDWARE_READ_O1_REMAINING_CAPACITY, HardwareReadO1RemainingCapacity, hardwareReadO1RemainingCapacity, HARDWARE_EMPTY, UInt16 remainingCapacity)
HARDWARE_MSG_DEF (HARDWARE_READ_O2_REMAINING_CAPACITY, HardwareReadO2RemainingCapacity, hardwareReadO2RemainingCapacity, HARDWARE_EMPTY, UInt16 remainingCapacity)
HARDWARE_MSG_DEF (HARDWARE_READ_O3_REMAINING_CAPACITY, HardwareReadO3RemainingCapacity, hardwareReadO3RemainingCapacity, HARDWARE_EMPTY, UInt16 remainingCapacity)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_BATT_LIFETIME_CHARGE_DISCHARGE, HardwareReadRioBattLifetimeChargeDischarge, hardwareReadRioBattLifetimeChargeDischarge, HARDWARE_EMPTY, HardwareChargeDischarge chargeDischarge)
HARDWARE_MSG_DEF (HARDWARE_READ_O1_BATT_LIFETIME_CHARGE_DISCHARGE, HardwareReadO1BattLifetimeChargeDischarge, hardwareReadO1BattLifetimeChargeDischarge, HARDWARE_EMPTY, HardwareChargeDischarge chargeDischarge)
HARDWARE_MSG_DEF (HARDWARE_READ_O2_BATT_LIFETIME_CHARGE_DISCHARGE, HardwareReadO2BattLifetimeChargeDischarge, hardwareReadO2BattLifetimeChargeDischarge, HARDWARE_EMPTY, HardwareChargeDischarge chargeDischarge)
HARDWARE_MSG_DEF (HARDWARE_READ_O3_BATT_LIFETIME_CHARGE_DISCHARGE, HardwareReadO3BattLifetimeChargeDischarge, hardwareReadO3BattLifetimeChargeDischarge, HARDWARE_EMPTY, HardwareChargeDischarge chargeDischarge)
HARDWARE_MSG_DEF (HARDWARE_PERFORM_CAL_ALL_BATTERY_MONITORS, HardwarePerformCalAllBatteryMonitors, hardwarePerformCalAllBatteryMonitors, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_PERFORM_CAL_RIO_BATTERY_MONITOR, HardwarePerformCalRioBatteryMonitor, hardwarePerformCalRioBatteryMonitor, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_PERFORM_CAL_O1_BATTERY_MONITOR, HardwarePerformCalO1BatteryMonitor, hardwarePerformCalO1BatteryMonitor, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_PERFORM_CAL_O2_BATTERY_MONITOR, HardwarePerformCalO2BatteryMonitor, hardwarePerformCalO2BatteryMonitor, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_PERFORM_CAL_O3_BATTERY_MONITOR, HardwarePerformCalO3BatteryMonitor, hardwarePerformCalO3BatteryMonitor, HARDWARE_EMPTY, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SWAP_RIO_BATTERY, HardwareSwapRioBattery, hardwareSwapRioBattery, HardwareBatterySwapData batterySwapData, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SWAP_O1_BATTERY, HardwareSwapO1Battery, hardwareSwapO1Battery, HardwareBatterySwapData batterySwapData, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SWAP_O2_BATTERY, HardwareSwapO2Battery, hardwareSwapO2Battery, HardwareBatterySwapData batterySwapData, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_SWAP_O3_BATTERY, HardwareSwapO3Battery, hardwareSwapO3Battery, HardwareBatterySwapData batterySwapData, HARDWARE_EMPTY)
HARDWARE_MSG_DEF (HARDWARE_READ_RIO_BATT_TEMPERATURE, HardwareReadRioBattTemperature, hardwareReadRioBattTemperature, HARDWARE_EMPTY, double temperature)
HARDWARE_MSG_DEF (HARDWARE_READ_O1_BATT_TEMPERATURE, HardwareReadO1BattTemperature, hardwareReadO1BattTemperature, HARDWARE_EMPTY, double temperature)
HARDWARE_MSG_DEF (HARDWARE_READ_O2_BATT_TEMPERATURE, HardwareReadO2BattTemperature, hardwareReadO2BattTemperature, HARDWARE_EMPTY, double temperature)
HARDWARE_MSG_DEF (HARDWARE_READ_O3_BATT_TEMPERATURE, HardwareReadO3BattTemperature, hardwareReadO3BattTemperature, HARDWARE_EMPTY, double temperature)
HARDWARE_MSG_DEF (HARDWARE_SEND_O_STRING, HardwareSendOString, hardwareSendOString, OInputContainer string, OResponseString string)

