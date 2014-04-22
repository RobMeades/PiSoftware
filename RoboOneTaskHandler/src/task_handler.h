/*
 * Task Handler functions used by the server.
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * FUNCTION PROTOTYPES
 */
void initTaskHandler (void);
void clearTaskList (void);
Bool handleTaskReq (RoboOneTaskReq *pTaskReq);
void tickTaskHandler (void);