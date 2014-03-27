/*
 *  Macros the help with message construction for the OneWire server
 */

/* The basic message macro, never used by itself but included her for completeness */
#define MSG_DEF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGsTRUCT, rEQmSGmEMBER, cNFmSGsTRUCT, cNFmSGmEMBER)

/* Extract the message type from the list */
#define MSG_DEF_TYPE(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGsTRUCT, rEQmSGmEMBER, cNFmSGsTRUCT, cNFmSGmEMBER) mSGtYPE,

/* Construct a full typedef for a REQ (incoming) message, putting the mandatory OneWireReqMsgHeader at the start */
#define MAKE_ONE_WIRE_MSG_STRUCT_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGsTRUCT, rEQmSGmEMBER, cNFmSGsTRUCT, cNFmSGmEMBER) typedef struct mSGsTRUCT##ReqTag   \
                                                                                                                            {                                  \
                                                                                                                                OneWireReqMsgHeader msgHeader; \
                                                                                                                                rEQmSGsTRUCT rEQmSGmEMBER;     \
                                                                                                                            } mSGsTRUCT##Req;

/* Construct a full typedef for a CNF (outgoing) message, putting the mandatory OneWireResult at the start */
#define MAKE_ONE_WIRE_MSG_STRUCT_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGsTRUCT, rEQmSGmEMBER, cNFmSGsTRUCT, cNFmSGmEMBER) typedef struct mSGsTRUCT##CnfTag   \
                                                                                                                            {                                  \
                                                                                                                                OneWireResult oneWireResult;   \
                                                                                                                                cNFmSGsTRUCT cNFmSGmEMBER;     \
                                                                                                                            } mSGsTRUCT##Cnf;

/* Construct the members of the message unions */
#define MAKE_UNION_MEMBER_CNF(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGsTRUCT, rEQmSGmEMBER, cNFmSGsTRUCT, cNFmSGmEMBER)        mSGsTRUCT##Cnf mSGmEMBER##Cnf;
#define MAKE_UNION_MEMBER_REQ(mSGtYPE, mSGsTRUCT, mSGmEMBER, rEQmSGsTRUCT, rEQmSGmEMBER, cNFmSGsTRUCT, cNFmSGmEMBER)        mSGsTRUCT##Req mSGmEMBER##Req;


