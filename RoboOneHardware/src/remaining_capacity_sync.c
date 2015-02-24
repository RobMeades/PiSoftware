/*
 * Main for RemainingCapacitySync.
 *
 * This is not part of the hardware server itself,
 * it is a separate executable that can be run
 * at startup and shutdown of Linux to sync the
 * volatile remaining capacity settings in the DS2438
 * battery monitoring devices with stored values.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strerror() */
#include <errno.h> /* for errno */
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <pthread.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <messaging_client.h>
#include <hardware_types.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * EXTERNS
 */
extern int errno;

/*
 * GLOBALS (prefixed with g)
 */
 
/*
 * STATIC FUNCTIONS
 */

/*
 * Send a message to start the Hardware Server.
 *
 * batteriesOnly  if true, only setup the battery
 *                devices, otherwise setup the lot.
 * 
 * @return true if successful, otherwise false.
 */
static Bool startHardwareServer (Bool batteriesOnly)
{
    return hardwareServerSendReceive (HARDWARE_SERVER_START, &batteriesOnly, sizeof (batteriesOnly), PNULL);
}

/*
 * Send a message to stop the Hardware Server.
 * 
 * @return true if successful, otherwise false.
 */
static Bool stopHardwareServer (void)
{
    return hardwareServerSendReceive (HARDWARE_SERVER_STOP, PNULL, 0, PNULL);
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Entry point
 */
int main (int argc, char **argv)
{
    Bool   success = false;
    UInt16 remainingCapacity;
    pid_t  hwServerPID;
    
    setDebugPrintsOnToFile ("robooneremainingcapacitysync.log");
    setProgressPrintsOn();

    printProgress ("Synchronising remaining battery capacity values...\n");
    /* Spawn a child that will become the Hardware server. */
    hwServerPID = fork();
    if (hwServerPID == 0)
    {
        /* Start Hardware Server process on a given port */
        static char *argv1[] = {HARDWARE_SERVER_EXE, HARDWARE_SERVER_PORT_STRING, PNULL};
        
        execv (HARDWARE_SERVER_EXE, argv1);
        printDebug ("!!! Couldn't launch %s, err: %s. !!!\n", HARDWARE_SERVER_EXE, strerror (errno));
    }
    else
    {
        if (hwServerPID < 0)
        {
            printDebug ("!!! Couldn't fork to launch %s, err: %s. !!!\n", HARDWARE_SERVER_EXE, strerror (errno));
        }
        else
        {   /* Parent process */
            /* Wait for the server to start */
            usleep (HARDWARE_SERVER_START_DELAY_PI_US);
            /* Now setup the Hardware Server, batteries
             * only on this occasion. This will restore
             * the remaining battery capacity from
             * non-volatile storage if it is out of date,
             * which would be the case at power-on. */
            success = startHardwareServer (true);

            if (success)
            {
                /* Now do a remaining capacity reading to complete the sync back to
                 * non-volatile storage in case we are about to power off */
                if (hardwareServerSendReceive (HARDWARE_READ_RIO_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity))
                {
                    printProgress ("Rio battery has %d mAh remaining.\n", remainingCapacity);
                }
                if (hardwareServerSendReceive (HARDWARE_READ_O1_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity))
                {
                    printProgress ("O1 battery has %d mAh remaining.\n", remainingCapacity);
                }
                if (hardwareServerSendReceive (HARDWARE_READ_O2_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity))
                {
                    printProgress ("O2 battery has %d mAh remaining.\n", remainingCapacity);
                }
                if (hardwareServerSendReceive (HARDWARE_READ_O3_REMAINING_CAPACITY, PNULL, 0, &remainingCapacity))
                {
                    printProgress ("O3 battery has %d mAh remaining.\n", remainingCapacity);
                }
            }

            /* Shut the Hardware Server down gracefully */
            stopHardwareServer();
            waitpid (hwServerPID, 0, 0); /* wait for Hardware Server process to exit */
        }
    }
    
    setDebugPrintsOff();

    if (success)
    {
        printProgress ("Synchronisation complete.\n");
    }
    else
    {
        printProgress ("\nSynchronisation failed!\n");
        exit (-1);
    }

    return success;
}