/*
 * utils.c
 * Utility functions for OneWire stuf
 */ 

#include <stdio.h>
#include <string.h>
#include <ownet.h>
#include <findtype.h>
#include <rob_system.h>
#include <one_wire.h>

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Find all devices on the OneWire bus.
 *
 * portNumber      the port number of the port being used for the
 *                 1-Wire Network.
 * pAddress        a pointer to an array of 8-byte arrays to store
 *                 the addresses of devices found.  May be PNULL,
 *                 in which case the search is performed but no
 *                 addresses are stored.
 * maxNumAddresses the number of 8-byte addresses that can be stored
 *                 at pAddress.
 *
 * @return  the number of devices found (which can be larger than
 *          maxNumAddresses).
 */
UInt8 owFindAllDevices (UInt8 portNumber, UInt8 *pAddress, UInt8 maxNumAddresses)
{
    Bool success;
    UInt8 count=0;

    /* Find the first device */
    success = owFirst (portNumber, true, false);
    while (success)
    {        
        count++;
        
        /* Copy out the address that was found */
        if (pAddress != PNULL && count <= maxNumAddresses)
        {
            owSerialNum (portNumber, pAddress, true);
            pAddress += NUM_BYTES_IN_SERIAL_NUM;
        }
        
        /* Find the next device */
        success = owNext(portNumber, true, false);
    }

    return count;
}