/*
 *  Macros the help with message construction for the task handler server
 */

/* Macro for empty message member to keep the compiler happy */
#define TASK_HANDLER_EMPTY UInt8 nothing

/* The basic message macro, never used by itself but included her for completeness */
#define TASK_HANDLER_MSG_DEF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER)

/* Extract the message type from the list */
#define TASK_HANDLER_MSG_DEF_TYPE(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER) mSGtYPE,

/* Make a message name from the list */
#define TASK_HANDLER_MSG_DEF_NAME(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER) #mSGtYPE,

/* Construct a full typedef for a REQ (incoming) message */
#define MAKE_TASK_HANDLER_MSG_STRUCT_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER) typedef struct mSGsTRUCT##ReqTag \
                                                                                                {                                    \
                                                                                                    rEQmSGmEMBER;                    \
                                                                                                } mSGsTRUCT##Req;

/* Construct a full typedef for a CNF (outgoing, in response to REQ) message */
#define MAKE_TASK_HANDLER_MSG_STRUCT_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER) typedef struct mSGsTRUCT##CnfTag \
                                                                                                {                                    \
                                                                                                    Bool success;                    \
                                                                                                } mSGsTRUCT##Cnf;

/* Construct a full typedef for an IND (outgoing, as a progress indicator) message */
#define MAKE_TASK_HANDLER_MSG_STRUCT_IND(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER) typedef struct mSGsTRUCT##IndTag \
                                                                                                {                                    \
                                                                                                    iNDmSGmEMBER;                    \
                                                                                                } mSGsTRUCT##Ind;

/* Construct the members of the message unions */
#define MAKE_TASK_HANDLER_UNION_MEMBER_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER)        mSGsTRUCT##Req mSGmEMBER##Req;
#define MAKE_TASK_HANDLER_UNION_MEMBER_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER)        mSGsTRUCT##Cnf mSGmEMBER##Cnf;
#define MAKE_TASK_HANDLER_UNION_MEMBER_IND(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, iNDmSGmEMBER)        mSGsTRUCT##Ind mSGmEMBER##Ind;