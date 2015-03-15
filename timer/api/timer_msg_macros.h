/*
 * Macros the help with message construction for the timer server.
 */

/* Macro for empty message member to keep the compiler happy */
#define TIMER_EMPTY UInt8 nothing

/* The basic message macro, never used by itself but included her for completeness */
#define TIMER_MSG_DEF(mSGtYPE, mSGsTRUCT, mSGmEMBER, mSGcONENTS)

/* Extract the message type from the list */
#define TIMER_MSG_DEF_TYPE(mSGtYPE, mSGsTRUCT, mSGmEMBER, mSGcONENTS) mSGtYPE,

/* Make a message name from the list */
#define TIMER_MSG_DEF_NAME(mSGtYPE, mSGsTRUCT, mSGmEMBER, mSGcONENTS) #mSGtYPE,

/* Construct a full typedef for a message */
#define MAKE_TIMER_MSG_STRUCT(mSGtYPE, mSGsTRUCT, mSGmEMBER, mSGcONENTS) typedef struct mSGsTRUCT##Tag  \
                                                                         {                              \
                                                                             mSGcONENTS;                \
                                                                         } mSGsTRUCT;
/* Construct the message union */
#define MAKE_TIMER_UNION_MEMBER(mSGtYPE, mSGsTRUCT, mSGmEMBER, mSGcONENTS) mSGsTRUCT mSGmEMBER;
