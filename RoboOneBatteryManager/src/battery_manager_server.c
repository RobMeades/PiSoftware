/*
 * battery_manager_server.c
 * Builds the Battery Manager into a sockets server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <messaging_server.h>
#include <task_handler_types.h>
#include <hardware_types.h>
#include <state_machine_server.h>
#include <state_machine_msg_auto.h>
#include <state_machine_client.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <battery_manager_server.h>
#include <battery_manager_msg_auto.h>
#include <battery_manager_client.h>

/*
 * MANIFEST CONSTANTS
 */

/* Thresholds */
#define MINIMUM_CHARGE_CHARGING_PERMITTED 2000
#define MINIMUM_CHARGE_CHARGING_NOT_PERMITTED 1800
#define FULL_CHARGE 2150
#define CHARGE_HYSTERESIS 100
#define MAXIMUM_TEMPERATURE_C 60
#define TEMPERATURE_BROKEN_C -20
#define TEMPERATURE_HYSTERESIS_C 10

/*
 * TYPES
 */

typedef struct BatteryDataContainerTag
{
    Bool updated;
    BatteryStatus batteryStatus;
    BatteryData batteryData;
}  BatteryContainerData;

/*
 * EXTERNS
 */
extern Char *pgBatteryManagerMessageNames[];

/*
 * GLOBALS - prefixed with g
 */

Bool gChargingPermitted = false;
Bool gAllFullyCharged = false;
Bool gAllInsufficientCharge = false;
Bool gInsufficientChargeThreshold = MINIMUM_CHARGE_CHARGING_NOT_PERMITTED;
BatteryContainerData gBatteryDataContainerRio;
BatteryContainerData gBatteryDataContainerO1;
BatteryContainerData gBatteryDataContainerO2;
BatteryContainerData gBatteryDataContainerO3;

/*
 * STATIC FUNCTIONS
 */

/*
 * Determine the charge state of the robot as a whole
 * and inform the state machine of it if necessary.
 */
static void signalChargeStateAll (void)
{
    if (!gAllInsufficientCharge)
    {
        printDebug ("Sufficient charge.\n");
        if ((gBatteryDataContainerRio.batteryStatus.insufficientCharge) ||
            (gBatteryDataContainerO1.batteryStatus.insufficientCharge) ||
            (gBatteryDataContainerO2.batteryStatus.insufficientCharge) ||
            (gBatteryDataContainerO3.batteryStatus.insufficientCharge))
        {
            printDebug ("...but now a battery is insufficiently charged.\n");
            gAllInsufficientCharge = true;
            stateMachineServerSendReceive (STATE_MACHINE_EVENT_INSUFFICIENT_CHARGE, PNULL, 0, PNULL, 0);    
        }
    }
    else
    {
        printDebug ("Insufficient charge.\n");
        if ((gBatteryDataContainerRio.batteryData.remainingCapacity > gInsufficientChargeThreshold + CHARGE_HYSTERESIS) ||
            (gBatteryDataContainerO1.batteryData.remainingCapacity > gInsufficientChargeThreshold + CHARGE_HYSTERESIS) ||
            (gBatteryDataContainerO2.batteryData.remainingCapacity > gInsufficientChargeThreshold + CHARGE_HYSTERESIS) ||
            (gBatteryDataContainerO3.batteryData.remainingCapacity > gInsufficientChargeThreshold + CHARGE_HYSTERESIS))
        {
            printDebug ("...but now a battery is sufficiently charged.\n");
            gAllInsufficientCharge = false;                
        }
    }
    
    if (!gAllFullyCharged)
    {
        printDebug ("Not all fully charged.\n");
        if ((gBatteryDataContainerRio.batteryData.remainingCapacity > FULL_CHARGE) &&
            (gBatteryDataContainerO1.batteryData.remainingCapacity > FULL_CHARGE) &&
            (gBatteryDataContainerO2.batteryData.remainingCapacity > FULL_CHARGE) &&
            (gBatteryDataContainerO3.batteryData.remainingCapacity > FULL_CHARGE))
        {
            printDebug ("...but now all batteries are fully charged.\n");
            gAllFullyCharged = true;
            stateMachineServerSendReceive (STATE_MACHINE_EVENT_FULLY_CHARGED, PNULL, 0, PNULL, 0);    
        }
    }
    else
    {
        printDebug ("All fully charged.\n");
        if ((gBatteryDataContainerRio.batteryData.remainingCapacity < FULL_CHARGE - CHARGE_HYSTERESIS) ||
            (gBatteryDataContainerO1.batteryData.remainingCapacity < FULL_CHARGE - CHARGE_HYSTERESIS) ||
            (gBatteryDataContainerO2.batteryData.remainingCapacity < FULL_CHARGE - CHARGE_HYSTERESIS) ||
            (gBatteryDataContainerO3.batteryData.remainingCapacity < FULL_CHARGE - CHARGE_HYSTERESIS))
        {
            printDebug ("...but now one battery is not fully charged.\n");
            gAllFullyCharged = false;                
        }
    }
}

/*
 * Determine whether a battery charger should be on or
 * off based on what we know.
 *
 * pBatteryContainerData  pointer to the data about the battery
 * 
 * @return                true if the battery charger should be
 *                        on, otherwise false.
 */
static Bool setChargerStatus (BatteryContainerData *pBatteryContainerData)
{
    ASSERT_PARAM (pBatteryContainerData != PNULL, (unsigned long) pBatteryContainerData);

    printDebug ("Voltage %d V.\n", pBatteryContainerData->batteryData.voltage);
    printDebug ("Current %d mA.\n", pBatteryContainerData->batteryData.current);
    printDebug ("Capacity %d mAh.\n", pBatteryContainerData->batteryData.remainingCapacity);
    printDebug ("Temperature %f C.\n", pBatteryContainerData->batteryData.temperature);
    printDebug ("Lifetime charge %d mAh.\n", pBatteryContainerData->batteryData.chargeDischarge.charge);
    printDebug ("Lifetime discharge %d mAh.\n", pBatteryContainerData->batteryData.chargeDischarge.discharge);
    
    if (!pBatteryContainerData->batteryStatus.overTemperature)
    {
        if (pBatteryContainerData->batteryData.temperature > MAXIMUM_TEMPERATURE_C)
        {
            printDebug ("!!! now over-temperature.\n");
            pBatteryContainerData->batteryStatus.overTemperature = true;
            pBatteryContainerData->batteryStatus.chargerOn = false;        
        }
        else
        {            
            if (!pBatteryContainerData->batteryStatus.temperatureBroken)
            {
                if (pBatteryContainerData->batteryData.temperature < TEMPERATURE_BROKEN_C)
                {
                    printDebug ("### temperature reading now broken.\n");
                    pBatteryContainerData->batteryStatus.temperatureBroken = true;
                    pBatteryContainerData->batteryStatus.chargerOn = false;        
                }
                else
                {
                    
                    if (!pBatteryContainerData->batteryStatus.insufficientCharge)
                    {
                        if (pBatteryContainerData->batteryData.remainingCapacity < gInsufficientChargeThreshold)
                        {
                            printDebug ("--- now insufficiently charged.\n");
                            pBatteryContainerData->batteryStatus.insufficientCharge = true;
                            pBatteryContainerData->batteryStatus.chargerOn = true;
                        }
                    }
                    else
                    {
                        if (pBatteryContainerData->batteryData.remainingCapacity > gInsufficientChargeThreshold + CHARGE_HYSTERESIS)
                        {
                            printDebug ("... now sufficiently charged.\n");
                            pBatteryContainerData->batteryStatus.insufficientCharge = false;                
                        }
                    }
                    
                    if (!pBatteryContainerData->batteryStatus.fullyCharged)
                    {
                        if (pBatteryContainerData->batteryData.remainingCapacity > FULL_CHARGE)
                        {
                            printDebug ("+++ now fully charged.\n");
                            pBatteryContainerData->batteryStatus.fullyCharged = true;
                            pBatteryContainerData->batteryStatus.chargerOn = false;
                        }
                    }
                    else
                    {
                        if (pBatteryContainerData->batteryData.remainingCapacity < FULL_CHARGE - CHARGE_HYSTERESIS)
                        {
                            printDebug ("... no longer fully charged.\n");
                            gBatteryDataContainerRio.batteryStatus.fullyCharged = false;                
                        }
                    }
                }
            }
            else
            {
                if (pBatteryContainerData->batteryData.temperature > TEMPERATURE_BROKEN_C + TEMPERATURE_HYSTERESIS_C)
                {
                    printDebug ("    temperature reading no longer broken.\n");
                    pBatteryContainerData->batteryStatus.temperatureBroken = false;
                }                
            }
        }
    }
    else
    {
        if (pBatteryContainerData->batteryData.temperature < MAXIMUM_TEMPERATURE_C - TEMPERATURE_HYSTERESIS_C)
        {
            printDebug ("    no longer over-temperature.\n");
            pBatteryContainerData->batteryStatus.overTemperature = false;
        }
    }
    
    if (pBatteryContainerData->batteryStatus.chargerOn)
    {
        printDebug ("So: this charger ON.\n");
    }
    else
    {
        printDebug ("So: this charger OFF.\n");        
    }
    
    return pBatteryContainerData->batteryStatus.chargerOn;
}

/*
 * Switch the RIO battery charger on or off based
 * on what we know.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool setRioChargerStatus (void)
{
    Bool success = true;

    printDebug ("Pi/Rio battery data:\n");
    if (setChargerStatus (&gBatteryDataContainerRio))
    {
        if (gChargingPermitted)
        {
            success = hardwareServerSendReceive (HARDWARE_SET_RIO_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
        }
    }
    else
    {
        success = hardwareServerSendReceive (HARDWARE_SET_RIO_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);               
    }
    
    return success;
}

/*
 * Switch the O1 battery charger on or off based
 * on what we know.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool setO1ChargerStatus (void)
{
    Bool success = true;
    
    printDebug ("O1 battery data:\n");
    if (setChargerStatus (&gBatteryDataContainerO1))
    {
        if (gChargingPermitted)
        {
            success = hardwareServerSendReceive (HARDWARE_SET_O1_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
        }
    }
    else
    {
        success = hardwareServerSendReceive (HARDWARE_SET_O1_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);               
    }
    
    return success;
}

/*
 * Switch the O2 battery charger on or off based
 * on what we know.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool setO2ChargerStatus (void)
{
    Bool success = true;
    
    printDebug ("O2 battery data:\n");
    if (setChargerStatus (&gBatteryDataContainerO2))
    {
        if (gChargingPermitted)
        {
            success = hardwareServerSendReceive (HARDWARE_SET_O2_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
        }
    }
    else
    {
        success = hardwareServerSendReceive (HARDWARE_SET_O2_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);               
    }
    
    return success;
}

/*
 * Switch the O3 battery charger on or off based
 * on what we know.
 * 
 * @return  true if successful, otherwise false.
 */
static Bool setO3ChargerStatus (void)
{
    Bool success = true;
    
    printDebug ("O3 battery data:\n");
    if (setChargerStatus (&gBatteryDataContainerO3))
    {
        if (gChargingPermitted)
        {
            success = hardwareServerSendReceive (HARDWARE_SET_O3_BATTERY_CHARGER_ON, PNULL, 0, PNULL);
        }
    }
    else
    {
        success = hardwareServerSendReceive (HARDWARE_SET_O3_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);               
    }
    
    return success;
}

/*
 * Handle a message that will cause us to start.
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionBatteryManagerServerStart (BatteryManagerServerStartCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    gChargingPermitted = false;
    gAllFullyCharged = false;
    gAllInsufficientCharge = false;
    
    memset (&gBatteryDataContainerRio, false, sizeof (gBatteryDataContainerRio));
    memset (&gBatteryDataContainerO1, false, sizeof (gBatteryDataContainerO1));
    memset (&gBatteryDataContainerO2, false, sizeof (gBatteryDataContainerO2));
    memset (&gBatteryDataContainerO3, false, sizeof (gBatteryDataContainerO3));
    
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that will cause us to stop.
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionBatteryManagerServerStop (BatteryManagerServerStopCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that gives us the status of a battery.
 * 
 * msgType  the message type that came in.
 * pData    pointer to the battery data.
 * 
 * @return  the length of the message body
 *          to send back.
 */
static UInt16 actionBatteryManagerData (BatteryManagerMsgType msgType, BatteryData * pData, BatteryStatus * pStatus)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pData != PNULL, (unsigned long) pData);
    ASSERT_PARAM (pStatus != PNULL, (unsigned long) pStatus);

    switch (msgType)
    {
        case BATTERY_MANAGER_DATA_RIO:
        {
            gBatteryDataContainerRio.updated = true;
            memcpy (&(gBatteryDataContainerRio.batteryData), pData, sizeof (gBatteryDataContainerRio.batteryData));
            setRioChargerStatus();
            memcpy (pStatus, &(gBatteryDataContainerRio.batteryStatus), sizeof (*pStatus));
            sendMsgBodyLength += sizeof (*pStatus);
        }
        break;
        case BATTERY_MANAGER_DATA_O1:
        {
            gBatteryDataContainerO1.updated = true;
            memcpy (&(gBatteryDataContainerO1.batteryData), pData, sizeof (gBatteryDataContainerO1.batteryData));
            setO1ChargerStatus();
            memcpy (pStatus, &(gBatteryDataContainerO1.batteryStatus), sizeof (*pStatus));
            sendMsgBodyLength += sizeof (*pStatus);
        }
        break;
        case BATTERY_MANAGER_DATA_O2:
        {
            gBatteryDataContainerO2.updated = true;
            memcpy (&(gBatteryDataContainerO2.batteryData), pData, sizeof (gBatteryDataContainerO2.batteryData));
            setO2ChargerStatus();
            memcpy (pStatus, &(gBatteryDataContainerO2.batteryStatus), sizeof (*pStatus));
            sendMsgBodyLength += sizeof (*pStatus);
        }
        break;
        case BATTERY_MANAGER_DATA_O3:
        {
            gBatteryDataContainerO3.updated = true;
            memcpy (&(gBatteryDataContainerO3.batteryData), pData, sizeof (gBatteryDataContainerO3.batteryData));
            setO3ChargerStatus();
            memcpy (pStatus, &(gBatteryDataContainerO3.batteryStatus), sizeof (*pStatus));
            sendMsgBodyLength += sizeof (*pStatus);
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    /* Tell whoever needs to know about the charge state */
    signalChargeStateAll();
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that tells us whether charging
 * is permitted or not.
 * 
 * isPermitted  true if charging is permitted, otherwise
 *              it is not.
 * 
 * @return      the length of the message body
 *              to send back.
 */
static UInt16 actionBatteryManagerChargingPermitted (Bool isPermitted)
{
    if (isPermitted)
    {
        printDebug ("Battery charging is permitted.\n");
        gInsufficientChargeThreshold = MINIMUM_CHARGE_CHARGING_PERMITTED;
    }
    else
    {
        printDebug ("Battery charging is NOT permitted.\n");       
        gInsufficientChargeThreshold = MINIMUM_CHARGE_CHARGING_NOT_PERMITTED;
    }
    
    if (gChargingPermitted && !isPermitted)
    {
        hardwareServerSendReceive (HARDWARE_SET_RIO_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);
        hardwareServerSendReceive (HARDWARE_SET_O1_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);
        hardwareServerSendReceive (HARDWARE_SET_O2_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);
        hardwareServerSendReceive (HARDWARE_SET_O3_BATTERY_CHARGER_OFF, PNULL, 0, PNULL);       
    }
    
    /* No need to check if charging is permitted now, the function calls
     * below will do that anyway */ 
    
    gChargingPermitted = isPermitted;
    
    setRioChargerStatus();
    setO1ChargerStatus();
    setO2ChargerStatus();
    setO3ChargerStatus();
    
    signalChargeStateAll();
    
    return 0;
}

/*
 * Handle the received message and implement the action.
 * 
 * receivedMsgType  the msgType, extracted from the
 *                  received mesage.
 * pReceivedMsgBody pointer to the body part of the
 *                  received message.
 * pSendMsg         pointer to a message that we
 *                  can fill in with the response.
 * 
 * @return          SERVER_SUCCESS_KEEP_RUNNING unless
 *                  exiting in which case SERVER_EXIT_NORMALLY.
 */
static ServerReturnCode doAction (BatteryManagerMsgType receivedMsgType, UInt8 * pReceivedMsgBody, Msg * pSendMsg)
{
    ServerReturnCode returnCode = SERVER_SUCCESS_KEEP_RUNNING;
        
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);
    
    pSendMsg->msgLength = 0;

    /* We always respond with the same message type */
    pSendMsg->msgType = (MsgType) receivedMsgType;
    /* Fill in the length so far, will make it right for each message later */
    pSendMsg->msgLength += sizeof (pSendMsg->msgType);
    
    /* Now handle each message specifically */
    switch (receivedMsgType)
    {
        case BATTERY_MANAGER_SERVER_START:
        {
            pSendMsg->msgLength += actionBatteryManagerServerStart ((BatteryManagerServerStartCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case BATTERY_MANAGER_SERVER_STOP:
        {
            pSendMsg->msgLength += actionBatteryManagerServerStop ((BatteryManagerServerStopCnf *) &(pSendMsg->msgBody[0]));
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        case BATTERY_MANAGER_DATA_RIO:
        {
            BatteryData * pBatteryData = &(((BatteryManagerDataRioInd *) pReceivedMsgBody)->data);
            BatteryManagerDataRioRsp *pRsp = (BatteryManagerDataRioRsp *) &(pSendMsg->msgBody[0]);
            pSendMsg->msgLength += actionBatteryManagerData (receivedMsgType, pBatteryData, &(pRsp->status));
        }
        break;
        case BATTERY_MANAGER_DATA_O1:
        {
            BatteryData * pBatteryData = &(((BatteryManagerDataO1Ind *) pReceivedMsgBody)->data);
            BatteryManagerDataO1Rsp *pRsp = (BatteryManagerDataO1Rsp *) &(pSendMsg->msgBody[0]);
            pSendMsg->msgLength += actionBatteryManagerData (receivedMsgType, pBatteryData, &(pRsp->status));
        }
        break;
        case BATTERY_MANAGER_DATA_O2:
        {
            BatteryData * pBatteryData = &(((BatteryManagerDataO2Ind *) pReceivedMsgBody)->data);
            BatteryManagerDataO2Rsp *pRsp = (BatteryManagerDataO2Rsp *) &(pSendMsg->msgBody[0]);
            pSendMsg->msgLength += actionBatteryManagerData (receivedMsgType, pBatteryData, &(pRsp->status));
        }
        break;
        case BATTERY_MANAGER_DATA_O3:
        {
            BatteryData * pBatteryData = &(((BatteryManagerDataO3Ind *) pReceivedMsgBody)->data);
            BatteryManagerDataO3Rsp *pRsp = (BatteryManagerDataO3Rsp *) &(pSendMsg->msgBody[0]);
            pSendMsg->msgLength += actionBatteryManagerData (receivedMsgType, pBatteryData, &(pRsp->status));
        }
        break;
        case BATTERY_MANAGER_CHARGING_PERMITTED:
        {
            Bool isPermitted = ((BatteryManagerChargingPermittedInd *) pReceivedMsgBody)->isPermitted;
            pSendMsg->msgLength += actionBatteryManagerChargingPermitted (isPermitted);            
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (receivedMsgType);   
        }
        break;
    }
    
    return returnCode;
}

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Handle a whole message received from the client
 * and send back a response.
 * 
 * pReceivedMsg   a pointer to the buffer containing the
 *                incoming message.
 * pSendMsg       a pointer to a message buffer to put
 *                the response into. Not touched if return
 *                code is a failure one.
 * 
 * @return        whatever doAction() returns.
 */
ServerReturnCode serverHandleMsg (Msg *pReceivedMsg, Msg *pSendMsg)
{
    ServerReturnCode returnCode;
    
    ASSERT_PARAM (pReceivedMsg != PNULL, (unsigned long) pReceivedMsg);
    ASSERT_PARAM (pSendMsg != PNULL, (unsigned long) pSendMsg);

    /* Check the type */
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_BATTERY_MANAGER_MSGS, pReceivedMsg->msgType);
    
    printDebug ("BM Server received message %s, length %d.\n", pgBatteryManagerMessageNames[pReceivedMsg->msgType], pReceivedMsg->msgLength);
    printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);
    /* Do the thang */
    returnCode = doAction ((BatteryManagerMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
    printDebug ("BM Server responding with message %s, length %d.\n", pgBatteryManagerMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
        
    return returnCode;
}
