/*
 * OneWire.h
 * General definitions for OneWire functions
 */ 

/* The number of bytes in a OneWire device serial number (AKA address) */
#define NUM_BYTES_IN_SERIAL_NUM     8

/* Utility functions */
UInt8 oneWireFindAllDevices (SInt32 portNumber, UInt8 *pAddress, UInt8 maxNumAddresses);
SInt32 oneWireStartBus (Char *pSerialPortString);
void oneWireStopBus (SInt32 portNumber);
Bool oneWireAccessDevice (SInt32 portNumber, UInt8 *pAddress);

/* To protect against deadlocks when looping for HW responses */
#define GUARD_COUNTER           255

/*
 * Definitions specific to DS2408 PIO chip
 */

#define FAMILY_PIO 0x29

#define DS2408_SEARCH_IS_ACTIVITY_LATCHED  0x01 /* Conditional search reads activity latch not just the PIO pins */ 
#define DS2408_SEARCH_IS_AND               0x02 /* Conditional search is a bit-wise AND search, not a bit-wise OR search */
#define DS2408_RSTZ_IS_STROBE              0x04 /* RSTZ line is a strobe, not a reset */
#define DS2408_DEVICE_HAS_POWER_ON_RESET   0x08 /* A power on reset has occurred */
#define DS2408_VCC_IS_PRESENT              0x80 /* VCC is connected to this device */
#define DS2408_MAX_BYTES_IN_CHANNEL_ACCESS 32   /* As returned by channelAccessReadDS2408() */

Bool disableTestModeDS2408 (SInt32 portNumber, UInt8 *pSerialNumber);
Bool readControlRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool writeControlRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 data);
Bool readPIOLogicStateDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
UInt8 channelAccessReadDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData, UInt8 numBytesToRead);
Bool channelAccessWriteDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool readPIOOutputLatchStateRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool readPIOActivityLatchStateRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool resetActivityLatchesDS2408 (SInt32 portNumber, UInt8 *pSerialNumber);
Bool readCSChannelSelectionMaskRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool writeCSChannelSelectionMaskRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 data);
Bool readCSChannelPolaritySelectionRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData);
Bool writeCSChannelPolaritySelectionRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 data);

/*
 * Definitions specific to DS2438 battery monitoring chip
 */
#define FAMILY_SBATTERY 0x26

#define DS2438_ADB_IS_BUSY              0x40 /* A/D conversion is in progress */
#define DS2438_NVB_IS_BUSY              0x20 /* Write to non-volatile storage is in progress */
#define DS2438_TB_IS_BUSY               0x10 /* Temperature measurement is in progress */
#define DS2438_AD_IS_VDD                0x08 /* A/D does a measurement of VDD rather than the separate A/D input */
#define DS2438_EE_IS_ENABLED            0x04 /* Shadow charge and discharge accummulation to non-volatile storage */
#define DS2438_CA_IS_ENABLED            0x02 /* Charge and discharge accummulation enabled */
#define DS2438_IAD_IS_ENABLED           0x01 /* Current measurement and integrated current accummulation enabled */
#define DS2438_NUM_BYTES_IN_PAGE        8
#define DS2438_NUM_USER_DATA_PAGES      4
#define DS2438_NUM_PAGES                8

Bool readNVPageDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem);
Bool writeNVPageDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem, UInt8 size);
Bool readVddDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage);
Bool readVadDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage);
Bool readTemperatureDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, double *pTemperature);
Bool readCurrentDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, SInt16 *pCurrent);
Bool readBatteryDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage, SInt16 *pCurrent);
Bool readNVConfigThresholdDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pConfig, UInt8 *pThreshold);
Bool writeNVConfigThresholdDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pConfig, UInt8 *pThreshold);
Bool readTimeCapacityCalDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt32 *pElapsedTime, UInt16 *pRemainingCapacity, SInt16 *pOffsetCal);
Bool writeTimeCapacityDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt32 *pElapsedTime, UInt16 *pRemainingCapacity);
Bool readTimePiOffChargingStoppedDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt32 *pPiOff, UInt32 *pChargingStopped);
Bool readNVChargeDischargeDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt32 *pCharge, UInt32 *pDischarge);
Bool writeNVChargeDischargeDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt32 *pCharge, UInt32 *pDischarge);
Bool readNVUserDataDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 block, UInt8 *pMem);
Bool writeNVUserDataDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 block, UInt8 *pMem, UInt8 size);
Bool performCalDS2438 (SInt32 portNumber, UInt8 *pSerialNumber, SInt16 *pOffsetCal);