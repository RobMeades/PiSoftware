/*
 * Communications with Orangutan (AKA Hindbrain).
 * Largely borrowed from http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <rob_system.h>
#include <orangutan.h>

/*
 * MANIFEST CONSTANTS
 */

#define ORANGUTAN_PORT_STRING             "/dev/OrangutanUSB"
#define ORANGUTAN_BAUD_RATE               B9600
#define ORANGUTAN_WAIT_TIMEOUT_TENTHS_SEC 20
#define ORANGUTAN_RESPONSE_TERMINATOR     '\r' /* CR */
#define ORANGUTAN_BUFFER_SIZE             255

/*
 * GLOBALS - prefixed with g
 */
static struct termios gSavedSettings;
static SInt32 gFd = -1;

/*
 * STATIC FUNCTIONS
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Open the Orangutan port.
 * 
 * @return  file descriptor, negative
 *          on failure.
 */
SInt32 openOrangutan (void)
{
    struct termios newSettings;
    
    gFd = open (ORANGUTAN_PORT_STRING, O_RDWR | O_NOCTTY); 
  
    if (gFd >= 0)
    {
        tcgetattr (gFd, &gSavedSettings); /* save current port settings */
          
        memset (&newSettings, 0, sizeof (newSettings));
        cfsetospeed(&newSettings, ORANGUTAN_BAUD_RATE);
        cfsetispeed(&newSettings, ORANGUTAN_BAUD_RATE);
        newSettings.c_cflag |= CS8 | CLOCAL | CREAD;        
        newSettings.c_iflag |= IGNBRK | IGNPAR;
        
        newSettings.c_cc[VTIME] = ORANGUTAN_WAIT_TIMEOUT_TENTHS_SEC;
        newSettings.c_cc[VMIN]  = 0;
          
        tcflush (gFd, TCIOFLUSH);
        tcsetattr (gFd, TCSANOW, &newSettings);
    }
    
    return gFd;
}

/*
 * Close the Orangutan port.
 */
void closeOrangutan (void)
{
    if (gFd >=0)
    {
        /* Restore terminal settings */
        tcsetattr (gFd, TCSANOW, &gSavedSettings);
        close (gFd);
    }
}

/*
 * Send a string to the Orangutan and wait for a response.
 * 
 * pSendString          pointer to a null terminated string
 *                      to send.
 * pReceiveString       pointer to a location where the
 *                      response can be placed.  A null
 *                      terminator will be included.  If
 *                      this is PNULL then this function
 *                      does not wait for a response.
 * pReceiveStringLength pointer to the length of the received
 *                      string.  This should be set by the
 *                      caller to the maximum received string
 *                      that can be stored (including a null
 *                      terminator).  It will be set by this
 *                      function to the length of the received
 *                      string (with guaranteed null terminator).
 *                      If the calling function sets this value
 *                      to zero then this function does not wait
 *                      for a response.  
 * 
 * @return              true if successful, otherwise false.
 */
Bool sendStringToOrangutan (Char *pSendString, Char *pReceiveString, UInt32 *pReceiveStringLength)
{
    Bool success = false;
    Bool done = false;
    SInt32 bytesReceived;
    Char buffer[ORANGUTAN_BUFFER_SIZE];
    SInt32 bytesToSend = strlen (pSendString);
    UInt8 i;
    
    ASSERT_PARAM (pSendString != PNULL, (unsigned long) pSendString);

    if (gFd >= 0)
    {
        /* Stop overruns */
        if (bytesToSend > ORANGUTAN_BUFFER_SIZE)
        {
            bytesToSend = ORANGUTAN_BUFFER_SIZE;
        }
        
        tcflush (gFd, TCIOFLUSH);
        
        /* Write the string, excluding the terminator */
        if (write (gFd, pSendString, bytesToSend) == bytesToSend)
        {
            success = true;
            /* If we need to, wait for the response string */
            if ((pReceiveString != PNULL) && (pReceiveStringLength != PNULL) && (*pReceiveStringLength > 0))
            {
                UInt32 maxBytesToReceive;
    
                /* Store the maximum length and replace it with zero */
                maxBytesToReceive = *pReceiveStringLength;
                *pReceiveStringLength = 0;
                
                while (!done)
                {
                    bytesReceived = read (gFd, &buffer[0], sizeof (buffer));
                    
                    if (bytesReceived > 0)
                    {
                        for (i = 0; (i < bytesReceived) && !done; i++)
                        {
                            if (buffer[i] == ORANGUTAN_RESPONSE_TERMINATOR)
                            {
                                done = true;
                                bytesReceived = i + 1; /* Chop off at the terminator */
                            }
                        }
                        
                        /* Stop overruns */
                        if (*pReceiveStringLength + bytesReceived > maxBytesToReceive - 1) /* -1 to leave room for adding a terminator */
                        {
                            bytesReceived = maxBytesToReceive - *pReceiveStringLength - 1;
                        }
                        
                        /* Copy to the output and set the length */
                        memcpy (pReceiveString + *pReceiveStringLength, &buffer[0], bytesReceived);                        
                        *pReceiveStringLength += bytesReceived;                            
                    }
                    else
                    {
                        done = true;
                    }
                }
                
                /* Add a null terminator if we're done */
                if (maxBytesToReceive > 0)
                {
                    *(pReceiveString + *pReceiveStringLength) = 0;
                    (*pReceiveStringLength)++;
                }
            }
        }
    }
    
    return success;
}