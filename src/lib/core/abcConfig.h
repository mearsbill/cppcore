/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Your Name Here   YourEmailAddressHere
// abcTemplate.h   Please use this templage .h file.   
//
#ifndef __ABC_CONFIG_H__
#define __ABC_CONFIG_H__

// trace abd debug printing.   Note... you can also
// put this block of defines into key c files for selective enabling
//
#define _USE_OBJECT_NAMES_
//#define _TRACE_METHOD_ENTRIES_
//#define _TRACE_METHOD_EXITS_
//#define _TRACE_CONSTRUCTION_
//#define _TRACE_DESTRUCTION_
#define _TRACE_NONIMPL_

// DEBUG PRINTING
#define _PRINT_DEBUG_ERROR_
#define _PRINT_DEBUG_A_

#define _ABORT_ON_FATAL_ERROR_	FALSE	// True or false
#define _PRINT_FATAL_ERROR_	    TRUE	// True or false
#define _PRINT_WARNING_		    TRUE	// True or false
	    
// memory allocation tracking
//#define ABC_MEM_PROD_TRACKING
#define ABC_MEM_DEBUG_TRACKING

// List configuration
#define _LIST_RIGOROUS_TESTING_
#define _LIST_ENFORCE_LOCKING_	// enforce readRegister/writeLock operation
#define _LIST_MAX_SLICE_COUNT_ 10000	//not the same as Hash bucked count

#endif //__ABC_CONFIG_H__
// EOF abcConfig.h

