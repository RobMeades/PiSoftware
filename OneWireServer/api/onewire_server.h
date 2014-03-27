/*
 *  Public stuff from the OneWire server
 */

/*
 * MANIFEST CONSTANTS
 */

#define ONE_WIRE_SERVER_PORT      5234

/* Things to size and find bits inside all messages */
#define OFFSET_TO_MSG_LENGTH      0
#define OFFSET_TO_MSG_TYPE        1
#define OFFSET_TO_MSG_BODY        2
#define MAX_MSG_LENGTH            64
#define MAX_MSG_BODY_LENGTH       MAX_MSG_LENGTH - OFFSET_TO_MSG_BODY

/* Things to size and find bits inside REQuest messages */
#define OFFSET_TO_REQ_MSG_HEADER  OFFSET_TO_MSG_BODY
#define MIN_REQ_MSG_LENGTH        OFFSET_TO_MSG_BODY + 4 + NUM_BYTES_IN_SERIAL_NUM

/*
 * TYPES
 */

/* These just to make things neater */
typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr SockAddr;

#pragma pack(push, 1) /* Force GCC to pack everything from here on as tightly as possible */

/* The list of possible OneWire messages,
 * matching the list of function prototypes
 * in OneWire\one_wire.h.  ORDER IS IMPORTANT,
 * must match that of the function table in
 * onewireserver.c */
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

/* The length of the body of a OneWire message
 * If you change this from a UInt8 you will need to change OFFSET_TO_MSG_TYPE above */
typedef UInt8 OneWireMsgLength;

/* All OneWire REQuest messages have this at the start of their msgBody */
typedef struct OneWireReqMsgHeaderTag
{
    SInt32 portNumber;
    UInt8 serialNumber[NUM_BYTES_IN_SERIAL_NUM];  
} OneWireReqMsgHeader;

/* The structure of a message to the OneWire server */
typedef struct OneWireMsgTag
{
    OneWireMsgLength msgLength;
    OneWireMsgType msgType;
    UInt8 msgBody[MAX_MSG_BODY_LENGTH];
} OneWireMsg;

#pragma pack(pop) /* End of packing */ 
