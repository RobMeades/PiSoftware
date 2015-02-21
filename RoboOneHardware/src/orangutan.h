/*
 * Orangutan interaction stuff.
 */ 

SInt32 openOrangutan (void);
void closeOrangutan (void);
Bool sendStringToOrangutan (Char *pSendString, Char *pReceiveString, UInt32 *pReceiveStringLength);