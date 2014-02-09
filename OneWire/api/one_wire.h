/*
** OneWire.h
** General definitions for OneWire functions
*/ 

#define ONEWIRE_PORT "/dev/USBSerial"
#define NUM_BYTES_IN_CRC  1
#define NUM_BYTES_IN_SERIAL_NUM 8
#define RSENS_TIMES_4096 410 

#define DS4238_NUM_BYTES_IN_PAGE 8
#define DS4238_NUM_PAGES 8
#define DS4238_COMMAND_RECALL_MEMORY    0xB8
#define DS4238_COMMAND_READ_SCRATCHPAD  0xBE
#define DS4238_COMMAND_WRITE_SCRATCHPAD 0x4E
#define DS4238_COMMAND_COPY_SCRATCHPAD  0x48
#define DS2438_COMMAND_READ_AD          0xB4
#define DS2438_COMMAND_READ_TEMPERATURE 0x44
#define DS2438_CONFIG_PAGE              0
#define DS2438_CONFIG_REG_OFFSET        0
#define DS2438_CONFIG_REG_SIZE          1
#define DS2438_ADB_IS_BUSY              0x40
#define DS2438_NVB_IS_BUSY              0x20
#define DS2438_TB_IS_BUSY               0x10
#define DS2438_AD_IS_VDD                0x08
#define DS2438_EE_IS_ENABLED            0x04
#define DS2438_CA_IS_ENABLED            0x02
#define DS2438_IAD_IS_ENABLED           0x01
#define DS2438_TEMPERATURE_REG_OFFSET   1
#define DS2438_VOLTAGE_REG_OFFSET       3
#define DS2438_CURRENT_REG_OFFSET       5

bool readSPPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem);
bool readNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem);
bool writeSPPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem, UInt8 size);
bool writeNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem, UInt8 size);
Bool readVddDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 * pVoltage);
Bool readVadDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 * pVoltage);
Bool readTemperatureDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, double * pTemperature);