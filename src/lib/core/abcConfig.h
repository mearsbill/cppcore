/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Your Name Here   YourEmailAddressHere
// abcTemplate.h   Please use this templage .h file.   
//
#ifndef __ABC_CONFIG_H__
#define __ABC_CONFIG_H__

// memory allocation tracking
//#define ABC_MEM_PROD_TRACKING
#define ABC_MEM_STRICT_CHECKING
#define ABC_MEM_DEBUG_TRACKING

// List configuration
#define _LIST_RIGOROUS_TESTING_
#define _LIST_ENFORCE_LOCKING_	// enforce readRegister/writeLock operation
#define _LIST_MAX_SLICE_COUNT_ 10000	//not the same as Hash bucked count

// thread stuff
//#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_ERRORCHECK_NP // Linux
#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_ERRORCHECK // MAC
#define ABC_BASE_PRIORITY 10

//
// object config (name and memToolUsage)
// typedef unsigned char CONFIG_t; defined in abcTypes.h
#define OBJ_CONFIG_UseMemTool 0x01
#define OBJ_CONFIG_UseObjName 0x02

// for debug and most situations
#define DEFAULT_OBJ_CONFIG (  OBJ_CONFIG_UseMemTool   | OBJ_CONFIG_UseObjName)
#define MEMTOOL_OBJ_CONFIG (/*OBJ_CONFIG_UseMemTool | */OBJ_CONFIG_UseObjName)

#define USE_OBJ_NAME(configBits) (configBits & OBJ_CONFIG_UseObjName)
#define USE_MEM_TOOL(configBits) (configBits & OBJ_CONFIG_UseMemTool)


#endif //__ABC_CONFIG_H__
// EOF abcConfig.h

