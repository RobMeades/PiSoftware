/*
 *  Macros the help with message construction for the RoboOne harware server
 */

/* Macro for empty message member to keep the compiler happy */
#define HARDWARE_EMPTY UInt8 nothing

/* The basic message macro, never used by itself but included her for completeness */
#define HARDWARE_MSG_DEF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)

/* Extract the message type from the list */
#define HARDWARE_MSG_DEF_TYPE(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) mSGtYPE,

/* Make a message name from the list */
#define HARDWARE_MSG_DEF_NAME(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) #mSGtYPE,

/* Construct a full typedef for a REQ (incoming) message */
#define MAKE_HARDWARE_MSG_STRUCT_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) typedef struct mSGsTRUCT##ReqTag  \
                                                                                                {                                 \
                                                                                                    rEQmSGmEMBER;                 \
                                                                                                } mSGsTRUCT##Req;

/* Construct a full typedef for a CNF (outgoing) message with the standard success field at the start */
#define MAKE_HARDWARE_MSG_STRUCT_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER) typedef struct mSGsTRUCT##CnfTag   \
                                                                                                {                                  \
                                                                                                    Bool success;                  \
                                                                                                    cNFmSGmEMBER;                  \
                                                                                                } mSGsTRUCT##Cnf;

/* Construct the members of the message unions */
#define MAKE_HARDWARE_UNION_MEMBER_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)        mSGsTRUCT##Req mSGmEMBER##Req;
#define MAKE_HARDWARE_UNION_MEMBER_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGmEMBER, cNFmSGmEMBER)        mSGsTRUCT##Cnf mSGmEMBER##Cnf;

