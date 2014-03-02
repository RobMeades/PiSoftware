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

#define DS2408_NUM_BYTES_IN_CRC                          2
#define DS2408_NUM_BYTES_IN_COMMAND                      1
#define DS2408_NUM_BYTES_IN_PAGE_ADDRESS                 2
#define DS2408_NUM_BYTES_IN_OVERHEAD                     (DS2408_NUM_BYTES_IN_CRC + DS2408_NUM_BYTES_IN_COMMAND + DS2408_NUM_BYTES_IN_PAGE_ADDRESS)
#define DS2408_COMMAND_DISABLE_TEST_MODE                 0x3C
#define DS2408_COMMAND_READ_PIO_REGISTERS                0xF0
#define DS2408_COMMAND_CHANNEL_ACCESS_READ               0xF5
#define DS2408_COMMAND_CHANNEL_ACCESS_WRITE              0x5A
#define DS2408_COMMAND_WRITE_CONDITIONAL_SEARCH_REGISTER 0xCC
#define DS2408_COMMAND_RESET_ACTIVITY_LATCHES            0xC3

#define DS2408_FIRST_PAGE                                         0x0088
#define DS2408_PIO_LOGIC_STATE_PAGE                               (DS2408_FIRST_PAGE)
#define DS2408_PIO_OUTPUT_LATCH_STATE_REGISTER_PAGE               (DS2408_FIRST_PAGE + 1)
#define DS2408_PIO_ACTIVITY_LATCH_STATE_REGISTER_PAGE             (DS2408_FIRST_PAGE + 2)
#define DS2408_CONDITIONAL_SEARCH_CHANNEL_SELECTION_MASK_PAGE     (DS2408_FIRST_PAGE + 3)
#define DS2408_CONDITIONAL_SEARCH_CHANNEL_POLARITY_SELECTION_PAGE (DS2408_FIRST_PAGE + 4)
#define DS2408_CONTROL_STATUS_REGISTER_PAGE                       (DS2408_FIRST_PAGE + 5)
#define DS2408_LAST_PAGE                                          0x008F
#define DS2408_NUM_PAGES                                          (DS2408_LAST_PAGE - DS2408_FIRST_PAGE + 1)

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
static Bool readMemoryDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt16 page, UInt8 *pMem, UInt8 size)
{
    Bool success;
    UInt8 buffer[DS2408_NUM_PAGES + DS2408_NUM_BYTES_IN_OVERHEAD];
    UInt8 count=0;
    UInt8 i;
    UInt16 lastCrc16;

    ASSERT_PARAM (page >= DS2408_FIRST_PAGE, page);
    ASSERT_PARAM (page <= DS2408_LAST_PAGE, page);
    ASSERT_PARAM (pSerialNumber != PNULL, (unsigned long) pSerialNumber);
    ASSERT_PARAM (size <= DS2408_NUM_PAGES, size);
    
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
 * PUBLIC FUNCTIONS
 */

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
Bool readPIOLogicStateDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_PIO_LOGIC_STATE_PAGE, pData, 1);
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
Bool readPIOOutputLatchStateRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
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
Bool readPIOActivityLatchStateRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_PIO_ACTIVITY_LATCH_STATE_REGISTER_PAGE, pData, 1);
}

/*
 * Read the conrol/status register on a DS2408 device.
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
Bool readControlRegisterDS2408 (UInt8 portNumber, UInt8 *pSerialNumber, UInt8 *pData)
{
    return readMemoryDS2408 (portNumber, pSerialNumber, DS2408_CONTROL_STATUS_REGISTER_PAGE, pData, 1);
}
