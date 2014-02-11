/*
 * ds2438.c
 * Functions for handling the DS2438 battery monitoring chip
 */ 

#include <stdio.h>
#include <string.h>
#include <ownet.h>
#include <findtype.h>
#include <rob_system.h>
#include <one_wire.h>

#define NUM_BYTES_IN_CRC                1
#define DS4238_NUM_PAGES                8
#define DS4238_COMMAND_RECALL_MEMORY    0xB8
#define DS4238_COMMAND_READ_SCRATCHPAD  0xBE
#define DS4238_COMMAND_WRITE_SCRATCHPAD 0x4E
#define DS4238_COMMAND_COPY_SCRATCHPAD  0x48
#define DS2438_COMMAND_READ_AD          0xB4
#define DS2438_COMMAND_READ_TEMPERATURE 0x44

#define DS2438_CONFIG_PAGE              0
#define DS2438_CONFIG_REG_OFFSET        0
#define DS2438_CONFIG_REG_SIZE          1
#define DS2438_TEMPERATURE_REG_OFFSET   1
#define DS2438_VOLTAGE_REG_OFFSET       3
#define DS2438_CURRENT_REG_OFFSET       5
#define DS2438_THRESHOLD_REG_OFFSET     7

#define DS2438_ETM_ICA_OFFSET_PAGE      1
#define DS2438_ETM_REG_OFFSET           0
#define DS2438_ICA_REG_OFFSET           4
#define DS2438_CAL_REG_OFFSET           5

#define DS2438_DISC_EOC_PAGE            2
#define DS2438_DISC_REG_OFFSET          0
#define DS2438_EOC_REG_OFFSET           4

#define DS2438_FIRST_USER_DATA_PAGE     3

#define DS2438_CCA_DCA_PAGE             7
#define DS2438_CCA_REG_OFFSET           4
#define DS2438_DCA_REG_OFFSET           6

#define TEMPERATURE_UNIT          0.03125
#define TEMPERATURE_READ_DELAY_MS 10
#define RSENS                     0.1

/* From the DS2438 data sheet remaining capacity (in mA hours) = ICA / (2048 * RSENS) */
#define ICA_TO_MAHOURS(x)        (((x) * 1000) / (2048 * RSENS))
#define MAHOURS_TO_ICA(x)        (((x) * 2048 * RSENS) / 1000)
/* The units for the accumulated registers are 15.625 mV hours as opposed
 * to 0.488s mV hours, so 32 times bigger */
#define XCA_TO_MAHOURS(x)        (((x) * 1000 * 32) / (2048 * RSENS))
#define MAHOURS_TO_XCA(x)        (((x) * 2048 * RSENS) / (1000 * 32))
/* From the DS2438 data sheet I (in Amps) = Current Register / (4096 * RSENS) */
#define CURRENT_TO_MA(x)         (((x) * 1000) / (4096 * RSENS))
#define MA_TO_CURRENT(x)         (((x) * 4096 * RSENS) / 1000)

/* To protect against deadlocks when looping for HW responses */
#define GUARD_COUNTER                   255

/*
 * STATIC FUNCTIONS
 */

/*
 * Read an 8-byte page from scratchpad memory on a DS2438
 * device.  The data is NOT flushed in from non-volatile
 * memory first and hence the scratchpad data cannot be
 * affected by this read.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * page          the page number to recall.
 * pMem          a pointer to an 8 byte block of memory in which
 *               to store the data. If PNULL then the reading is
 *               performed but no data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
static Bool readSPPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem)
{
    Bool success;
    UInt8 buffer[20];
    UInt8 count=0;
    UInt8 i;
    UInt16 lastCrc8;

    ASSERT_PARAM (page <= DS4238_NUM_PAGES, page);
    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to read the scratchpad */
    success = owAccess (portNumber);
    if (success)
    {
        /* Read scratchpad */
        buffer[count++] = DS4238_COMMAND_READ_SCRATCHPAD;
        buffer[count++] = page;

        for (i = 0; i < DS4238_NUM_BYTES_IN_PAGE + NUM_BYTES_IN_CRC; i++)
        {
            buffer[count++] = 0xFF;
        }

        if (owBlock (portNumber, FALSE, buffer, count))
        {
            /* Check CRC over the 8 bytes received and the 9th CRC byte */
            setcrc8 (portNumber, 0);
            for (i = 2; i < count; i++)
            {   
                lastCrc8 = docrc8 (portNumber, buffer[i]);
            }

           if (lastCrc8 != 0)
           {
               success = false;
           }
           else
           {
               /* Copy out the result */
               if (pMem != PNULL)
               {
                   memcpy (pMem, &(buffer[2]), DS4238_NUM_BYTES_IN_PAGE);
               }
           }
        }
        else
        {
            success = false;
        }
    }

    return success;
}

/*
 * Write to the scratchpad memory of an 8-byte page on a DS2438
 * device.  The write is NOT flushed through to non-volatile
 * memory.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * page          the page number to write to.
 * pMem          a pointer to block of memory containing the data
 *               to be written.
 * size          the number of bytes to be written.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
static Bool writeSPPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem, UInt8 size)
{
    Bool success;
    UInt8 buffer[20];
    UInt8 count = 0;
    UInt8 i;

    ASSERT_PARAM (page <= DS4238_NUM_PAGES, page);
    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pMem != PNULL, (unsigned long) pMem);
    ASSERT_PARAM (size <= DS4238_NUM_BYTES_IN_PAGE, size);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to write the page into the scratchpad */
    success = owAccess (portNumber);
    if (success)
    {
        /* Write to scratchpad */
        buffer[count++] = DS4238_COMMAND_WRITE_SCRATCHPAD;
        buffer[count++] = page;

        /* copy in the data */
        for (i = 0; i < size; i++)
        {
            buffer[count++] = *(pMem + i);
        }
    
        if (!owBlock (portNumber, FALSE, buffer, count))
        {
            success = false;
        }
    }

    return success;
}

/*
 * Read A/D of the DS2438 chip (answer in mV).
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * isVdd         set to true for a Vdd read, false for a Vad read.
 * pVoltage      a pointer to somewhere to put the answer (may be
 *               PNULL).
 *
 * @return  true if the operation succeeded, otherwise false.
 */
static Bool readAdDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, Bool isVdd, UInt16 * pVoltage)
{
    Bool success;
    Bool done = false;
    UInt8 buffer[20];
    UInt8 busyByte;
    UInt8 guardCounter = GUARD_COUNTER;
    UInt8 guardCounter1 = GUARD_COUNTER;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);

    while (success && !done && (guardCounter > 0))
    {
        /* Read the configuration register so that we can do a logical OR of the AD bit in it */
        success = readSPPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);
        if (success)
        {
            if (isVdd)
            {
                /* Set the Vdd bit in the scratchpad */
                buffer[DS2438_CONFIG_REG_OFFSET] = buffer[DS2438_CONFIG_REG_OFFSET] | DS2438_AD_IS_VDD;
            }
            else
            {
                /* Reset the Vdd bit in the scratchpad (so to read Vad) */
                buffer[DS2438_CONFIG_REG_OFFSET] = buffer[DS2438_CONFIG_REG_OFFSET] & ~DS2438_AD_IS_VDD;
            }
            
            /* Write back to scratchpad (don't write to NVRAM to save cycles) */ 
            success = writeSPPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[DS2438_CONFIG_REG_OFFSET], DS2438_CONFIG_REG_SIZE);
        }
        
        /* Now do an A/D read */
        if (success && owAccess (portNumber))
        {
            if (owWriteByte (portNumber, DS2438_COMMAND_READ_AD))
            {
                /* Block until the reading is complete */
                busyByte = 0;
                while ((busyByte == 0) && (guardCounter1 > 0))
                {
                    busyByte = owReadByte (portNumber);
                    guardCounter1--;
                }
                if (guardCounter1 == 0)
                {
                    success = false;
                }
            }
            else
            {
                success = false;
            }
        }
        
        /* Read the answer */
        if (success)
        {
            success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);

            /* Only done if it was the reading we wanted and the busy flag has been reset */
            if (success &&
                (buffer[DS2438_CONFIG_REG_OFFSET] & ~DS2438_ADB_IS_BUSY) &&
                ((isVdd && (buffer[DS2438_CONFIG_REG_OFFSET] | DS2438_AD_IS_VDD)) ||
                 (!isVdd && (buffer[DS2438_CONFIG_REG_OFFSET] & ~DS2438_AD_IS_VDD))))
            {
                done = true;
                if (pVoltage != PNULL)
                {
                    *pVoltage = (buffer[DS2438_VOLTAGE_REG_OFFSET] + (((UInt16) buffer[DS2438_VOLTAGE_REG_OFFSET + 1]) << 8)) * 10;
                }
            }    
        }
        
        guardCounter--;
    }

    if (guardCounter == 0)
    {
        success = false;
    }

    return success;
}

/*
 * Read the current as measured by the DS2438 chip without.
 * any conversion to mA.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pCurrent      a pointer to somewhere to put the answer.
 *               May be PNULL.
 * 
 * @return  true if the operation succeeded, otherwise false.
 */
static  Bool readRawCurrentDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, SInt16 *pCurrent)
{
    Bool success;
    UInt8 buffer[20];
    
    /* Read the page with the data in it */
    success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);
    
    if (success)
    {
        if (pCurrent != PNULL)
        {
            *pCurrent = buffer[DS2438_CURRENT_REG_OFFSET] | (buffer[DS2438_CURRENT_REG_OFFSET + 1] << 8);
        }
    }
            
    return success;
}

/*
 * Write the offset calibration data to the DS2438 device.
 * Note that this will zero the elapsed time and remaining
 * capacity data, but those are volatile in any case.
 *
 * portNumber     the port number of the port being used for the
 *                1-Wire Network.
 * pSerialNumber  the serial number for the part that the read is
 *                to be done on.
 * pOffsetCal     the offset calibration value.  This will be
 *                stored exactly as is, no shifting is done before
 *                it is stored.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
static Bool writeNVCalDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, SInt16 offsetCal)
{
    Bool success;
    UInt8 buffer[DS2438_CAL_REG_OFFSET + sizeof (offsetCal)];

    memset (&buffer[0], 0, sizeof (buffer));
    buffer[DS2438_CAL_REG_OFFSET] = offsetCal;
    buffer[DS2438_CAL_REG_OFFSET + 1] = offsetCal >> 8;

    success = writeNVPageDS2438 (portNumber, pSerialNumber, DS2438_ETM_ICA_OFFSET_PAGE, &buffer[0], sizeof (buffer));
    
    return success;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Read an 8-byte page from non-volatilve memory on a DS2438
 * device.  Note that this will flush the page into the
 * scratch area first and hence the scratch area for that page
 * will be overwritten.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * page          the page number to recall.
 * pMem          a pointer to an 8 byte block of memory in which
 *               to store the data. If PNULL then the reading is
 *               performed but no data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem)
{
    Bool success;
    UInt8 buffer[20];
    UInt8 count=0;

    ASSERT_PARAM (page <= DS4238_NUM_PAGES, page);
    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to recall the page into the scratchpad */
    success = owAccess (portNumber); 
    if (success)
    {
        /* Recall page */
        buffer[count++] = DS4238_COMMAND_RECALL_MEMORY;
        buffer[count++] = page;
    
        if (!owBlock (portNumber, FALSE, buffer, count))
        {
            success = false;
        }
    }
    
    /* Now read the scratchpad */
    if (success)
    {
        success = readSPPageDS2438 (portNumber, pSerialNumber, page, pMem);
    }

    return success;
}

/*
 * Write to an 8-byte page on a DS2438 device, flushing the
 * page to non-volatile memory as well.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * page          the page number to write to.
 * pMem          a pointer to block of memory containing the data
 *               to be written.
 * size          the number of bytes to be written.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 *pMem, UInt8 size)
{
    Bool success;
    UInt8 buffer[20];
    UInt8 count = 0;
    UInt8 busyByte;
    UInt8 guardCounter = GUARD_COUNTER;

    ASSERT_PARAM (page <= DS4238_NUM_PAGES, page);
    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pMem != PNULL, (unsigned long) pMem);
    ASSERT_PARAM (size <= DS4238_NUM_BYTES_IN_PAGE, size);

    /* Write the page into the scratchpad */
    success = writeSPPageDS2438 (portNumber, pSerialNumber, page, pMem, size);

    if (success)
    {
        /* Now access the device to copy the data into EEPROM */
        success = owAccess (portNumber);
        if (success)
        {
            /* Copy scratchpad */
            buffer[count++] = DS4238_COMMAND_COPY_SCRATCHPAD;
            buffer[count++] = page;
    
            if (owBlock (portNumber, FALSE, buffer, count))
            {
                /* Block until the write is complete */
                busyByte = 0;
                while ((busyByte == 0) && (guardCounter > 0))
                {
                    busyByte = owReadByte (portNumber);
                    guardCounter--;
                }
                if (guardCounter == 0)
                {
                    success = false;
                }
            }
            else
            {
                success = false;
            }
        }
    }
    
    return success;
}

/*
 * Read Vdd of the DS2438 chip.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pVoltage      a pointer to somewhere to put the answer (in mV).
 *               May be PNULL.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readVddDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage)
{
    return readAdDS2438 (portNumber, pSerialNumber, true, pVoltage);
}

/*
 * Read Vad of the DS2438 chip.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pVoltage      a pointer to somewhere to put the answer (in mV).
 *               May be PNULL.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readVadDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage)
{
    return readAdDS2438 (portNumber, pSerialNumber, false, pVoltage);
}

/*
 * Read the current as measured by the DS2438 chip.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pCurrent      a pointer to somewhere to put the answer (in mA).
 *               May be PNULL.
 * 
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readCurrentDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, SInt16 *pCurrent)
{
    Bool success;
    
    /* Read the raw current and then convert it */
    success = readRawCurrentDS2438 (portNumber, pSerialNumber, pCurrent); 
    
    if (success)
    {
        if (pCurrent != PNULL)
        {
            *pCurrent = CURRENT_TO_MA (*pCurrent);
        }
    }
            
    return success;
}

/*
 * Read the current and voltage of the battery as measured by the
 * DS2438 chip.  This is the most efficient way to get battery status.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pCurrent      a pointer to somewhere to put the current (in mA).
 *               May be PNULL.
 * pVoltage      a pointer to somewhere to put the voltage (in mV).
 *               May be PNULL.
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readBatteryDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 *pVoltage, SInt16 *pCurrent)
{
    Bool success;
    Bool done = false;
    UInt8 buffer[20];
    UInt8 busyByte;
    UInt8 guardCounter = GUARD_COUNTER;
    UInt8 guardCounter1 = GUARD_COUNTER;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);

    while (success && !done && guardCounter > 0)
    {
        /* Read the configuration register so that we can do a logical OR of the AD bit in it */
        success = readSPPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);
        if (success)
        {
            /* Set the Vdd bit in the scratchpad */
            buffer[DS2438_CONFIG_REG_OFFSET] = buffer[DS2438_CONFIG_REG_OFFSET] | DS2438_AD_IS_VDD;
            
            /* Write back to scratchpad (don't write to NVRAM to save cycles) */ 
            success = writeSPPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[DS2438_CONFIG_REG_OFFSET], DS2438_CONFIG_REG_SIZE);
        }
        
        /* Now do an A/D read */
        if (success && owAccess (portNumber))
        {
            if (owWriteByte (portNumber, DS2438_COMMAND_READ_AD))
            {
                /* Block until the reading is complete */
                busyByte = 0;
                while ((busyByte == 0) && guardCounter1 > 0)
                {
                    busyByte = owReadByte (portNumber);
                    guardCounter1--;
                }
                if (guardCounter1 == 0)
                {
                    success = false;
                }
            }
            else
            {
                success = false;
            }
        }
        
        /* Read the answer */
        if (success)
        {
            success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);

            /* Only done if it was the reading we wanted and the busy flag has been reset */
            if (success &&
                (buffer[DS2438_CONFIG_REG_OFFSET] & ~DS2438_ADB_IS_BUSY) &&
                (buffer[DS2438_CONFIG_REG_OFFSET] | DS2438_AD_IS_VDD))
            {
                done = true;
                if (pVoltage != PNULL)
                {
                    *pVoltage = (buffer[DS2438_VOLTAGE_REG_OFFSET] + (((UInt16) buffer[DS2438_VOLTAGE_REG_OFFSET + 1]) << 8)) * 10;
                }
                if (pCurrent != PNULL)
                {
                    *pCurrent = buffer[DS2438_CURRENT_REG_OFFSET] | (buffer[DS2438_CURRENT_REG_OFFSET + 1] << 8);
                    /* From the DS2438 data sheet I (in Amps) = Current Register / (4096 * RSENS) */
                    *pCurrent = CURRENT_TO_MA (*pCurrent);
                }
            }    
        }
        
        guardCounter--;
    }
    
    if (guardCounter == 0)
    {
        success = false;
    }

    return success;
}

/*
 * Read the temperature of the DS2438 chip (in Celsius).
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pTemperature  a pointer to somewhere to put the answer (may
 *               be PNULL).
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readTemperatureDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, double *pTemperature)
{
    Bool success;
    UInt8 buffer[20];
    UInt8 busyByte;
    UInt8 guardCounter = GUARD_COUNTER;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);

    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Do a temperature reading */
    success = owAccess (portNumber); 
    if (success)
    {
        success = owWriteByte (portNumber, DS2438_COMMAND_READ_TEMPERATURE);

        /* Block until the reading is complete */
        busyByte = 0;
        while ((busyByte == 0) && (guardCounter > 0))
        {
            busyByte = owReadByte (portNumber);
            guardCounter--;
        }
        
        if (guardCounter == 0)
        {
            success = false;
        }
    }
    
    /* Read the answer */
    if (success)
    {
        msDelay (TEMPERATURE_READ_DELAY_MS);

        guardCounter = GUARD_COUNTER;
        do
        {
            success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);
            guardCounter--;
        }
        while (success && (buffer[DS2438_CONFIG_REG_OFFSET] & DS2438_TB_IS_BUSY) && (guardCounter > 0));

        if (guardCounter == 0)
        {
            success = false;
        }

        if (success && (pTemperature != PNULL))
        {
            *pTemperature = (((buffer[DS2438_TEMPERATURE_REG_OFFSET + 1] << 8) | buffer[DS2438_TEMPERATURE_REG_OFFSET]) >> 3) * TEMPERATURE_UNIT;
        }
    }

    return success;
}

/*
 * Read the configuration and threshold data from the DS2438
 * device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pConfig       a pointer to a location to store the config
 *               data (may be PNULL).
 * pThreshold    a pointer to a location to store the threshold
 *               data (may be PNULL).
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readNVConfigThresholdDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pConfig, UInt8 *pThreshold)
{
    bool success;
    UInt8 buffer[20];
    
    success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);
    
    if (success)
    {
        if (pConfig != PNULL)
        {
            *pConfig = buffer[DS2438_CONFIG_REG_OFFSET];
        }
        if (pThreshold != PNULL)
        {
            *pThreshold = buffer[DS2438_THRESHOLD_REG_OFFSET];
        }
    }
    
    return success;
}

/*
 * Write the configuration and optionally the threshold data to
 * a DS2438 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pConfig       pointer to the config byte.
 * pThreshold    pointer to the threshold byte (may be PNULL).
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeNVConfigThresholdDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pConfig, UInt8 *pThreshold)
{
    Bool success;
    UInt8 buffer[DS2438_THRESHOLD_REG_OFFSET + sizeof (*pThreshold)]; /* Leave enough room in buffer for both */
    UInt8 size = DS2438_CONFIG_REG_OFFSET + sizeof (*pConfig);         /* but only size for the first for now */

    ASSERT_PARAM (pConfig != PNULL, (unsigned long) pConfig);
    
    buffer[DS2438_CONFIG_REG_OFFSET] = *pConfig;
    
    if (pThreshold != PNULL)
    {
        buffer[DS2438_THRESHOLD_REG_OFFSET] = *pThreshold;
        size = sizeof (buffer);
    }
    
    success = writeNVPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0], size);
    
    return success;
}

/*
 * Read the elapsed time, remaining capacity and the offset
 * calibration data from the DS2438 device.
 *
 * portNumber          the port number of the port being used for the
 *                     1-Wire Network.
 * pSerialNumber       the serial number for the part that the read is
 *                     to be done on.
 * pElapsedTime        a pointer to a location to store the ETM
 *                     data (in seconds).  May be PNULL.
 * pRemainingCapacity  a pointer to a location to store the Integrated
 *                     Current Accumulator data (in mA hours). May be
 *                     PNULL.
 * pOffsetCal          a pointer to a location to store the offset data
 *                     (may be PNULL).  Note that the number returned is
 *                     exactly that stored in the register (which is
 *                     stored shifted left three bits).
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readTimeCapacityCalDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pElapsedTime, UInt16 *pRemainingCapacity, SInt16 *pOffsetCal)
{
    bool success;
    UInt8 buffer[DS4238_NUM_BYTES_IN_PAGE];
    
    success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_ETM_ICA_OFFSET_PAGE, &buffer[0]);
    
    if (success)
    {
        if (pElapsedTime != PNULL)
        {
            *pElapsedTime = buffer[DS2438_ETM_REG_OFFSET] | (buffer[DS2438_ETM_REG_OFFSET + 1] << 8) | (buffer[DS2438_ETM_REG_OFFSET + 2] << 16) | (buffer[DS2438_ETM_REG_OFFSET + 3] << 24);
        }
        if (pRemainingCapacity != PNULL)
        {
            *pRemainingCapacity = ICA_TO_MAHOURS (buffer[DS2438_ICA_REG_OFFSET]);
        }
        if (pOffsetCal != PNULL)
        {
            *pOffsetCal = buffer[DS2438_CAL_REG_OFFSET] | (buffer[DS2438_CAL_REG_OFFSET + 1] << 8);
        }
    }
    
    return success;
}

/*
 * Write the elapsed time and (optionally) remaining capacity data
 * to the DS2438 device.  Note that these numbers are not stored in
 * non-volatile memory.  
 *
 * portNumber          the port number of the port being used for the
 *                     1-Wire Network.
 * pSerialNumber       the serial number for the part that the read is
 *                     to be done on.
 * pElapsedTime        a pointer to a location to store the ETM
 *                     data (in seconds).
 * pRemainingCapacity  a pointer to a location to store the Integrated
 *                     Current Accumulator data (in mA hours).  May be
 *                     PNULL.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeTimeCapacityDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pElapsedTime, UInt16 *pRemainingCapacity)
{
    Bool success;
    UInt8 ica;
    UInt8 buffer[DS2438_ICA_REG_OFFSET + sizeof (ica)];          /* Leave enough room in buffer for both */
    UInt8 size = DS2438_ETM_REG_OFFSET + sizeof (*pElapsedTime); /* but only size for the first for now */

    ASSERT_PARAM (pElapsedTime != PNULL, (unsigned long) pElapsedTime);
    
    buffer[DS2438_ETM_REG_OFFSET] = *pElapsedTime;
    buffer[DS2438_ETM_REG_OFFSET + 1] = *pElapsedTime >> 8;
    buffer[DS2438_ETM_REG_OFFSET + 2] = *pElapsedTime >> 16;
    buffer[DS2438_ETM_REG_OFFSET + 3] = *pElapsedTime >> 24;
    
    if (pRemainingCapacity != PNULL)
    {
        /* Convert back to ICA units */
        ica = MAHOURS_TO_ICA (*pRemainingCapacity);
        buffer[DS2438_ICA_REG_OFFSET] = ica;
        size = sizeof (buffer);
    }
    
    success = writeNVPageDS2438 (portNumber, pSerialNumber, DS2438_ETM_ICA_OFFSET_PAGE, &buffer[0], size);
    
    return success;
}

/*
 * Read the time that the Pi went off and that charging stopped
 * from the DS2438 device.  All times are relative to the
 * elapsed time and are volatile.
 *
 * portNumber          the port number of the port being used for the
 *                     1-Wire Network.
 * pSerialNumber       the serial number for the part that the read is
 *                     to be done on.
 * pPiOff              a pointer to a location to store the timestamp
 *                     when the Pi went off (may be PNULL).
 * pChargingStopped    a pointer to a location to store the timestamp
 *                     when the charging last stopped (may be PNULL).
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readTimePiOffChargingStoppedDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pPiOff, UInt32 *pChargingStopped)
{
    bool success;
    UInt8 buffer[DS4238_NUM_BYTES_IN_PAGE];
    
    success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_DISC_EOC_PAGE, &buffer[0]);
    
    if (success)
    {
        if (pPiOff != PNULL)
        {
            *pPiOff = buffer[DS2438_DISC_REG_OFFSET] | (buffer[DS2438_DISC_REG_OFFSET + 1] << 8) | (buffer[DS2438_DISC_REG_OFFSET + 2] << 16) | (buffer[DS2438_DISC_REG_OFFSET + 3] << 24);
        }
        if (pChargingStopped != PNULL)
        {
            *pChargingStopped = buffer[DS2438_EOC_REG_OFFSET] | (buffer[DS2438_EOC_REG_OFFSET + 1] << 8) | (buffer[DS2438_EOC_REG_OFFSET + 2] << 16) | (buffer[DS2438_EOC_REG_OFFSET + 3] << 24);
        }
    }
    
    return success;
}

/*
 * Read the charge and discharge accumulators from non-volatile
 * storage on the DS2438 device.
 *
 * portNumber     the port number of the port being used for the
 *                1-Wire Network.
 * pSerialNumber  the serial number for the part that the read is
 *                to be done on.
 * pCharge        a pointer to a location to store the charge
 *                accumulated (in mA hours). May be PNULL.
 * pDischarge     a pointer to a location to store the discharge
 *                accumulated (in mA hours). May be PNULL.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readNVChargeDischargeDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pCharge, UInt32 *pDischarge)
{
    bool success;
    UInt8 buffer[DS4238_NUM_BYTES_IN_PAGE];
    
    success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_CCA_DCA_PAGE, &buffer[0]);
    
    if (success)
    {
        if (pCharge != PNULL)
        {
            *pCharge = buffer[DS2438_CCA_REG_OFFSET] | (buffer[DS2438_CCA_REG_OFFSET + 1] << 8);
            *pCharge = XCA_TO_MAHOURS (*pCharge);
        }
        if (pDischarge != PNULL)
        {
            *pDischarge = buffer[DS2438_DCA_REG_OFFSET] | (buffer[DS2438_DCA_REG_OFFSET + 1] << 8);
            *pDischarge = XCA_TO_MAHOURS (*pDischarge);
        }
    }
    
    return success;
}

/*
 * Write the charge and (optionally) the discharge accumulators
 * to non-volatile storage on the DS2438 device.
 *
 * portNumber     the port number of the port being used for the
 *                1-Wire Network.
 * pSerialNumber  the serial number for the part that the read is
 *                to be done on.
 * pCharge        a pointer to a location to store the charge
 *                accumulated (in mA hours).
 * pDischarge     a pointer to a location to store the discharge
 *                accumulated (in mA hours).  May be PNULL.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeNVChargeDischargeDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt32 *pCharge, UInt32 *pDischarge)
{
    Bool success;
    UInt16 charge;
    UInt16 discharge;
    UInt8 buffer[DS2438_DCA_REG_OFFSET + sizeof (discharge)]; /* Leave enough room in buffer for both */
    UInt8 size = DS2438_CCA_REG_OFFSET + sizeof (charge);     /* but only size for the first for now */

    ASSERT_PARAM (pCharge != PNULL, (unsigned long) pCharge);
    
    memset (&buffer[0], 0, sizeof(buffer)); /* Zero the buffer because we will then neatly write zeros to the */
                                            /* unused 4 bytes at the start of the page that CCA/DCA are in */ 

    /* Convert charge back to the correct units */
    charge = MAHOURS_TO_XCA (*pCharge);
    buffer[DS2438_CCA_REG_OFFSET] = charge;
    buffer[DS2438_CCA_REG_OFFSET + 1] = charge >> 8;
    
    if (pDischarge != PNULL)
    {
        /* Convert discharge back to the correct units */
        discharge = MAHOURS_TO_XCA (*pDischarge);
        buffer[DS2438_DCA_REG_OFFSET] = discharge;
        buffer[DS2438_DCA_REG_OFFSET + 1] = discharge >> 8;
        size = sizeof (buffer);
    }
    
    success = writeNVPageDS2438 (portNumber, pSerialNumber, DS2438_CCA_DCA_PAGE, &buffer[0], size);
    
    return success;
}

/*
 * Read user data from a given page on the device. The user data
 * is in three eight byte blocks, number 0, 1 and 2.
 *
 * portNumber     the port number of the port being used for the
 *                1-Wire Network.
 * pSerialNumber  the serial number for the part that the read is
 *                to be done on.
 * block          the number of the block to read from (0 to 2).
 * pMem           a pointer to a place to put the data.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readNVUserDataDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 block, UInt8 *pMem)
{
    ASSERT_PARAM (pMem != PNULL, (unsigned long) pMem);
    ASSERT_PARAM (block < DS2438_NUM_USER_DATA_PAGES, block);

    return readNVPageDS2438 (portNumber, pSerialNumber, DS2438_FIRST_USER_DATA_PAGE + block, pMem);
}

/*
 * Write user data to a given page on the device. The user data
 * is in four eight byte blocks and each block must be written
 * from byte zero forwards.  So, for instance, to write 0xAA
 * to the third byte in block one you must provide bytes 0, 1
 * and 2 for that block. That's just the way the chip works
 * I'm afraid.
 *
 * portNumber     the port number of the port being used for the
 *                1-Wire Network.
 * pSerialNumber  the serial number for the part that the read is
 *                to be done on.
 * block          the number of the block to write to (0 to 3).
 * pMem           a pointer to up to 8 bytes of data to write.
 * size           the number of bytes from pMem to write.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeNVUserDataDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 block, UInt8 *pMem, UInt8 size)
{
    ASSERT_PARAM (pMem != PNULL, (unsigned long) pMem);
    ASSERT_PARAM (block < DS2438_NUM_USER_DATA_PAGES, block);
    ASSERT_PARAM (size <= DS4238_NUM_BYTES_IN_PAGE, size);

    return writeNVPageDS2438 (portNumber, pSerialNumber, DS2438_FIRST_USER_DATA_PAGE + block, pMem, size);
}

/*
 * Perform offset calibration for the current A/D.  Note that this
 * should only be run when there is NO CURRENT running through RSENS.
 * 
 * Calibration is done as follows (from the data sheet):
 * 
 * 1. Write all zeroes to the Offset Register.
 * 2. Read the Current Register value.
 * 3. Disable the current ADC by setting the IAD bit in the.
 *    Status/Configuration Register to 0.
 * 4. Change the sign of the previously-read Current Register value
 *    by performing the two’s complement
 * 5. Write the result to the Offset Register.
 * 6. Re-enable the current ADC by setting the IAD bit in the
 *    Status/Configuration Register to 1.
 *
 * Note that as a side-effect this will also zero the elapsed time
 * and remaining capacity data.
 * 
 * portNumber      the port number of the port being used for the
 *                 1-Wire Network.
 * pSerialNumber   the serial number for the part that the read is
 *                 to be done on.
 * pOffsetCal      a pointer to return the newly derived Offset
 *                 Calibation (may be PNULL).  Note that this
 *                 is in mA, so it is NOT the number written to
 *                 the register, which is in units of 0.2441 mV.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool performCalDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, SInt16 *pOffsetCal)
{
    bool success;
    SInt16 current = 0;
    UInt8 storedConfig;
    UInt8 tempConfig;
    bool iadWasEnabled = false;
    
    /* Write in zero to the offset calibration */
    success = writeNVCalDS2438 (portNumber, pSerialNumber, current);
    if (success)
    {
        /* Read the current when there should be none flowing */
        success = readRawCurrentDS2438 (portNumber, pSerialNumber, &current);
        if (success)
        {
            /* Switch off current measurement */
            success = readNVConfigThresholdDS2438 (portNumber, pSerialNumber, &storedConfig, PNULL);
            if (success && ((storedConfig | DS2438_IAD_IS_ENABLED) != 0))
            {
                iadWasEnabled = true;
                tempConfig = storedConfig & ~DS2438_IAD_IS_ENABLED;
                success = writeNVConfigThresholdDS2438 (portNumber, pSerialNumber, &tempConfig, PNULL);
                if (success)
                {
                    /* Two's complement the current measurement */
                    current *= -1;

                    /* Now, here's where the data sheet is less clear.  The current register is a straight
                     * 10 bit signed number.  The offset register is in the same units (0.2441 mVs) but the
                     * value coded into the register is a 9 bit number and is shifted left by three bits
                     * For the result to make sense, I think the current value needs to be manipulated in
                     * the same way (at least, this works by experiment with real calibrations) */
                    
                    /* So, first lose the 10th bit in a sign-sensitive way, then shift the number */
                    if (current < 0)
                    {
                        current |= 0x02;                        
                    }
                    else
                    {
                        current &= ~0x02;                                                
                    }
                    current <<= 3;
                    /* Now write the number back to the register */
                    success = writeNVCalDS2438 (portNumber, pSerialNumber, current);
                    if (pOffsetCal != PNULL)
                    {
                        *pOffsetCal = CURRENT_TO_MA (current);
                    }
                    if (success && iadWasEnabled)
                    {
                        /* Put IAD back as it was */
                        success = writeNVConfigThresholdDS2438 (portNumber, pSerialNumber, &storedConfig, PNULL);
                    }
                }
            }
        }
    }
    
    return success;
}