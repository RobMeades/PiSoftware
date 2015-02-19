/*
 *  Public types for the Hardware server
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * TYPES
 */

typedef struct HardwareChargeDischargeTag
{
    UInt32 charge;
    UInt32 discharge;
} HardwareChargeDischarge;

#pragma pack(pop) /* End of packing */ 