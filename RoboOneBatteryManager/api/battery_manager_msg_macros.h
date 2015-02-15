/*
 * Macros the help with message construction for the RoboOne battery manager server
 * These are different from the other state machines in that the battery manager
 * receives INDication and sends RSPonses for most things and so has separate macros
 * for REQ/CNF (IR versus RC). 
 */

/* Macro for empty message member to keep the compiler happy */
#define BATTERY_MANAGER_EMPTY UInt8 nothing

/* The basic message macro, never used by itself but included her for completeness */
#define BATTERY_MANAGER_MSG_DEF(mSGtYPE, mSGsTRUCT, mSGmEMBER, iNDmSGmEMBER, rSPmSGmEMBER)

/* Extract the message type from the list */
#define BATTERY_MANAGER_MSG_DEF_TYPE(mSGtYPE, mSGsTRUCT, mSGmEMBER, iNDrEQmSGmEMBER, rSPcNFmSGmEMBER) mSGtYPE,

/* Make a message name from the list */
#define BATTERY_MANAGER_MSG_DEF_NAME(mSGtYPE, mSGsTRUCT, mSGmEMBER, iNDrEQmSGmEMBER, rSPcNFmSGmEMBER) #mSGtYPE,

/* Construct a full typedef for an IND (incoming) message */
#define MAKE_BATTERY_MANAGER_MSG_STRUCT_IND(mSGtYPE, mSGsTRUCT, mSGmEMBER, iNDmSGmEMBER, rSPmSGmEMBER) typedef struct mSGsTRUCT##IndTag  \
                                                                                                       {                                 \
                                                                                                           iNDmSGmEMBER;                 \
                                                                                                       } mSGsTRUCT##Ind;
/* Construct a full typedef for an RSP (outgoing) message with the standard success field at the start */
#define MAKE_BATTERY_MANAGER_MSG_STRUCT_RSP(mSGtYPE, mSGsTRUCT, mSGmEMBER, iNDmSGmEMBER, rSPmSGmEMBER) typedef struct mSGsTRUCT##RspTag   \
                                                                                                       {                                  \
                                                                                                           rSPmSGmEMBER;                  \
                                                                                                       } mSGsTRUCT##Rsp;
/* Construct a full typedef for an REQ (incoming) message */
#define MAKE_BATTERY_MANAGER_MSG_STRUCT_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) typedef struct mSGsTRUCT##ReqTag  \
                                                                                                       {                                 \
                                                                                                           rEQmSGmEMBER;                 \
                                                                                                       } mSGsTRUCT##Req;

/* Construct a full typedef for an CNF (outgoing) message with the standard success field at the start */
#define MAKE_BATTERY_MANAGER_MSG_STRUCT_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) typedef struct mSGsTRUCT##cNFTag   \
                                                                                                       {                                  \
                                                                                                           cNFmSGmEMBER;                  \
                                                                                                       } mSGsTRUCT##Cnf;
/* Construct the members of the message unions */
#define MAKE_BATTERY_MANAGER_UNION_MEMBER_IND(mSGtYPE, mSGsTRUCT, mSGmEMBER, iNDmSGmEMBER, rSPmSGmEMBER)        mSGsTRUCT##Ind mSGmEMBER##Ind;
#define MAKE_BATTERY_MANAGER_UNION_MEMBER_RSP(mSGtYPE, mSGsTRUCT, mSGmEMBER, iNDmSGmEMBER, rSPmSGmEMBER)        mSGsTRUCT##Rsp mSGmEMBER##Rsp;
#define MAKE_BATTERY_MANAGER_UNION_MEMBER_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)        mSGsTRUCT##Req mSGmEMBER##Req;
#define MAKE_BATTERY_MANAGER_UNION_MEMBER_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)        mSGsTRUCT##Cnf mSGmEMBER##Cnf;

