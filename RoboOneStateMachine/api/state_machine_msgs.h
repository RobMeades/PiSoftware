/*
 *  Messages that go from/to the state machine server.
 */

/* This file is included in the state_machine_msg_auto header and
 * defines the message structures for the messages sent to
 * and from the server.  The items in the list are:
 * 
 * - the message type enum,
 * - the message typedef struct (without a Cnf or Req on the end),
 * - the message variable name for use in the message union
 *   (again, without a Cnf or Req on the end),
 * - the message member that is needed in the Req message structure
 *   beyond the mandatory msgHeader (which is added automatagically),
 * - the message member that is needed in the Cnf message structure.
 */

/*
 * Definitions
 */
STATE_MACHINE_MSG_DEF (STATE_MACHINE_SERVER_START, StateMachineServerStart, stateMachineServerStart, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_SERVER_STOP, StateMachineServerStop, stateMachineServerStop, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_INIT, StateMachineEventInit, stateMachineEventInit, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_INIT_FAILURE, StateMachineEventInitFailure, stateMachineEventInitFaiure, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_TIMER_EXPIRY, StateMachineEventTimerExpiry, stateMachineEventTimerExpiry, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_TASKS_AVAILABLE, StateMachineEventTasksAvailable, stateMachineEventTasksAvailable, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_NO_TASKS_AVAILABLE, StateMachineEventNoTasksAvailable, stateMachineEventNoTasksAvailable, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_MAINS_POWER_AVAILABLE, StateMachineEventMainsPowerAvailable, stateMachineEventMainsPowerAvailable, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_INSUFFICIENT_POWER, StateMachineEventInsufficientPower, stateMachineEventInsufficientPower, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_FULLY_CHARGED, StateMachineEventFullyCharged, stateMachineEventFullyCharged, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)
STATE_MACHINE_MSG_DEF (STATE_MACHINE_EVENT_SHUTDOWN, StateMachineEventShutdown, stateMachineEventShutdown, STATE_MACHINE_EMPTY, STATE_MACHINE_EMPTY)