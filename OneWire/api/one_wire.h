/*
** OneWire.h
** General definitions for OneWire functions
*/ 

#define ONEWIRE_PORT "/dev/USBSerial"
#define NUM_BYTES_IN_SERIAL_NUM 8

/* To protect against deadlocks when looping for HW responses */
#define GUARD_COUNTER           255

/*
** Definitions specific to DS2408 PIO chip
*/

#define PIO_FAM 0x29

#define DS2408_SEARCH_IS_ACTIVITY_LATCHED 0x01
#define DS2408_SEARCH_IS_AND              0x02
#define DS2408_RSTZ_IS_STROBE             0x04
#define DS2408_DEVICE_HAS_POWER_ON_RESET  0x08
#define DS2408_VCC_IS_PRESENT             0x80
#define DS2408_MAX_BYTES_TO_READ          32 /* As returned by channelAccessReadDS2408() */

Bool disableTestModeDS2408 (UInt8 portNumber, UInt8 *pSerialNumber);
Bool readControlRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool writeControlRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 data);
Bool readPIOLogicStateDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
UInt8 channelAccessReadDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool channelAccessWriteDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool readPIOOutputLatchStateRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool readPIOActivityLatchStateRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool resetActivityLatchesDS2408 (UInt8 portNumber, UInt8 *pSerialNumber);
Bool readCSChannelSelectionMaskRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool writeCSChannelSelectionMaskRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 data);
Bool readCSChannelPolaritySelectionRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool writeCSChannelPolaritySelectionRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 data);

/*
** Definitions specific to DS2438 battery monitoring chip
*/

#define DS2438_ADB_IS_BUSY              0x40
#define DS2438_NVB_IS_BUSY              0x20
#define DS2438_TB_IS_BUSY               0x10
#define DS2438_AD_IS_VDD                0x08
#define DS2438_EE_IS_ENABLED            0x04
#define DS2438_CA_IS_ENABLED            0x02
#define DS2438_IAD_IS_ENABLED           0x01
#define DS4238_NUM_BYTES_IN_PAGE        8
#define DS2438_NUM_USER_DATA_PAGES      4

Bool readNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem);
Bool writeNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem, UInt8 size);
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