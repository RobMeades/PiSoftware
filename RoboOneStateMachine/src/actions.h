/*
 * Action functions, used by the state machine.
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * FUNCTION PROTOTYPES
 */

Bool actionEnableExternalRelays (void);
Bool actionDisableExternalRelays (void);
Bool actionEnableOnPCBRelays (void);
Bool actionDisableOnPCBRelays (void);
Bool actionIsMains12VAvailable (void);
Bool actionSwitchOnHindbrain (void);
Bool actionSwitchOffHindbrain (void);
Bool actionSwitchPiRioTo12VMainsPower (void);
Bool actionSwitchPiRioToBatteryPower (void);
Bool actionSwitchHindbrainTo12VMainsPower (void);
Bool actionSwitchHindbrainToBatteryPower (void);
Bool actionStartTimer (void);
Bool actionStopTimer (void);