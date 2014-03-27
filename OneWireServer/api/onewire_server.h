/*
 *  Public stuff for the OneWire server
 */

/*
 * MANIFEST CONSTANTS
 */

#define ONE_WIRE_SERVER_PORT      5234

/*
 * TYPES
 */

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/* The list of possible OneWire messages,
 * matching the list of function prototypes
 * in OneWire\one_wire.h.  ORDER IS IMPORTANT,
 * must match that of the function table in
 * onewire_server.c */
typedef enum OneWireMsgTypeTag
{
    /* DS2408 functions */
    DISABLE_TEST_MODE_DS2408,
    READ_CONTROL_REGISTER_DS2408,
    WRITE_CONTROL_REGISTER_DS2408,
    READ_PIO_LOGIC_STATE_DS2408,
    CHANNEL_ACCESS_READ_DS2408,
    CHANNEL_ACCESS_WRITE_DS2408,
    READ_PIO_OUTPUT_LATCH_STATE_REGISTER_DS2408,
    READ_PIO_ACTIVITY_LATCH_STATE_REGISTER_DS2408,
    RESET_ACTIVITY_LATCHES_DS2408,
    READ_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408,
    WRITE_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408,
    READ_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408,
    WRITE_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408,
    /* DS2438 functions */
    READ_NV_PAGE_DS2438,
    WRITE_NV_PAGE_DS2438,
    READ_VDD_DS2438,
    READ_VAD_DS2438,
    READ_TEMPERATURE_DS2438,
    READ_CURRENT_DS2438,
    READ_BATTERY_DS2438,
    READ_NV_CONFIG_THRESHOLD_DS2438,
    WRITE_NV_CONFIG_THRESHOLD_DS2438,
    READ_TIME_CAPACITY_CAL_DS2438,
    WRITE_TIME_CAPACITY_DS2438,
    READ_TIME_PI_OFF_CHARGING_STOPPED_DS2438,
    READ_NV_CHARGE_DISCHARGE_DS2438,
    WRITE_NV_CHARGE_DISCHARGE_DS2438,
    READ_NV_USER_DATA_DS2438,
    WRITE_NV_USER_DATA_DS2438,
    PERFORM_CAL_DS2438,
    MAX_NUM_ONE_WIRE_MSG
} OneWireMsgType;

/* All OneWire REQuest messages have this at the start of their msgBody */
typedef struct OneWireReqMsgHeaderTag
{
    SInt32 portNumber;
    UInt8 serialNumber[NUM_BYTES_IN_SERIAL_NUM];  
} OneWireReqMsgHeader;

#pragma pack(pop) /* End of packing */ 