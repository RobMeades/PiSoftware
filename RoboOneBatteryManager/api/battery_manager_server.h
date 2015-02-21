/*
 *  Public stuff for the Battery Manager server
 */

/*
 * MANIFEST CONSTANTS
 */

#define BATTERY_MANAGER_SERVER_EXE "./roboone_battery_manager_server"
#define BATTERY_MANAGER_SERVER_PORT_STRING "5232"

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE TYPES
 */

/*
 * TYPES FOR IND MESSAGES
 */

typedef struct BatteryDataTag
{
    SInt16 current;
    UInt16 voltage;
    UInt16 remainingCapacity;
    HardwareChargeDischarge chargeDischarge;
}  BatteryData;

/*
 * TYPES FOR RSP MESSAGES
 */

/*
 * TYPES FOR REQ MESSAGES
 */

/*
 * TYPES FOR CNF MESSAGES
 */

#pragma pack(pop) /* End of packing */ 