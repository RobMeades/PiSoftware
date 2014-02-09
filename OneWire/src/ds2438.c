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

#define TEMPERATURE_UNIT          0.03125
#define TEMPERATURE_READ_DELAY_MS 10

/*
 * STATIC FUNCTIONS
 */

/*
 * Read A/D of the DS2438 chip (answer in mV).
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * isVdd         set to true for a Vdd read, false for a Vad read.
 * pVoltage      a pointer to somewhere to put the answer.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
static Bool readAdDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, Bool isVdd, UInt16 * pVoltage)
{
    Bool success;
    Bool done = false;
    UInt8 buffer[20];
    UInt8 busyByte;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);

    /* TODO: have a timeout here */
    while (!done)
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
                /* TODO: have a timeout here */
                busyByte = 0;
                while (busyByte == 0)
                {
                    busyByte = owReadByte (portNumber);
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
    }

    return success;
}

/*
 * PUBLIC FUNCTIONS
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
Bool readSPPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem)
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
Bool readNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem)
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
Bool writeSPPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem, UInt8 size)
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
Bool writeNVPageDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 page, UInt8 * pMem, UInt8 size)
{
    Bool success;
    UInt8 buffer[20];
    UInt8 count = 0;
    UInt8 busyByte;

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
                /* TODO: have a timeout here */
                busyByte = 0;
                while (busyByte == 0)
                {
                    busyByte = owReadByte (portNumber);
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
 * pVoltage      a pointer to somewhere to put the answer.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readVddDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 * pVoltage)
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
 * pVoltage      a pointer to somewhere to put the answer.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readVadDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 * pVoltage)
{
    return readAdDS2438 (portNumber, pSerialNumber, false, pVoltage);
}

/*
 * Read the temperature of the DS2438 chip (in Celsius).
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pTemperature  a pointer to somewhere to put the answer.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readTemperatureDS2438 (UInt8 portNumber, UInt8 *pSerialNumber, double * pTemperature)
{
    Bool success;
    UInt8 buffer[20];
    UInt8 busyByte;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);

    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Do a temperature reading */
    success = owAccess (portNumber); 
    if (success)
    {
        success = owWriteByte (portNumber, DS2438_COMMAND_READ_TEMPERATURE);

        /* Block until the reading is complete */
        /* TODO: have a timeout here */
        busyByte = 0;
        while (busyByte == 0)
        {
            busyByte = owReadByte (portNumber);
        }
    }
    
    /* Read the answer */
    if (success)
    {
        msDelay (TEMPERATURE_READ_DELAY_MS);

        /* TODO: have a timeout here */
        do
        {
            success = readNVPageDS2438 (portNumber, pSerialNumber, DS2438_CONFIG_PAGE, &buffer[0]);
        }
        while (success && (buffer[DS2438_CONFIG_REG_OFFSET] & DS2438_TB_IS_BUSY));

        if (success && (pTemperature != PNULL))
        {
            *pTemperature = (((buffer[DS2438_TEMPERATURE_REG_OFFSET + 1] << 8) | buffer[DS2438_TEMPERATURE_REG_OFFSET]) >> 3) * TEMPERATURE_UNIT;
        }
    }

    return success;
}