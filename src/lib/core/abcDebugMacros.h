/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author: Bill Mears   bill@imears.com
 // File:  abcDebugMacros.h
 //
 #ifndef __ABC_DEBUG_MACROS_H__
 #define __ABC_DEBUG_MACROS_H__

#include "abcBase.h"

/* useful and interesting #defines that control behavior in this file

_USE_OBJECT_NAMES_
_TRACE_METHOD_ENTRIES_
_TRACE_METHOD_EXITS_
_TRACE_CONSTRUCTION_
_TRACE_DESTRUCTION_
_TRACE_NONIMPL_

_PRINT_DEBUG_ERROR_
_PRINT_DEBUG_A_
_PRINT_FATAL_ERROR_
_PRINT_WARNING_

_ABORT_ON_FATAL_ERROR_

*/

// object names macros
#ifdef _USE_OBJECT_NAMES_
#define OBJ_NAME	"char *name"
#else
#define OBJ_NAME
#endif

// TRACE GENERIC 
#define TRACE_GENERIC(obj,def) {fprintf(stderr,"%s::%s => %s at %s:%d\n",obj,__FUNCTION__,def,__FILE__,__LINE__);}

// TRACE  METHODS
#ifdef _TRACE_METHOD_ENTRIES_
	#define TRACE_ENTRY(abc) TRACE_GENERIC(abc,"Entry")
#else
	#define TRACE_ENTRY(abc)
#endif
#ifdef _TRACE_METHOD_EXITS_
	#define TRACE_EXIT(abc) TRACE_GENERIC(abc,"Exit")
#else
	#define TRACE_EXIT(abc)
#endif

// TRACE CONSTRUCTION/DESTRUCTION
#ifdef _TRACE_CONSTRUCTION_
	#define TRACE_CONSTRUCT(abc) TRACE_GENERIC(abc,"Construct")
#else
	#define TRACE_CONSTRUCT(abc)
#endif
#ifdef _TRACE_DESTRUCTION_
	#define TRACE_DESTROY(abc) TRACE_GENERIC(abc,"Destroy")
#else
	#define TRACE_DESTROY(abc)
#endif

#ifdef _TRACE_NONIMPL_
	#define TRACE_NONIMPL(abc) TRACE_GENERIC(abc,"Not Implemented")
#else
	#define TRACE_NONIMPL(abc)
#endif

//
// Error macro
//

// PRINT_GENERIC 
#define PRINT_GENERIC(abc,def) {fprintf(stderr,"%s: in %s file: %s:%d...%s\n",abc,__FUNCTION__,__FILE__,__LINE__,def);}
//#define PRINT_GENERIC(xyz,def) {fprintf(stderr,"%s: %s: %s  at %s:%d\n:",__FUNCTION__,xyz,def,__FILE__,__LINE__);}

// PROD_ERROR
#define PROD_ERROR(abc) PRINT_GENERIC("ERROR",abc)

//
#ifdef _PRINT_DEBUG_ERROR_
	#define DEBUG_ERROR(abc) PRINT_GENERIC("ERROR",abc)
#else
	#define DEBUG_ERROR(abc) 
#endif

//DEBUG_A
#ifdef _PRINT_DEBUG_A_
//	#define DEBUG_A(x...) {char msg[256]; snprintf(msg,256,abc,..); fprintf(stderr,msg);}
	#define DEBUG_A(x...) {fprintf(stderr,x); }
#else
	#define DEBUG_A(abc) 
#endif

//DEBUG_B
#ifdef _PRINT_DEBUG_B_
	#define DEBUG_B(x...) {fprintf(stderr,x); }
#else
	#define DEBUG_B(abc) 
#endif


// FATAL_ERROR
#define FATAL_ERROR(reason) { setErrorReason(reason); if (_ABORT_ON_FATAL_ERROR_)  { fprintf(stderr,"Fatal Error... aborting.  Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);abort(); } \
	else {if (_PRINT_FATAL_ERROR_)fprintf(stderr,"Fatal Error... not aborting.  Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);  }}

#define FATAL_ERROR_G(reason) { abcGlobalSetErrorReason(reason); if (_ABORT_ON_FATAL_ERROR_) { fprintf(stderr,"Fatal Error... aborting.  Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);abort(); } \
	else {if (_PRINT_FATAL_ERROR_)fprintf(stderr,"Fatal Error... not aborting.  Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);  }}

// WARNING
#define WARNING(reason) { setErrorReason(reason); if (_PRINT_WARNING_)fprintf(stderr,"WARNING! Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);  }
#define WARNING_G(reason) { abcGlobalSetErrorReason(reason); if (_PRINT_WARNING_)fprintf(stderr,"WARNING! Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);  }

#define PERROR_LTD(pStart,filterRate,reason) { static int pCount=0; pCount++; if ((pCount <= pStart) || ( (filterRate>0 && (!(pCount % filterRate))))) fprintf(stderr,"WARNING(%d)! Reason=%s in method %s at %s:%d\n", pCount,STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__); }

#endif //__ABC_DEBUG_MACROS_H__
 // EOF abcDebugMacros.h

