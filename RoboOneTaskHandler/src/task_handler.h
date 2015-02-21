/*
 * Task Handler functions used by the server.
 */

/*
 * MANIFEST CONSTANTS
 */

/*
 * FUNCTION PROTOTYPES
 */
Bool initTaskList (void);
Bool clearTaskList (void);
Bool handleNewTaskReq (RoboOneTaskReq *pTaskReq);
Bool tickTaskHandler (void);