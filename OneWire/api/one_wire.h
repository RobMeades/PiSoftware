/*
** OneWire.h
** General definitions for OneWire functions
*/ 

#define ONEWIRE_PORT "/dev/USBSerial"
#define NUM_BYTES_IN_SERIAL_NUM 8

#define DS2438_ADB_IS_BUSY              0x40
#define DS2438_NVB_IS_BUSY              0x20
#define DS2438_TB_IS_BUSY               0x10
#define DS2438_AD_IS_VDD                0x08
#define DS2438_EE_IS_ENABLED            0x04
#define DS2438_CA_IS_ENABLED            0x02
#define DS2438_IAD_IS_ENABLED           0x01
#define DS4238_NUM_BYTES_IN_PAGE        8
#define DS2438_NUM_USER_DATA_PAGES      4

bool readNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem);
bool writeNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem, UInt8 size);
Bool readVddDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage);
Bool readVadDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage);
Bool readTemperatureDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, double *pTemperature);
Bool readCurrentDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, SInt16 *pCurrent);
Bool readBatteryDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage, SInt16 *pCurrent);
Bool readNVConfigThresholdDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pConfig, UInt8 *pThreshold);
Bool writeNVConfigThresholdDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pConfig, UInt8 *pThreshold);
Bool readTimeCapacityCalDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pElapsedTime, UInt16 *pRemainingCapacity, SInt16 *pOffsetCal);
Bool writeTimeCapacityDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pElapsedTime, UInt16 *pRemainingCapacity);
Bool readTimePiOffChargingStoppedDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pPiOff, UInt32 *pChargingStopped);
Bool readNVChargeDischargeDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pCharge, UInt32 *pDischarge);
Bool writeNVChargeDischargeDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pCharge, UInt32 *pDischarge);
Bool readNVUserDataDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 block, UInt8 *pMem);
Bool writeNVUserDataDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 block, UInt8 *pMem, UInt8 size);
Bool performCalDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, SInt16 *pOffsetCal);