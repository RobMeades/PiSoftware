/*
 *  Messages that go from/to the One Wire server.
 */

/* This file is included in the one_wire_msg_auto header and
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
 * Definitions to do with the One Wire bus itself
 */
ONE_WIRE_MSG_DEF (ONE_WIRE_SERVER_EXIT, OneWireServerExit, oneWireServerExit, ONE_WIRE_EMPTY, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (ONE_WIRE_START_BUS, OneWireStartBus, oneWireStartBus, Char serialPortString[MAX_SERIAL_PORT_NAME_LENGTH], SInt32 serialPortNumber)
ONE_WIRE_MSG_DEF (ONE_WIRE_STOP_BUS, OneWireStopBus, oneWireStopBus, SInt32 portNumber, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (ONE_WIRE_GET_PORT, OneWireGetPort, oneWireGetPort, ONE_WIRE_EMPTY, SInt32 serialPortNumber)
ONE_WIRE_MSG_DEF (ONE_WIRE_FIND_ALL_DEVICES, OneWireFindAllDevices, oneWireFindAllDevices, ONE_WIRE_EMPTY, DeviceList deviceList)
ONE_WIRE_MSG_DEF (ONE_WIRE_ACCESS_DEVICE, OneWireAccessDevice, oneWireAccessDevice, ONE_WIRE_EMPTY, ONE_WIRE_EMPTY)

/*
 * Definitions specific to DS2408 PIO chip
 */
ONE_WIRE_MSG_DEF (DISABLE_TEST_MODE_DS2408, DisableTestModeDS2408, disableTestModeDS2408, ONE_WIRE_EMPTY, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_CONTROL_REGISTER_DS2408, ReadControlRegisterDS2408, readControlRegisterDS2408, ONE_WIRE_EMPTY, UInt8 data)
ONE_WIRE_MSG_DEF (WRITE_CONTROL_REGISTER_DS2408, WriteControlRegisterDS2408, writeControlRegisterDS2408, UInt8 data, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_PIO_LOGIC_STATE_DS2408, ReadPIOLogicStateDS2408, readPIOLogicStateDS2408, ONE_WIRE_EMPTY, UInt8 data)
ONE_WIRE_MSG_DEF (CHANNEL_ACCESS_READ_DS2408, ChannelAccessReadDS2408, channelAccessReadDS2408, UInt8 numBytesToRead, OneWireChannelAccessReadDS2408 channelAccessReadDS2408)
ONE_WIRE_MSG_DEF (CHANNEL_ACCESS_WRITE_DS2408, ChannelAccessWriteDS2408, channelAccessWriteDS2408, UInt8 data[DS2408_MAX_BYTES_IN_CHANNEL_ACCESS], ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_PIO_OUTPUT_LATCH_STATE_REGISTER_DS2408, ReadPIOOutputLatchStateRegisterDS2408, readPIOOutputLatchStateRegisterDS2408, ONE_WIRE_EMPTY, UInt8 data)
ONE_WIRE_MSG_DEF (READ_PIO_ACTIVITY_LATCH_STATE_REGISTER_DS2408, ReadPIOActivityLatchStateRegisterDS2408, readPIOActivityLatchStateRegisterDS2408, ONE_WIRE_EMPTY, UInt8 data)
ONE_WIRE_MSG_DEF (RESET_ACTIVITY_LATCHES_DS2408, ResetActivityLatchesDS2408, resetActivityLatchesDS2408, ONE_WIRE_EMPTY, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408, ReadCSChannelSelectionMaskRegisterDS2408, readCSChannelSelectionMaskRegisterDS2408, ONE_WIRE_EMPTY, UInt8 data)
ONE_WIRE_MSG_DEF (WRITE_CS_CHANNEL_SELECTION_MASK_REGISTER_DS2408, WriteCSChannelSelectionMaskRegisterDS2408, writeCSChannelSelectionMaskRegisterDS2408, UInt8 data, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408, ReadCSChannelPolaritySelectionRegisterDS2408, readCSChannelPolaritySelectionRegisterDS2408, ONE_WIRE_EMPTY, UInt8 data)
ONE_WIRE_MSG_DEF (WRITE_CS_CHANNEL_POLARITY_SELECTION_REGISTER_DS2408, WriteCSChannelPolaritySelectionRegisterDS2408, writeCSChannelPolaritySelectionRegisterDS2408, UInt8 data, ONE_WIRE_EMPTY)

/*
 * Definitions specific to DS2438 battery monitoring chip
 */
ONE_WIRE_MSG_DEF (READ_NV_PAGE_DS2438, ReadNVPageDS2438, readNVPageDS2438, UInt8 page, UInt8 mem[DS2438_NUM_BYTES_IN_PAGE])
ONE_WIRE_MSG_DEF (WRITE_NV_PAGE_DS2438, WriteNVPageDS2438, writeNVPageDS2438, OneWireWriteNVPageDS2438 writeNVPageDS2438, ONE_WIRE_EMPTY)  /* Don't use this unless you know what you're doing (read the DS2438 data sheet first) */
ONE_WIRE_MSG_DEF (READ_VDD_DS2438, ReadVddDS2438, readVddDS2438, ONE_WIRE_EMPTY, UInt16 voltage)
ONE_WIRE_MSG_DEF (READ_VAD_DS2438, ReadVadDS2438, readVadDS2438, ONE_WIRE_EMPTY, UInt16 voltage)
ONE_WIRE_MSG_DEF (READ_TEMPERATURE_DS2438, ReadTemperatureDS2438, readTemperatureDS2438, ONE_WIRE_EMPTY, double temperature)
ONE_WIRE_MSG_DEF (READ_CURRENT_DS2438, ReadCurrentDS2438, readCurrentDS2438, ONE_WIRE_EMPTY, SInt16 current)
ONE_WIRE_MSG_DEF (READ_BATTERY_DS2438, ReadBatteryDS2438, readBatteryDS2438, ONE_WIRE_EMPTY, OneWireBatteryDS2438 batteryDS2438)
ONE_WIRE_MSG_DEF (READ_NV_CONFIG_THRESHOLD_DS2438, ReadNVConfigThresholdDS2438, readNVConfigThresholdDS2438, ONE_WIRE_EMPTY, OneWireReadNVConfigThresholdDS2438 readNVConfigThresholdDS2438)
ONE_WIRE_MSG_DEF (WRITE_NV_CONFIG_THRESHOLD_DS2438, WriteNVConfigThresholdDS2438, writeNVConfigThresholdDS2438, OneWireWriteNVConfigThresholdDS2438 writeNVConfigThresholdDS2438, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_TIME_CAPACITY_CAL_DS2438, ReadTimeCapacityCalDS2438, readTimeCapacityCalDS2438, ONE_WIRE_EMPTY, OneWireReadTimeCapacityCalDS2438 readTimeCapacityCalDS2438)
ONE_WIRE_MSG_DEF (WRITE_TIME_CAPACITY_DS2438, WriteTimeCapacityDS2438, writeTimeCapacityDS2438, OneWireWriteTimeCapacityDS2438 writeTimeCapacityDS2438, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_TIME_PI_OFF_CHARGING_STOPPED_DS2438, ReadTimePiOffChargingStoppedDS2438, readTimePiOffChargingStoppedDS2438, ONE_WIRE_EMPTY, OneWireReadTimePiOffChargingStoppedDS2438 readTimePiOffChargingStoppedDS2438)
ONE_WIRE_MSG_DEF (READ_NV_CHARGE_DISCHARGE_DS2438, ReadNVChargeDischargeDS2438, readNVChargeDischargeDS2438, ONE_WIRE_EMPTY, OneWireReadNVChargeDischargeDS2438 readNVChargeDischargeDS2438)
ONE_WIRE_MSG_DEF (WRITE_NV_CHARGE_DISCHARGE_DS2438, WriteNVChargeDischargeDS2438, writeNVChargeDischargeDS2438, OneWireWriteNVChargeDischargeDS2438 writeNVChargeDischargeDS2438, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (READ_NV_USER_DATA_DS2438, ReadNVUserDataDS2438, readNVUserDataDS2438, UInt8 block, UInt8 mem[DS2438_NUM_BYTES_IN_PAGE])
ONE_WIRE_MSG_DEF (WRITE_NV_USER_DATA_DS2438, WriteNVUserDataDS2438, writeNVUserDataDS2438, OneWireWriteNVUserDataDS2438 writeNVUserDataDS2438, ONE_WIRE_EMPTY)
ONE_WIRE_MSG_DEF (PERFORM_CAL_DS2438, PerformCalDS2438, performCalDS2438, ONE_WIRE_EMPTY, SInt16 offsetCal)
