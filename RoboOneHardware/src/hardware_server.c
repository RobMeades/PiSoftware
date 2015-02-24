/*
 * hardware_server.c
 * Builds knowledge of the RoboOne hardware, plus the onewire.a library, into a sockets server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rob_system.h>
#include <one_wire.h>
#include <messaging_server.h>
#include <hardware_types.h>
#include <hardware_server.h>
#include <hardware_msg_auto.h>
#include <hardware_client.h>
#include <ow_bus.h>
#include <orangutan.h>

/*
 * MANIFEST CONSTANTS
 */

/*
 * TYPES
 */

/*
 * EXTERNS
 */
extern Char *pgHardwareMessageNames[];

/*
 * GLOBALS - prefixed with g
 */

/*
 * STATIC FUNCTIONS
 */

/*
 * Handle a message that will cause us to start.
 * 
 * batteriesOnly if true, only start up the battery
 *               related items on the OneWire bus
 *               (useful when just synchronising
 *               remaining battery capacity values),
 *               otherwise setup the PIOs as well.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionHardwareServerStart (Bool batteriesOnly, HardwareServerStartCnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
   /* First of all, start up the OneWire bus */
    success = startOneWireBus();      
    if (success)
    {
        /* Find and setup the devices on the OneWire bus */
        success = setupDevices (batteriesOnly);
        if (!success)
        {
            /* If the setup fails, print out what devices we can find */
            findAllDevices();
        }
    }
    
    pSendMsgBody->success = success;
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
static UInt16 actionHardwareServerStop (HardwareServerStopCnf *pSendMsgBody)
{
    UInt16 sendMsgBodyLength = 0;

    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    /* Shut the OneWire stuff down gracefully */
    stopOneWireBus ();
    
    /* Shut the Orangutan down in case it was up */
    closeOrangutan();
    
    pSendMsgBody->success = true;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that reads the charger
 * state pins.
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadChargerStatePins (HardwareReadChargerStatePinsCnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 pinsState = 0;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readChargerStatePins (&pinsState);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->pinsState = pinsState;
    sendMsgBodyLength += sizeof (pSendMsgBody->pinsState);            
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that reads the charge state.
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadChargerState (HardwareReadChargerStateCnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    HardwareChargeState chargeState;
    
    chargeState.flashDetectPossible = false;
    memset (&chargeState.state, 0, sizeof (chargeState.state));
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    
    success = readChargerState (&(chargeState.state[0]), &chargeState.flashDetectPossible);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    memcpy (&(pSendMsgBody->chargeState), &chargeState, sizeof (pSendMsgBody->chargeState));
    sendMsgBodyLength += sizeof (pSendMsgBody->chargeState);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that calls a function with
 * no return value.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionSetBool (HardwareMsgType msgType, UInt8 *pSendMsgBody)
{
    Bool success = false;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
        case HARDWARE_TOGGLE_O_PWR:
        {
            success = toggleOPwr();
        }
        break;
        case HARDWARE_TOGGLE_O_RST:
        {
            success = toggleORst();
        }
        break;
        case HARDWARE_TOGGLE_PI_RST:
        {
            success = togglePiRst();
        }
        break;
        case HARDWARE_SET_RIO_PWR_12V_ON:
        {
            success = setRioPwr12VOn();
        }
        break;
        case HARDWARE_SET_RIO_PWR_12V_OFF:
        {
            success = setRioPwr12VOff();
        }
        break;
        case HARDWARE_SET_RIO_PWR_BATT_ON:
        {
            success = setRioPwrBattOn();
        }
        break;
        case HARDWARE_SET_RIO_PWR_BATT_OFF:
        {
            success = setRioPwrBattOff();
        }
        break;
        case HARDWARE_SET_O_PWR_12V_ON:
        {
            success = setOPwr12VOn();
        }
        break;
        case HARDWARE_SET_O_PWR_12V_OFF:
        {
            success = setOPwr12VOff();
        }
        break;
        case HARDWARE_SET_O_PWR_BATT_ON:
        {
            success = setOPwrBattOn();
        }
        break;
        case HARDWARE_SET_O_PWR_BATT_OFF:
        {
            success = setOPwrBattOff();
        }
        break;
        case HARDWARE_SET_RIO_BATTERY_CHARGER_ON:
        {
            success = setRioBatteryChargerOn();
        }
        break;
        case HARDWARE_SET_RIO_BATTERY_CHARGER_OFF:
        {
            success = setRioBatteryChargerOff();
        }
        break;
        case HARDWARE_SET_O1_BATTERY_CHARGER_ON:
        {
            success = setO1BatteryChargerOn();
        }
        break;
        case HARDWARE_SET_O1_BATTERY_CHARGER_OFF:
        {
            success = setO1BatteryChargerOff();
        }
        break;
        case HARDWARE_SET_O2_BATTERY_CHARGER_ON:
        {
            success = setO2BatteryChargerOn();
        }
        break;
        case HARDWARE_SET_O2_BATTERY_CHARGER_OFF:
        {
            success = setO2BatteryChargerOff();
        }
        break;
        case HARDWARE_SET_O3_BATTERY_CHARGER_ON:
        {
            success = setO3BatteryChargerOn();
        }
        break;
        case HARDWARE_SET_O3_BATTERY_CHARGER_OFF:
        {
            success = setO3BatteryChargerOff();
        }
        break;
        case HARDWARE_SET_ALL_BATTERY_CHARGERS_ON:
        {
            success = setAllBatteryChargersOn();
        }
        break;
        case HARDWARE_SET_ALL_BATTERY_CHARGERS_OFF:
        {
            success = setAllBatteryChargersOff();
        }
        break;
        case HARDWARE_DISABLE_ON_PCB_RELAYS:
        {
            success = disableOnPCBRelays();
        }
        break;
        case HARDWARE_ENABLE_ON_PCB_RELAYS:
        {
            success = enableOnPCBRelays();
        }
        break;
        case HARDWARE_DISABLE_EXTERNAL_RELAYS:
        {
            success = disableExternalRelays();
        }
        break;
        case HARDWARE_ENABLE_EXTERNAL_RELAYS:
        {
            success = enableExternalRelays();
        }
        break;
        case HARDWARE_PERFORM_CAL_ALL_BATTERY_MONITORS:
        {
            success = performCalAllBatteryMonitors();
        }
        break;
        case HARDWARE_PERFORM_CAL_RIO_BATTERY_MONITOR:
        {
            success = performCalRioBatteryMonitor();
        }
        break;
        case HARDWARE_PERFORM_CAL_O1_BATTERY_MONITOR:
        {
            success = performCalO1BatteryMonitor();
        }
        break;
        case HARDWARE_PERFORM_CAL_O2_BATTERY_MONITOR:
        {
            success = performCalO2BatteryMonitor();
        }
        break;
        case HARDWARE_PERFORM_CAL_O3_BATTERY_MONITOR:
        {
            success = performCalO3BatteryMonitor();
        }
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);            
        }
        break;
    }
    
    /* Note that the following assumes that the header on
     * the returned message body is always the succes
     * Bool and that packing is 1 */
    *((Bool *) pSendMsgBody) = success;
    sendMsgBodyLength += sizeof (Bool);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that needs just a Bool returned.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadBool (HardwareMsgType msgType, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    Bool isOn = false;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
        case HARDWARE_READ_MAINS_12V:
        {
            success = readMains12VPin (&isOn);
            ((HardwareReadMains12VCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadMains12VCnf *) pSendMsgBody)->success);
            ((HardwareReadMains12VCnf *) pSendMsgBody)->mains12VIsPresent = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadMains12VCnf *) pSendMsgBody)->mains12VIsPresent);            
        }
        break;
        case HARDWARE_READ_O_PWR:
        {
            success = readOPwr (&isOn);
            ((HardwareReadOPwrCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadOPwrCnf *) pSendMsgBody)->success);
            ((HardwareReadOPwrCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadOPwrCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_O_RST:
        {
            success = readORst (&isOn);
            ((HardwareReadORstCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadORstCnf *) pSendMsgBody)->success);
            ((HardwareReadORstCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadORstCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_RIO_PWR_12V:
        {
            success = readRioPwr12V (&isOn);
            ((HardwareReadRioPwr12VCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadRioPwr12VCnf *) pSendMsgBody)->success);
            ((HardwareReadRioPwr12VCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadRioPwr12VCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_RIO_PWR_BATT:
        {
            success = readRioPwrBatt (&isOn);
            ((HardwareReadRioPwrBattCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadRioPwrBattCnf *) pSendMsgBody)->success);
            ((HardwareReadRioPwrBattCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadRioPwrBattCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_O_PWR_12V:
        {
            success = readOPwr12V (&isOn);
            ((HardwareReadOPwr12VCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadOPwr12VCnf *) pSendMsgBody)->success);
            ((HardwareReadOPwr12VCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadOPwr12VCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_O_PWR_BATT:
        {
            success = readOPwrBatt (&isOn);
            ((HardwareReadOPwrBattCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadOPwrBattCnf *) pSendMsgBody)->success);
            ((HardwareReadOPwrBattCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadOPwrBattCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_RIO_BATTERY_CHARGER:
        {
            success = readRioBatteryCharger (&isOn);
            ((HardwareReadRioBatteryChargerCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadRioBatteryChargerCnf *) pSendMsgBody)->success);
            ((HardwareReadRioBatteryChargerCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadRioBatteryChargerCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_O1_BATTERY_CHARGER:
        {
            success = readO1BatteryCharger (&isOn);
            ((HardwareReadO1BatteryChargerCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO1BatteryChargerCnf *) pSendMsgBody)->success);
            ((HardwareReadO1BatteryChargerCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadO1BatteryChargerCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_O2_BATTERY_CHARGER:
        {
            success = readO2BatteryCharger (&isOn);
            ((HardwareReadO2BatteryChargerCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO2BatteryChargerCnf *) pSendMsgBody)->success);
            ((HardwareReadO2BatteryChargerCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadO2BatteryChargerCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_O3_BATTERY_CHARGER:
        {
            success = readO3BatteryCharger (&isOn);
            ((HardwareReadO3BatteryChargerCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO3BatteryChargerCnf *) pSendMsgBody)->success);
            ((HardwareReadO3BatteryChargerCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadO3BatteryChargerCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_EXTERNAL_RELAYS_ENABLED:
        {
            success = readExternalRelaysEnabled (&isOn);
            ((HardwareReadExternalRelaysEnabledCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadExternalRelaysEnabledCnf *) pSendMsgBody)->success);
            ((HardwareReadExternalRelaysEnabledCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadExternalRelaysEnabledCnf *) pSendMsgBody)->isOn);            
        }
        break;
        case HARDWARE_READ_ON_PCB_RELAYS_ENABLED:
        {
            success = readOnPCBRelaysEnabled (&isOn);
            ((HardwareReadOnPCBRelaysEnabledCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadOnPCBRelaysEnabledCnf *) pSendMsgBody)->success);
            ((HardwareReadOnPCBRelaysEnabledCnf *) pSendMsgBody)->isOn = isOn;
            sendMsgBodyLength += sizeof (((HardwareReadOnPCBRelaysEnabledCnf *) pSendMsgBody)->isOn);            
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that reads the general
 * purpose IO pins
 * 
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadGeneralPurposeIOs (HardwareReadGeneralPurposeIOsCnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt8 pinsState = 0;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = readGeneralPurposeIOs (&pinsState);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    pSendMsgBody->pinsState = pinsState;
    sendMsgBodyLength += sizeof (pSendMsgBody->pinsState);            
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that reads current.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadCurrent (HardwareMsgType msgType, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    SInt16 current = 0;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
        case HARDWARE_READ_RIO_BATT_CURRENT:
        {
            success = readRioBattCurrent (&current);
            ((HardwareReadRioBattCurrentCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadRioBattCurrentCnf *) pSendMsgBody)->success);
            ((HardwareReadRioBattCurrentCnf *) pSendMsgBody)->current = current;
            sendMsgBodyLength += sizeof (((HardwareReadRioBattCurrentCnf *) pSendMsgBody)->current);            
        }
        break;
        case HARDWARE_READ_O1_BATT_CURRENT:
        {
            success = readO1BattCurrent (&current);
            ((HardwareReadO1BattCurrentCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO1BattCurrentCnf *) pSendMsgBody)->success);
            ((HardwareReadO1BattCurrentCnf *) pSendMsgBody)->current = current;
            sendMsgBodyLength += sizeof (((HardwareReadO1BattCurrentCnf *) pSendMsgBody)->current);            
        }
        break;
        case HARDWARE_READ_O2_BATT_CURRENT:
        {
            success = readO2BattCurrent (&current);
            ((HardwareReadO2BattCurrentCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO2BattCurrentCnf *) pSendMsgBody)->success);
            ((HardwareReadO2BattCurrentCnf *) pSendMsgBody)->current = current;
            sendMsgBodyLength += sizeof (((HardwareReadO2BattCurrentCnf *) pSendMsgBody)->current);            
        }
        break;
        case HARDWARE_READ_O3_BATT_CURRENT:
        {
            success = readO3BattCurrent (&current);
            ((HardwareReadO3BattCurrentCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO3BattCurrentCnf *) pSendMsgBody)->success);
            ((HardwareReadO3BattCurrentCnf *) pSendMsgBody)->current = current;
            sendMsgBodyLength += sizeof (((HardwareReadO3BattCurrentCnf *) pSendMsgBody)->current);            
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that reads voltage.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadVoltage (HardwareMsgType msgType, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt16 voltage = 0;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
        case HARDWARE_READ_RIO_BATT_VOLTAGE:
        {
            success = readRioBattVoltage (&voltage);
            ((HardwareReadRioBattVoltageCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadRioBattVoltageCnf *) pSendMsgBody)->success);
            ((HardwareReadRioBattVoltageCnf *) pSendMsgBody)->voltage = voltage;
            sendMsgBodyLength += sizeof (((HardwareReadRioBattVoltageCnf *) pSendMsgBody)->voltage);            
        }
        break;
        case HARDWARE_READ_O1_BATT_VOLTAGE:
        {
            success = readO1BattVoltage (&voltage);
            ((HardwareReadO1BattVoltageCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO1BattVoltageCnf *) pSendMsgBody)->success);
            ((HardwareReadO1BattVoltageCnf *) pSendMsgBody)->voltage = voltage;
            sendMsgBodyLength += sizeof (((HardwareReadO1BattVoltageCnf *) pSendMsgBody)->voltage);            
        }
        break;
        case HARDWARE_READ_O2_BATT_VOLTAGE:
        {
            success = readO2BattVoltage (&voltage);
            ((HardwareReadO2BattVoltageCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO2BattVoltageCnf *) pSendMsgBody)->success);
            ((HardwareReadO2BattVoltageCnf *) pSendMsgBody)->voltage = voltage;
            sendMsgBodyLength += sizeof (((HardwareReadO2BattVoltageCnf *) pSendMsgBody)->voltage);            
        }
        break;
        case HARDWARE_READ_O3_BATT_VOLTAGE:
        {
            success = readO3BattVoltage (&voltage);
            ((HardwareReadO3BattVoltageCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO3BattVoltageCnf *) pSendMsgBody)->success);
            ((HardwareReadO3BattVoltageCnf *) pSendMsgBody)->voltage = voltage;
            sendMsgBodyLength += sizeof (((HardwareReadO3BattVoltageCnf *) pSendMsgBody)->voltage);            
        }
        break;
        case HARDWARE_READ_ANALOGUE_MUX:
        {
            success = readAnalogueMux (&voltage);
            ((HardwareReadAnalogueMuxCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadAnalogueMuxCnf *) pSendMsgBody)->success);
            ((HardwareReadAnalogueMuxCnf *) pSendMsgBody)->voltage = voltage;
            sendMsgBodyLength += sizeof (((HardwareReadAnalogueMuxCnf *) pSendMsgBody)->voltage);            
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that reads remaining capacity.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadRemainingCapacity (HardwareMsgType msgType, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    UInt16 remainingCapacity = 0;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
        case HARDWARE_READ_RIO_REMAINING_CAPACITY:
        {
            success = readRioRemainingCapacity (&remainingCapacity);
            ((HardwareReadRioRemainingCapacityCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadRioRemainingCapacityCnf *) pSendMsgBody)->success);
            ((HardwareReadRioRemainingCapacityCnf *) pSendMsgBody)->remainingCapacity = remainingCapacity;
            sendMsgBodyLength += sizeof (((HardwareReadRioRemainingCapacityCnf *) pSendMsgBody)->remainingCapacity);            
        }
        break;
        case HARDWARE_READ_O1_REMAINING_CAPACITY:
        {
            success = readO1RemainingCapacity (&remainingCapacity);
            ((HardwareReadO1RemainingCapacityCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO1RemainingCapacityCnf *) pSendMsgBody)->success);
            ((HardwareReadO1RemainingCapacityCnf *) pSendMsgBody)->remainingCapacity = remainingCapacity;
            sendMsgBodyLength += sizeof (((HardwareReadO1RemainingCapacityCnf *) pSendMsgBody)->remainingCapacity);            
        }
        break;
        case HARDWARE_READ_O2_REMAINING_CAPACITY:
        {
            success = readO2RemainingCapacity (&remainingCapacity);
            ((HardwareReadO2RemainingCapacityCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO2RemainingCapacityCnf *) pSendMsgBody)->success);
            ((HardwareReadO2RemainingCapacityCnf *) pSendMsgBody)->remainingCapacity = remainingCapacity;
            sendMsgBodyLength += sizeof (((HardwareReadO2RemainingCapacityCnf *) pSendMsgBody)->remainingCapacity);            
        }
        break;
        case HARDWARE_READ_O3_REMAINING_CAPACITY:
        {
            success = readO3RemainingCapacity (&remainingCapacity);
            ((HardwareReadO3RemainingCapacityCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO3RemainingCapacityCnf *) pSendMsgBody)->success);
            ((HardwareReadO3RemainingCapacityCnf *) pSendMsgBody)->remainingCapacity = remainingCapacity;
            sendMsgBodyLength += sizeof (((HardwareReadO3RemainingCapacityCnf *) pSendMsgBody)->remainingCapacity);            
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that reads lifetime
 * charge/discharge.
 * 
 * msgType       the msgType, extracted from the
 *               received mesage.
 * pSendMsgBody  pointer to the relevant message
 *               type to fill in with a response,
 *               which will be overlaid over the
 *               body of the response message.
 * 
 * @return       the length of the message body
 *               to send back.
 */
static UInt16 actionReadChargeDischarge (HardwareMsgType msgType, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    HardwareChargeDischarge chargeDischarge;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    chargeDischarge.charge = 0;
    chargeDischarge.discharge = 0;
    switch (msgType)
    {
        case HARDWARE_READ_RIO_BATT_LIFETIME_CHARGE_DISCHARGE:
        {
            success = readRioBattLifetimeChargeDischarge (&chargeDischarge.charge, &chargeDischarge.discharge);
            ((HardwareReadRioBattLifetimeChargeDischargeCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadRioBattLifetimeChargeDischargeCnf *) pSendMsgBody)->success);
            memcpy ((&((HardwareReadRioBattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge), &chargeDischarge, sizeof (((HardwareReadRioBattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge));
            sendMsgBodyLength += sizeof (((HardwareReadRioBattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge);            
        }
        break;
        case HARDWARE_READ_O1_BATT_LIFETIME_CHARGE_DISCHARGE:
        {
            success = readO1BattLifetimeChargeDischarge (&chargeDischarge.charge, &chargeDischarge.discharge);
            ((HardwareReadO1BattLifetimeChargeDischargeCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO1BattLifetimeChargeDischargeCnf *) pSendMsgBody)->success);
            memcpy ((&((HardwareReadO1BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge), &chargeDischarge, sizeof (((HardwareReadO1BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge));
            sendMsgBodyLength += sizeof (((HardwareReadO1BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge);            
        }
        break;
        case HARDWARE_READ_O2_BATT_LIFETIME_CHARGE_DISCHARGE:
        {
            success = readO2BattLifetimeChargeDischarge (&chargeDischarge.charge, &chargeDischarge.discharge);
            ((HardwareReadO2BattLifetimeChargeDischargeCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO2BattLifetimeChargeDischargeCnf *) pSendMsgBody)->success);
            memcpy ((&((HardwareReadO2BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge), &chargeDischarge, sizeof (((HardwareReadO2BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge));
            sendMsgBodyLength += sizeof (((HardwareReadO2BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge);            
        }
        break;
        case HARDWARE_READ_O3_BATT_LIFETIME_CHARGE_DISCHARGE:
        {
            success = readO3BattLifetimeChargeDischarge (&chargeDischarge.charge, &chargeDischarge.discharge);
            ((HardwareReadO3BattLifetimeChargeDischargeCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareReadO3BattLifetimeChargeDischargeCnf *) pSendMsgBody)->success);
            memcpy ((&((HardwareReadO3BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge), &chargeDischarge, sizeof (((HardwareReadO3BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge));
            sendMsgBodyLength += sizeof (((HardwareReadO3BattLifetimeChargeDischargeCnf *) pSendMsgBody)->chargeDischarge);            
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that does a battery swap.
 * 
 * msgType          the msgType, extracted from the
 *                  received mesage.
 * pReceivedMsgBody a pointer to the data to be
 *                  passed to the battery swap
 *                  function.
 * pSendMsgBody     pointer to the relevant message
 *                  type to fill in with a response,
 *                  which will be overlaid over the
 *                  body of the response message.
 * 
 * @return          the length of the message body
 *                  to send back.
 */
static UInt16 actionSwapBattery (HardwareMsgType msgType, UInt8 *pReceivedMsgBody, UInt8 *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    HardwareBatterySwapData *pBatterySwapData = PNULL;
    
    ASSERT_PARAM (pReceivedMsgBody != PNULL, (unsigned long) pReceivedMsgBody);
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    switch (msgType)
    {
        case HARDWARE_SWAP_RIO_BATTERY:
        {
            pBatterySwapData = &(((HardwareSwapRioBatteryReq *) pReceivedMsgBody)->batterySwapData);
            success = swapRioBattery (pBatterySwapData->systemTime, pBatterySwapData->remainingCapacity);
            ((HardwareSwapRioBatteryCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareSwapRioBatteryCnf *) pSendMsgBody)->success);
        }
        break;
        case HARDWARE_SWAP_O1_BATTERY:
        {
            pBatterySwapData = &(((HardwareSwapO1BatteryReq *) pReceivedMsgBody)->batterySwapData);
            success = swapO1Battery (pBatterySwapData->systemTime, pBatterySwapData->remainingCapacity);
            ((HardwareSwapO1BatteryCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareSwapO1BatteryCnf *) pSendMsgBody)->success);
        }
        break;
        case HARDWARE_SWAP_O2_BATTERY:
        {
            pBatterySwapData = &(((HardwareSwapO2BatteryReq *) pReceivedMsgBody)->batterySwapData);
            success = swapO2Battery (pBatterySwapData->systemTime, pBatterySwapData->remainingCapacity);
            ((HardwareSwapO2BatteryCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareSwapO2BatteryCnf *) pSendMsgBody)->success);
        }
        break;
        case HARDWARE_SWAP_O3_BATTERY:
        {
            pBatterySwapData = &(((HardwareSwapO3BatteryReq *) pReceivedMsgBody)->batterySwapData);
            success = swapO3Battery (pBatterySwapData->systemTime, pBatterySwapData->remainingCapacity);
            ((HardwareSwapO3BatteryCnf *) pSendMsgBody)->success = success;
            sendMsgBodyLength += sizeof (((HardwareSwapO3BatteryCnf *) pSendMsgBody)->success);
        }
        break;
        default:
        {
            ASSERT_ALWAYS_PARAM (msgType);
        }
        break;
    }
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that sets the input to the
 * analogue mux.
 * 
 * input            the input to use.
 * pSendMsgBody     pointer to the relevant message
 *                  type to fill in with a response,
 *                  which will be overlaid over the
 *                  body of the response message.
 * 
 * @return          the length of the message body
 *                  to send back.
 */
static UInt16 actionSetAnalogueMuxInput (UInt8 input, HardwareSetAnalogueMuxInputCnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);

    success = setAnalogueMuxInput (input);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    
    return sendMsgBodyLength;
}

/*
 * Handle a message that sends a string to the 
 * Orangutan, AKA Hindbrain.
 * 
 * pString          the null terminated string to send.
 * waitForResponse  true if a response is required.
 * pSendMsgBody     pointer to the relevant message
 *                  type to fill in with a response,
 *                  which will be overlaid over the
 *                  body of the response message.
 * 
 * @return          the length of the message body
 *                  to send back.
 */
static UInt16 actionSendOString (Char *pInputString, Bool waitForResponse, HardwareSendOStringCnf *pSendMsgBody)
{
    Bool success;
    UInt16 sendMsgBodyLength = 0;
    Char *pResponseString = PNULL;
    UInt32 *pResponseStringLength = PNULL;
    Char displayBuffer[MAX_O_STRING_LENGTH];
    
    ASSERT_PARAM (pSendMsgBody != PNULL, (unsigned long) pSendMsgBody);
    ASSERT_PARAM (pInputString != PNULL, (unsigned long) pInputString);

    printDebug ("HW Server: received '%s', sending to Orangutan.\n", removeCtrlCharacters (pInputString, &(displayBuffer[0])));
    pResponseStringLength = &(pSendMsgBody->string.stringLength); 
    *pResponseStringLength = 0;
    
    if (waitForResponse)
    {
        pResponseString = &(pSendMsgBody->string.string[0]);
        *pResponseStringLength = sizeof (pSendMsgBody->string.string);
    }
    
    success = sendStringToOrangutan (pInputString, pResponseString, pResponseStringLength);
    pSendMsgBody->success = success;
    sendMsgBodyLength += sizeof (pSendMsgBody->success);
    sendMsgBodyLength += *pResponseStringLength + sizeof (*pResponseStringLength); /* Assumes packing of 1 */
    if (success)
    {
        if (pResponseString != PNULL)
        {
            printDebug ("HW Server: received '%s', back from Orangutan.\n", removeCtrlCharacters (pResponseString, &(displayBuffer[0])));
        }
        else
        {
            printDebug ("HW Server: send successful, not waiting for a response.\n");
        }
    }
    else
    {
        printDebug ("HW Server: send failed.\n");        
    }
    
    return sendMsgBodyLength;
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
 *                  exitting in which case SERVER_EXIT_NORMALLY.
 */
static ServerReturnCode doAction (HardwareMsgType receivedMsgType, UInt8 * pReceivedMsgBody, Msg *pSendMsg)
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
        case HARDWARE_SERVER_START:
        {
            Bool batteriesOnly = ((HardwareServerStartReq *) pReceivedMsgBody)->batteriesOnly;
            pSendMsg->msgLength += actionHardwareServerStart (batteriesOnly, (HardwareServerStartCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_SERVER_STOP:
        {
            pSendMsg->msgLength += actionHardwareServerStop ((HardwareServerStopCnf *) &(pSendMsg->msgBody[0]));
            returnCode = SERVER_EXIT_NORMALLY;
        }
        break;
        case HARDWARE_READ_CHARGER_STATE_PINS:
        {
            pSendMsg->msgLength += actionReadChargerStatePins ((HardwareReadChargerStatePinsCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_READ_CHARGER_STATE:
        {
            pSendMsg->msgLength += actionReadChargerState ((HardwareReadChargerStateCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_READ_MAINS_12V:
        case HARDWARE_READ_O_PWR:
        case HARDWARE_READ_O_RST:
        case HARDWARE_READ_RIO_PWR_12V:
        case HARDWARE_READ_RIO_PWR_BATT:
        case HARDWARE_READ_O_PWR_12V:
        case HARDWARE_READ_O_PWR_BATT:
        case HARDWARE_READ_RIO_BATTERY_CHARGER:
        case HARDWARE_READ_O1_BATTERY_CHARGER:
        case HARDWARE_READ_O2_BATTERY_CHARGER:
        case HARDWARE_READ_O3_BATTERY_CHARGER:
        case HARDWARE_READ_ON_PCB_RELAYS_ENABLED:
        case HARDWARE_READ_EXTERNAL_RELAYS_ENABLED:
        {
            pSendMsg->msgLength += actionReadBool (receivedMsgType, &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_TOGGLE_O_PWR:
        case HARDWARE_TOGGLE_O_RST:
        case HARDWARE_TOGGLE_PI_RST:
        case HARDWARE_SET_RIO_PWR_12V_ON:
        case HARDWARE_SET_RIO_PWR_12V_OFF:
        case HARDWARE_SET_RIO_PWR_BATT_ON:
        case HARDWARE_SET_RIO_PWR_BATT_OFF:
        case HARDWARE_SET_O_PWR_12V_ON:
        case HARDWARE_SET_O_PWR_12V_OFF:
        case HARDWARE_SET_O_PWR_BATT_ON:
        case HARDWARE_SET_O_PWR_BATT_OFF:
        case HARDWARE_SET_RIO_BATTERY_CHARGER_ON:
        case HARDWARE_SET_RIO_BATTERY_CHARGER_OFF:
        case HARDWARE_SET_O1_BATTERY_CHARGER_ON:
        case HARDWARE_SET_O1_BATTERY_CHARGER_OFF:
        case HARDWARE_SET_O2_BATTERY_CHARGER_ON:
        case HARDWARE_SET_O2_BATTERY_CHARGER_OFF:
        case HARDWARE_SET_O3_BATTERY_CHARGER_ON:
        case HARDWARE_SET_O3_BATTERY_CHARGER_OFF:
        case HARDWARE_SET_ALL_BATTERY_CHARGERS_ON:
        case HARDWARE_SET_ALL_BATTERY_CHARGERS_OFF:
        case HARDWARE_DISABLE_ON_PCB_RELAYS:
        case HARDWARE_ENABLE_ON_PCB_RELAYS:
        case HARDWARE_DISABLE_EXTERNAL_RELAYS:
        case HARDWARE_ENABLE_EXTERNAL_RELAYS:
        case HARDWARE_PERFORM_CAL_ALL_BATTERY_MONITORS:
        case HARDWARE_PERFORM_CAL_RIO_BATTERY_MONITOR:
        case HARDWARE_PERFORM_CAL_O1_BATTERY_MONITOR:
        case HARDWARE_PERFORM_CAL_O2_BATTERY_MONITOR:
        case HARDWARE_PERFORM_CAL_O3_BATTERY_MONITOR:
        {
            pSendMsg->msgLength += actionSetBool (receivedMsgType, &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_READ_GENERAL_PURPOSE_IOS:
        {
            pSendMsg->msgLength += actionReadGeneralPurposeIOs ((HardwareReadGeneralPurposeIOsCnf *) &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_READ_RIO_BATT_CURRENT:
        case HARDWARE_READ_O1_BATT_CURRENT:
        case HARDWARE_READ_O2_BATT_CURRENT:
        case HARDWARE_READ_O3_BATT_CURRENT:
        {
            pSendMsg->msgLength += actionReadCurrent (receivedMsgType, &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_READ_RIO_BATT_VOLTAGE:
        case HARDWARE_READ_O1_BATT_VOLTAGE:
        case HARDWARE_READ_O2_BATT_VOLTAGE:
        case HARDWARE_READ_O3_BATT_VOLTAGE:
        case HARDWARE_READ_ANALOGUE_MUX:
        {
            pSendMsg->msgLength += actionReadVoltage (receivedMsgType, &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_READ_RIO_REMAINING_CAPACITY:
        case HARDWARE_READ_O1_REMAINING_CAPACITY:
        case HARDWARE_READ_O2_REMAINING_CAPACITY:
        case HARDWARE_READ_O3_REMAINING_CAPACITY:
        {
            pSendMsg->msgLength += actionReadRemainingCapacity (receivedMsgType, &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_READ_RIO_BATT_LIFETIME_CHARGE_DISCHARGE:
        case HARDWARE_READ_O1_BATT_LIFETIME_CHARGE_DISCHARGE:
        case HARDWARE_READ_O2_BATT_LIFETIME_CHARGE_DISCHARGE:
        case HARDWARE_READ_O3_BATT_LIFETIME_CHARGE_DISCHARGE:
        {
            pSendMsg->msgLength += actionReadChargeDischarge (receivedMsgType, &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_SWAP_RIO_BATTERY:
        case HARDWARE_SWAP_O1_BATTERY:
        case HARDWARE_SWAP_O2_BATTERY:
        case HARDWARE_SWAP_O3_BATTERY:
        {
            pSendMsg->msgLength += actionSwapBattery (receivedMsgType, pReceivedMsgBody, &(pSendMsg->msgBody[0]));
        }
        break;
        case HARDWARE_SET_ANALOGUE_MUX_INPUT:
        {
            UInt8 input = ((HardwareSetAnalogueMuxInputReq *) pReceivedMsgBody)->input;
            pSendMsg->msgLength += actionSetAnalogueMuxInput (input, (HardwareSetAnalogueMuxInputCnf *) &(pSendMsg->msgBody[0]));            
        }
        break;
        case HARDWARE_SEND_O_STRING:
        {
            Char *pString = &(((HardwareSendOStringReq *) pReceivedMsgBody)->string.string[0]);
            Bool waitForResponse = ((HardwareSendOStringReq *) pReceivedMsgBody)->string.waitForResponse;
            pSendMsg->msgLength += actionSendOString (pString, waitForResponse, (HardwareSendOStringCnf *) &(pSendMsg->msgBody[0]));                        
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
    ASSERT_PARAM (pReceivedMsg->msgType < MAX_NUM_HARDWARE_MSGS, pReceivedMsg->msgType);
    
    printDebug ("HW Server received message %s, length %d.\n", pgHardwareMessageNames[pReceivedMsg->msgType], pReceivedMsg->msgLength);
    printHexDump (pReceivedMsg, pReceivedMsg->msgLength + 1);
    /* Do the thang */
    returnCode = doAction ((HardwareMsgType) pReceivedMsg->msgType, pReceivedMsg->msgBody, pSendMsg);
    printDebug ("HW Server responding with message %s, length %d.\n", pgHardwareMessageNames[pSendMsg->msgType], pSendMsg->msgLength);
        
    return returnCode;
}
