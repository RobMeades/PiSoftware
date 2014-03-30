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

MSG_DEF (ONE_WIRE_SERVER_EXIT, OneWireServerExit, oneWireServerExit, ONE_WIRE_EMPTY, ONE_WIRE_EMPTY)
MSG_DEF (ONE_WIRE_START_BUS, OneWireStartBus, oneWireStartBus, Char serialPortString[MAX_SERIAL_PORT_NAME_LENGTH], SInt32 port)
MSG_DEF (ONE_WIRE_STOP_BUS, OneWireStopBus, oneWireStopBus, SInt32 portNumber, ONE_WIRE_EMPTY)
MSG_DEF (ONE_WIRE_FIND_ALL_DEVICES, OneWireFindAllDevices, oneWireFindAllDevices, ONE_WIRE_EMPTY, DeviceList deviceList)

MSG_DEF (DISABLE_TEST_MODE_DS2408, DisableTestModeDS2408, disableTestModeDS2408, ONE_WIRE_EMPTY, ONE_WIRE_EMPTY)
MSG_DEF (READ_CONTROL_REGISTER_DS2408, ReadControlRegisterDS2408, readControlRegisterDS2408, ONE_WIRE_EMPTY, UInt8 data)
MSG_DEF (WRITE_CONTROL_REGISTER_DS2408, WriteControlRegisterDS2408, writeControlRegisterDS2408, UInt8 data, ONE_WIRE_EMPTY)
