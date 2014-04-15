/*
 *  Macros the help with message construction for the state machine server
 */

/* Macro for empty message member to keep the compiler happy */
#define STATE_MACHINE_EMPTY UInt8 nothing

/* The basic message macro, never used by itself but included her for completeness */
#define STATE_MACHINE_MSG_DEF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)

/* Extract the message type from the list */
#define STATE_MACHINE_MSG_DEF_TYPE(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) mSGtYPE,

/* Construct a full typedef for a REQ (incoming) message */
#define MAKE_STATE_MACHINE_MSG_STRUCT_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) typedef struct mSGsTRUCT##ReqTag            \
                                                                                                {                                                \
                                                                                                    rEQmSGmEMBER;                                \
                                                                                                } mSGsTRUCT##Req;

/* Construct a full typedef for a CNF (outgoing) message */
#define MAKE_STATE_MACHINE_MSG_STRUCT_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) typedef struct mSGsTRUCT##CnfTag            \
                                                                                                {                                                \
                                                                                                    cNFmSGmEMBER;                                \
                                                                                                } mSGsTRUCT##Cnf;

/* Construct the members of the message unions */
#define MAKE_STATE_MACHINE_UNION_MEMBER_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)        mSGsTRUCT##Req mSGmEMBER##Req;
#define MAKE_STATE_MACHINE_UNION_MEMBER_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)        mSGsTRUCT##Cnf mSGmEMBER##Cnf;

