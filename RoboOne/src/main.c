/*
 * One Wire bus handling thread for RoboOne.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <rob_system.h>
#include <one_wire.h>
#include <ow_bus.h>
#include <dashboard.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <one_wire_server.h>
#include <one_wire_msg_auto.h>

/*
 * MANIFEST CONSTANTS
 */

#define SERVER_EXE "./one_wire_server"
#define SERVER_PORT_STRING "5234"

/*
 * EXTERN
 */

extern int errno;

/*
 * GLOBALS (prefixed with g)
 */

SInt32 gOneWireServerPort = -1;

/*
 * STATIC FUNCTIONS
 */

/*
 * Send a message to stop the One Wire Server.
 * 
 * oneWireServerPort the port the server is listening on.
 * 
 * @return           true if successful, otherwise false.
 */
static Bool stopOneWireServer (SInt32 oneWireServerPort)
{
    ClientReturnCode returnCode;
    Bool success = true;
    Msg *pSendMsg;
    Msg *pReceivedMsg;
    
    ASSERT_PARAM (oneWireServerPort >= 0, oneWireServerPort);
    
    pSendMsg = malloc (sizeof (Msg));
    
    if (pSendMsg != PNULL)
    {
        /* Setup an exit message, only has the type in it */
        pSendMsg->msgLength = 1;
        pSendMsg->msgType = ONE_WIRE_SERVER_EXIT;
        
        pReceivedMsg = malloc (sizeof (Msg));
        if (pReceivedMsg != PNULL)
        {
            returnCode = runMessagingClient (oneWireServerPort, pSendMsg, pReceivedMsg);
            
            if ((returnCode != CLIENT_SUCCESS) || (pReceivedMsg->msgType != ONE_WIRE_SERVER_EXIT))
            {
                success = false;    
            }

            free (pReceivedMsg);
        }
        else
        {
            success = false;
        }        
        
        free (pSendMsg);
    }
    else
    {
        success = false;
    }

    return success;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool  success = true;
    pid_t serverPID;
    
    /* setDebugPrintsOn(); */
    setProgressPrintsOn();

    /* Setup the global for everyone to use */
    gOneWireServerPort = atoi (SERVER_PORT_STRING);
    
    /* Spawn a child that will become the One Wire server. */
    serverPID = fork();
    
    if (serverPID == 0)
    {
        /* Start OneWire server process on port 5000 */
        static char *argv[]={SERVER_EXE, SERVER_PORT_STRING, PNULL};
        
        execv (SERVER_EXE, argv);
        printDebug ("Couldn't launch %s, err: %s\n", SERVER_EXE, strerror (errno));
        success = false;
    }
    else
    { /* Parent process */
      /* Setup what's necessary for OneWire bus stuff */
        usleep (SERVER_START_DELAY_PI_US); /* Wait for the server to start */
        success = startOneWireBus();
        
        /* Find and setup the devices on the OneWire bus */
        if (success)
        {
            success = setupDevices();
            
            if (success)
            {
                /* Display the dashboard */
                success = runDashboard();
            }
            else
            {
                /* If the setup fails, print out what devices we can find */
                findAllDevices();
            }
        }
        
        if (success)
        {
            printProgress ("\nDone.\n");
        }
        else
        {
            printProgress ("\nFailed!\n");
        }
        
        /* Shut things down gracefully */
        stopOneWireBus ();
        
        /* Stop the server */
        stopOneWireServer (gOneWireServerPort);
        
        waitpid (serverPID, 0, 0); /* wait for server to exit */
    }
    
    return success;
}