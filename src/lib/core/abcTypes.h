/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author Bill Mears   bill@imears.com  2017-1201
 // abcTypes.h   
 //
 #ifndef __ABC_TYPES_H__
 #define __ABC_TYPES_H__

// This is where all the local types are defined.

typedef int64_t abcTime1b_t;			// time in nanoseconds
typedef int64_t abcTime1m_t;			// time in microseconds.
typedef int64_t abcTime1k_t;			// time in milliseconds.
typedef int64_t abcTime1s_t;			// time in seconds.

#define SEC_AT_1M (1000000)			// one second = 1 million ticks for abcTime1m_t 

// constants
#define TRUE (1)
#define FALSE (0)
#define MAX_64B_INT (0x7FFFFFFFFFFFFFFFLL)
#define MAX_32B_INT (0x7FFFFFFF)

// configBits stuff
typedef unsigned char CONFIG_t;

#endif //__ABC_TYPES_H__
/////////////////////////
// EOF abcTypes.h
