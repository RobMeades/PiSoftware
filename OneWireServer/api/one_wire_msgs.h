/*
 *  Messages that go from/to the One Wire server.
 */

/* This file is included in the one_wire_msg_auto header and
 * defines the message structures for the messages sent to
 * and from the server.  The items in the list are:
 * 
 * - the message enum,
 * - the message typedef struct (without a Cnf or Req on the end),
 * - the message variable name for use in the message union
 *   (again, without a Cnf or Req on the end),
 * - a typedef struct that is needed in the Req message structure
 *   beyond the mandatory msgHeader (which is added automatagically),
 * - the variable name for the above,
 * - a typedef struct that is needed in the Cnf message structure,
 * - the variable name for the above.
 */

MSG_DEF (DISABLE_TEST_MODE_DS2408, DisableTestModeDS2408, disableTestModeDS2408, OneWireEmptyItem, oneWireEmptyItem, OneWireEmptyItem, oneWireEmptyItem)
MSG_DEF (READ_CONTROL_REGISTER_DS2408, ReadControlRegisterDS2408, readControlRegisterDS2408, OneWireEmptyItem, oneWireEmptyItem, OneWireReadByte, oneWireReadByte)
MSG_DEF (WRITE_CONTROL_REGISTER_DS2408, WriteControlRegisterDS2408, writeControlRegisterDS2408, OneWireWriteByte, oneWireWriteByte, OneWireEmptyItem, oneWireEmptyItem)
