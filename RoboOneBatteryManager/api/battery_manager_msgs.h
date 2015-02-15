/*
 *  Messages that go from/to the battery manager server.
 */

/* This file is included in the battery_manager_msg_auto header and
 * defines the message structures for the messages sent to
 * and from the server.  The items in the list are:
 * 
 * - the message type enum,
 * - the message typedef struct (without an Ind/Req or Rsp/Cnf on the end),
 * - the message variable name for use in the message union
 *   (again, without a Rsp/Cnf or Ind/Req on the end),
 * - the message member that is needed in the Ind message structure
 *   beyond the mandatory msgHeader (which is added automatagically),
 * - the message member that is needed in the Rsp/Cnf message structure.
 */

/*
 * Definitions
 */
BATTERY_MANAGER_MSG_DEF (BATTERY_MANAGER_SERVER_START, BatteryManagerServerStart, batteryManagerServerStart, BATTERY_MANAGER_EMPTY, Bool success)
BATTERY_MANAGER_MSG_DEF (BATTERY_MANAGER_SERVER_STOP, BatteryManagerServerStop, batteryManagerServerStop, BATTERY_MANAGER_EMPTY, Bool success)
BATTERY_MANAGER_MSG_DEF (BATTERY_MANAGER_DATA_RIO, BatteryManagerDataRio, batteryManagerDataRio, BatteryData data, BATTERY_MANAGER_EMPTY)
BATTERY_MANAGER_MSG_DEF (BATTERY_MANAGER_DATA_O1, BatteryManagerDataO1, batteryManagerDataO1, BatteryData data, BATTERY_MANAGER_EMPTY)
BATTERY_MANAGER_MSG_DEF (BATTERY_MANAGER_DATA_O2, BatteryManagerDataO2, batteryManagerDataO2, BatteryData data, BATTERY_MANAGER_EMPTY)
BATTERY_MANAGER_MSG_DEF (BATTERY_MANAGER_DATA_O3, BatteryManagerDataO3, batteryManagerDataO3, BatteryData data, BATTERY_MANAGER_EMPTY)
BATTERY_MANAGER_MSG_DEF (BATTERY_MANAGER_CHARGING_PERMITTED, BatteryManagerChargingPermitted, batteryManagerChargingPermitted, Bool isPermitted, BATTERY_MANAGER_EMPTY)