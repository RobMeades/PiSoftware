/*
 *  Public stuff for the OneWire server
 */

/*
 * MANIFEST CONSTANTS
 */

/* The port to use */
#define ONE_WIRE_SERVER_PORT      5234

/*
 * TYPES
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE STRUCTURES
 */

/* All OneWire REQuest messages have this at the start of their msgBody */
typedef struct OneWireReqMsgHeaderTag
{
    SInt32 portNumber;
    UInt8 serialNumber[NUM_BYTES_IN_SERIAL_NUM];  
} OneWireReqMsgHeader;

/* A confirmation (or not) of succes */
typedef struct OneWireResultTag
{
    Bool success;  
} OneWireResult;

/* An empty item, used to make the compiler happy when automagic would otherwise make an actually empty structure */
typedef struct OneWireEmptyItemTag
{
    UInt8 nothing;  
} OneWireEmptyItem;

/*
 * MESSAGES BODIES: REQ MESSAGES
 */

typedef struct OneWireWriteByteTag
{
    UInt8 data;
} OneWireWriteByte;

typedef struct OneWireWritePageDS2438Tag
{
    UInt8 dataLength;
    UInt8 data[DS4238_NUM_BYTES_IN_PAGE];
} OneWireWritePageDS2438;

typedef struct OneWireWriteConfigThresholdDS2438Tag
{
    UInt8 config;
    Bool thresholdPresent;
    UInt8 threshold;
} OneWireWriteConfigThresholdDS2438;

typedef struct OneWireWriteTimeCapacityDS2438Tag
{
    UInt32 elapsedTime;
    Bool remainingCapacityPresent;
    UInt16 remainingCapacity;
} OneWireWriteTimeCapacityDS2438;

typedef struct OneWireWriteChargeDischargeDS2438Tag
{
    UInt32 charge;
    Bool dischargePresent;
    UInt32 discharge;
} OneWireWriteChargeDischargeDS2438;

/*
 * MESSAGES BODIES: CNF MESSAGES
 */

typedef struct OneWireReadByteTag
{
    UInt8 data;
} OneWireReadByte;

typedef struct OneWireChannelAccessReadDS2408Tag
{
    UInt8 dataLength;
    UInt8 data[DS2408_MAX_BYTES_TO_READ];
} OneWireChannelAccessReadDS2408;

typedef struct OneWireReadPageDS2438Tag
{
    UInt8 dataLength;
    UInt8 data[DS4238_NUM_BYTES_IN_PAGE];
} OneWireReadPageDS2438;

typedef struct OneWireReadTemperatureDS2438Tag
{
    double temperature;
} OneWireReadTemperatureDS2438;

typedef struct OneWireReadVoltageDS2438Tag
{
    UInt16 voltage;
} OneWireReadVoltageDS2438;

typedef struct OneWireReadCurrentDS2438Tag
{
    SInt16 current;
} OneWireReadCurrentDS2438;

typedef struct OneWireReadBatteryDS2438Tag
{
    UInt16 voltage;
    SInt16 current;
} OneWireReadBatteryDS2438;

typedef OneWireWriteConfigThresholdDS2438 OneWireReadConfigThresholdDS2438;

typedef struct OneWireReadTimeCapacityCalDS2438Tag
{
    UInt32 elapsedTime;
    UInt16 remainingCapacity;
    SInt16 offsetCal;
} OneWireReadTimeCapacityCalDS2438;

typedef struct OneWireReadTimePiOffChargingStoppedDS2438Tag
{
    UInt32 piOff;
    UInt32 chargingStopped;
} OneWireReadTimePiOffChargingStoppedDS2438;

typedef struct OneWireReadChargeDischargeDS2438Tag
{
    UInt32 charge;
    UInt32 discharge;
} OneWireReadChargeDischargeDS2438;

typedef struct OneWireReadCalDS2438Tag
{
    SInt16 offsetCal;
} OneWireReadCalDS2438;

#pragma pack(pop) /* End of packing */ 