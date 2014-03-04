/*
 * ds2408.c
 * Functions for handling the DS2408 PIO chip
 */ 

#include <stdio.h>
#include <string.h>
#include <ownet.h>
#include <findtype.h>
#include <rob_system.h>
#include <one_wire.h>

#define DS2408_NUM_BYTES_IN_CRC                 2
#define DS2408_NUM_BYTES_IN_COMMAND             1
#define DS2408_NUM_BYTES_IN_PAGE_ADDRESS        2
#define DS2408_NUM_BYTES_IN_OVERHEAD            (DS2408_NUM_BYTES_IN_CRC + DS2408_NUM_BYTES_IN_COMMAND + DS2408_NUM_BYTES_IN_PAGE_ADDRESS)
#define DS2408_MINIMUM_BUFFER_SIZE              5 /* Enough for a channel access write */
#define DS2408_COMMAND_DISABLE_TEST_MODE        0x3C
#define DS2408_COMMAND_READ_PIO_REGISTERS       0xF0
#define DS2408_COMMAND_CHANNEL_ACCESS_READ      0xF5
#define DS2408_COMMAND_CHANNEL_ACCESS_WRITE     0x5A
#define DS2408_COMMAND_WRITE_CS_REGISTER        0xCC
#define DS2408_COMMAND_RESET_ACTIVITY_LATCHES   0xC3

#define DS2408_CONFIRM_VALUE  0xAA
#define DS2408_UNUSED_VALUE   0x55

#define DS2408_FIRST_PAGE                             0x0088
#define DS2408_PIO_LOGIC_STATE_PAGE                   (DS2408_FIRST_PAGE)
#define DS2408_PIO_OUTPUT_LATCH_STATE_REGISTER_PAGE   (DS2408_FIRST_PAGE + 1)
#define DS2408_PIO_ACTIVITY_LATCH_STATE_REGISTER_PAGE (DS2408_FIRST_PAGE + 2)
#define DS2408_CS_CHANNEL_SELECTION_MASK_PAGE         (DS2408_FIRST_PAGE + 3)
#define DS2408_FIRST_WRITEABLE_PAGE                   DS2408_CS_CHANNEL_SELECTION_MASK_PAGE
#define DS2408_CS_CHANNEL_POLARITY_SELECTION_PAGE     (DS2408_FIRST_PAGE + 4)
#define DS2408_CONTROL_STATUS_REGISTER_PAGE           (DS2408_FIRST_PAGE + 5)
#define DS2408_LAST_USEFUL_PAGE                       DS2408_CONTROL_STATUS_REGISTER_PAGE
#define DS2408_NUM_USEFUL_PAGES                       (DS2408_LAST_USEFUL_PAGE - DS2408_FIRST_PAGE + 1)
#define DS2408_LAST_PAGE                              0x008F
#define DS2408_NUM_PAGES                              (DS2408_LAST_PAGE - DS2408_FIRST_PAGE + 1)


/*
 * STATIC FUNCTIONS
 */

/*
 * Read memory on a DS2408 device.  Note that in order to read
 * the CRC the read process will always continue to the end of
 * the device memory, though only the number of bytes requested
 * in 'size' will be returned.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * page          the page address to recall.
 * pMem          a pointer to a location to store the data.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 * size          the number of bytes to read.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
static Bool readMemoryDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt16 page, UInt8 *pMem, UInt8 size)
{
    Bool success;
    UInt8 buffer[DS2408_NUM_PAGES + DS2408_NUM_BYTES_IN_OVERHEAD];
    UInt8 count=0;
    UInt8 i;
    UInt16 lastCrc16;

    ASSERT_PARAM (page >= DS2408_FIRST_PAGE, page);
    ASSERT_PARAM (page <= DS2408_LAST_USEFUL_PAGE, page);
    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pSerialNumber[0] == PIO_FAM, pSerialNumber[0]);
    ASSERT_PARAM (size <= DS2408_NUM_USEFUL_PAGES, size);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to read the memory */
    success = owAccess (portNumber);
    if (success)
    {
        /* Read the memory */
        buffer[count++] = DS2408_COMMAND_READ_PIO_REGISTERS;
        buffer[count++] = page; /* It's a two byte address, little end first */
        buffer[count++] = page >> 8;

        for (i = 0; i < (DS2408_LAST_PAGE - page) + 1 + DS2408_NUM_BYTES_IN_CRC; i++)
        {
            buffer[count++] = 0xFF;
        }

        if (owBlock (portNumber, FALSE, buffer, count))
        {
            /* Check CRC over all the bytes sent and received, apart from the CRC bytes themselves */
            setcrc16 (portNumber, 0);
            for (i = 0; i < count - DS2408_NUM_BYTES_IN_CRC; i++)
            {   
                lastCrc16 = docrc16 (portNumber, (UInt16) buffer[i]);
            }

          /* The CRC bytes returned by the device are the inverse of the CRC, so XOR them
           * with what we've calculated and we should get 0xFFFF */ 
           if ((lastCrc16 ^ (buffer[count - 2] + (UInt16) (buffer[count - 1] << 8))) != 0xFFFF)
           {
               success = false;
           }
           else
           {
               if (pMem != PNULL)
               {
                   memcpy (pMem, &(buffer[DS2408_NUM_BYTES_IN_COMMAND + DS2408_NUM_BYTES_IN_PAGE_ADDRESS]), size);
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
 * Write to memory on a DS2408 device.  Note that no checking
 * is done that the write was correct, this can be done by
 * the calling function reading back the written value.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * page          the page address to recall.
 * pMem          a pointer to a location to store the data.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 * size          the number of bytes to read.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
static Bool writeMemoryDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt16 page, UInt8 *pMem, UInt8 size)
{
    Bool success;
    UInt8 buffer[DS2408_NUM_PAGES + DS2408_NUM_BYTES_IN_OVERHEAD];
    UInt8 count=0;
    UInt8 i;

    ASSERT_PARAM (page >= DS2408_FIRST_WRITEABLE_PAGE, page);
    ASSERT_PARAM (page <= DS2408_LAST_PAGE, page);
    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pSerialNumber[0] == PIO_FAM, pSerialNumber[0]);
    ASSERT_PARAM (size <= DS2408_NUM_USEFUL_PAGES, size);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to write to the memory */
    success = owAccess (portNumber);
    if (success)
    {
        /* Write to the memory */
        buffer[count++] = DS2408_COMMAND_WRITE_CS_REGISTER;
        buffer[count++] = page; /* It's a two byte address, little end first */
        buffer[count++] = page >> 8;

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
 * PUBLIC FUNCTIONS
 */

/*
 * Disable test mode on a DS2408 device.  The data sheet
 * recommends that this is done after power-on as a lock-up
 * can sometimes occur.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the operation is
 *               to be done on.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool disableTestModeDS2408 (SInt32 portNumber, UInt8 *pSerialNumber)
{
    Bool success;
    UInt8 buffer[DS2408_MINIMUM_BUFFER_SIZE];
    UInt8 count=0;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pSerialNumber[0] == PIO_FAM, pSerialNumber[0]);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to send the command */
    success = owAccess (portNumber);
    if (success)
    {
        /* Send the command */
        buffer[count++] = DS2408_COMMAND_DISABLE_TEST_MODE;

        if (!owBlock (portNumber, FALSE, buffer, count))
        {
            success = false;
        }
    }

    return success;
}

/*
 * Read the control/status register on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to a byte in which to store the result.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readControlRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_CONTROL_STATUS_REGISTER_PAGE, pData, 1);
}

/*
 * Write to the control/status register on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the write is
 *               to be done on.
 * data          the byte to be written.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeControlRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 data)
{
    Bool success;
    UInt8 readbackValue = DS2408_UNUSED_VALUE;
    
    success = writeMemoryDS2408 (portNumber, pSerialNumber, DS2408_CONTROL_STATUS_REGISTER_PAGE, &data, 1);
 
    /* Read the byte back to ensure that it was written correctly */
    if (success)
    {
        success = readControlRegisterDS2408 (portNumber, pSerialNumber, &readbackValue);
        /* Need to mask off the top nibble bit as those bits are read-only */
        readbackValue &= 0x0f;
        data &= 0x0f;
        if (success && (readbackValue != data))
        {
            success = false;
        }
    }
    
    return success;
}

/*
 * Read the PIO logic state register on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to a byte in which to store the result.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readPIOLogicStateDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_PIO_LOGIC_STATE_PAGE, pData, 1);
}

/*
 * Read the PIO's up to 32 times on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to 32 bytes in which to store the result.
 *               If PNULL then the reading is performed but no
 *               data is copied (though the return value will still
 *               indicate the amount of data that could have been
 *               returned).
 *
 * @return  the number of times the PIO has been read.
 */
UInt8 channelAccessReadDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    UInt8 buffer[DS2408_MAX_BYTES_TO_READ + DS2408_NUM_BYTES_IN_COMMAND + DS2408_NUM_BYTES_IN_CRC]; 
    UInt8 count=0;
    UInt8 i;
    UInt16 lastCrc16;
    UInt8 bytesRead = 0;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pSerialNumber[0] == PIO_FAM, pSerialNumber[0]);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to read the PIOs */
    if (owAccess (portNumber))
    {
        /* Read the PIOs */
        buffer[count++] = DS2408_COMMAND_CHANNEL_ACCESS_READ;
        while (count < sizeof (buffer))
        {
            buffer[count++] = 0xFF;
        }

        if (owBlock (portNumber, FALSE, buffer, count))
        {
            /* Check CRC over all the bytes sent and received, apart from the CRC bytes themselves */
            setcrc16 (portNumber, 0);
            for (i = 0; i < count - DS2408_NUM_BYTES_IN_CRC; i++)
            {   
                lastCrc16 = docrc16 (portNumber, (UInt16) buffer[i]);
            }

          /* The CRC bytes returned are the inverse of the CRC, so XOR them
           * with what we've calculated and we should get 0xFFFF */ 
           if ((lastCrc16 ^ (buffer[count - 2] + (UInt16) (buffer[count - 1] << 8))) == 0xFFFF)
           {
               bytesRead = DS2408_MAX_BYTES_TO_READ;
               if (pData != PNULL)
               {
                   memcpy (pData, &(buffer[DS2408_NUM_BYTES_IN_COMMAND]), DS2408_MAX_BYTES_TO_READ);
               }
           }
        }
    }

    return bytesRead;
}

/*
 * Write to the PIOs on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to the byte to be written to the PIOs
 *               and to which the status of the pins will be
 *               written-back afterwards.  LSB is P0, MSB is P7,
 *               a '1' means high, a '0' means low.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool channelAccessWriteDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    Bool success;
    UInt8 buffer[DS2408_MINIMUM_BUFFER_SIZE]; 
    UInt8 count=0;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pSerialNumber[0] == PIO_FAM, pSerialNumber[0]);
    ASSERT_PARAM (pData != PNULL, (unsigned long) pData);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to write to the PIOs */
    success = owAccess (portNumber);
    if (success)
    {
        /* Write the data to the PIOs (twice, one inverted, so that the device knows we mean it) */
        buffer[count++] = DS2408_COMMAND_CHANNEL_ACCESS_WRITE;
        buffer[count++] = *pData;
        buffer[count++] = ~(*pData);
        while (count < sizeof (buffer))
        {
            buffer[count++] = 0xFF;
        }

        if (!owBlock (portNumber, FALSE, buffer, count))
        {
            success = false;
        }
        
        if (success)
        {
            /* Check that whether the device has written back the confirmation value */
            if (buffer[3] != DS2408_CONFIRM_VALUE)
            {
                success = false;   
            }
            else
            {
                /* The next byte then contains the value read back from the PIOs,
                 * which we return in pData */
                *pData = buffer[4]; 
            }
        }
    }

    return success;
}

/*
 * Read the PIO output latch state register on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to a byte in which to store the result.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readPIOOutputLatchStateRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_PIO_OUTPUT_LATCH_STATE_REGISTER_PAGE, pData, 1);
}

/*
 * Read the PIO activity latch state register on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to a byte in which to store the result.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readPIOActivityLatchStateRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_PIO_ACTIVITY_LATCH_STATE_REGISTER_PAGE, pData, 1);
}

/*
 * Reset the activity latches on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the reset is
 *               to be done on.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool resetActivityLatchesDS2408 (SInt32 portNumber, UInt8 *pSerialNumber)
{
    Bool success;
    UInt8 buffer[DS2408_MINIMUM_BUFFER_SIZE];
    UInt8 count=0;

    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (pSerialNumber[0] == PIO_FAM, pSerialNumber[0]);
    
    /* Select the device */
    owSerialNum (portNumber, pSerialNumber, FALSE);
    
    /* Access the device to send the command */
    success = owAccess (portNumber);
    if (success)
    {
        /* Send the command and fill in 0xFF to allow for something to be read back */
        buffer[count++] = DS2408_COMMAND_RESET_ACTIVITY_LATCHES;
        while (count < sizeof (buffer))
        {
            buffer[count++] = 0xFF;
        }

        if (!owBlock (portNumber, FALSE, buffer, count))
        {
            success = false;
        }
        
        if (success)
        {
            /* Check that whether the device has written back the confirmation value */
            if (buffer[1] != DS2408_CONFIRM_VALUE)
            {
                success = false;   
            }
        }
    }

    return success;
}

/*
 * Read the conditional search channel selection mask register on
 * a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to a byte in which to store the result.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readCSChannelSelectionMaskRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_CS_CHANNEL_SELECTION_MASK_PAGE, pData, 1);
}

/*
 * Write to the conditional search channel selection mask register
 * on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the write is
 *               to be done on.
 * data          the byte to be written.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeCSChannelSelectionMaskRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 data)
{
    Bool success;
    UInt8 readbackValue = DS2408_UNUSED_VALUE;
    
    success = writeMemoryDS2408 (portNumber, pSerialNumber, DS2408_CS_CHANNEL_SELECTION_MASK_PAGE, &data, 1);
 
    /* Read the byte back to ensure that it was written correctly */
    if (success)
    {
        success = readCSChannelSelectionMaskRegisterDS2408 (portNumber, pSerialNumber, &readbackValue);
        if (success && (readbackValue != data))
        {
            success = false;
        }
    }
    
    return success;
}

/*
 * Read the conditional search channel polarity selection register on
 * a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the read is
 *               to be done on.
 * pData         a pointer to a byte in which to store the result.
 *               If PNULL then the reading is performed but no
 *               data is copied.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool readCSChannelPolaritySelectionRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_CS_CHANNEL_POLARITY_SELECTION_PAGE, pData, 1);
}

/*
 * Write to the conditional search channel polarity selection register
 * on a DS2408 device.
 *
 * portNumber    the port number of the port being used for the
 *               1-Wire Network.
 * pSerialNumber the serial number for the part that the write is
 *               to be done on.
 * data          the byte to be written.
 *
 * @return  true if the operation succeeded, otherwise false.
 */
Bool writeCSChannelPolaritySelectionRegisterDS2408 (SInt32 portNumber, UInt8 *pSerialNumber, UInt8 data)
{
    Bool success;
    UInt8 readbackValue = DS2408_UNUSED_VALUE;
    
    success = writeMemoryDS2408 (portNumber, pSerialNumber, DS2408_CS_CHANNEL_POLARITY_SELECTION_PAGE, &data, 1);
 
    /* Read the byte back to ensure that it was written correctly */
    if (success)
    {
        success = readCSChannelPolaritySelectionRegisterDS2408 (portNumber, pSerialNumber, &readbackValue);
        if (success && (readbackValue != data))
        {
            success = false;
        }
    }
    
    return success;
}

