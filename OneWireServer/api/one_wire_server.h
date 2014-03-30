/*
 *  Public stuff for the OneWire server
 */

/*
 * MANIFEST CONSTANTS
 */

/* Maximum length of the string naming the serial port to use */
#define MAX_SERIAL_PORT_NAME_LENGTH 80
/* The maximum number of devices that oneWireFindAllDevices() can report */
#define MAX_DEVICES_TO_FIND             10

/*
 * TYPES
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/*
 * GENERAL MESSAGE TYPES
 */

/* All OneWire REQuest messages have this at the start of their msgBody */
typedef struct MsgHeaderTag
{
    SInt32 portNumber;
    UInt8 serialNumber[NUM_BYTES_IN_SERIAL_NUM];  
} MsgHeader;

/*
 * TYPES FOR REQ MESSAGES
 */
typedef struct DeviceListTag
{
    UInt8 numDevices; /* IMPORTANT: numDevices can be bigger than MAX_DEVICES_TO_FIND */
    UInt8 address [NUM_BYTES_IN_SERIAL_NUM * MAX_DEVICES_TO_FIND];
} DeviceList;

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

typedef struct OneWireWriteNVUserDataDS2438Tag
{
    UInt8 block;
    UInt8 dataLength;
    UInt8 data[DS4238_NUM_BYTES_IN_PAGE];
} OneWireWriteNVUserDataDS2438;

/*
 * TYPES FOR CNF MESSAGES
 */

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

typedef double OneWireTemperatureDS2438;

typedef UInt16 OneWireVoltageDS2438;

typedef SInt16 OneWireCurrentDS2438;

typedef struct OneWireBatteryDS2438Tag
{
    UInt16 voltage;
    SInt16 current;
} OneWireBatteryDS2438;

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

#pragma pack(pop) /* End of packing */ 