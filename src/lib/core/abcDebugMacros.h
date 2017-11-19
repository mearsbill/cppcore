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

_TRACE_NONIMPL_

_PRINT_DEBUG_ERROR_
_PRINT_DEBUG_A_
_PRINT_FATAL_ERROR_
_PRINT_WARNING_

_ABORT_ON_FATAL_ERROR_

*/

// control param enum.   pick which type of printing we're talking about for the globalPrintEnabled function.
typedef enum debugPrint_e	// param to globalPrintEnabled()
{
	DBGP_UNKNOWN=0,
	DBGP_E,	// debugPrinting Errors		Typically on for production
	DBGP_W,	// debugPrinting Warnings	Typically on for production
	DBGP_P,	// debugPrinting Printing	Typically on for production
	DBGP_D,	// debugPrinting Debugging	Typically OFF for production
} debugPrint_e;

// control enum for globalAbortEnabled 
typedef enum  debugAbort_e		// param to globalAbortEnabled()
{
	DBGA_UNKNOWN = 0,
	DBGA_E,					// abort on error.
} debugAbort_e;



// TRACE 
#define TRACE_GENERIC(obj,def) {fprintf(stderr,"%s::%s => %s at %s:%d\n",obj,__FUNCTION__,def,__FILE__,__LINE__);}
#define TRACE_NONIMPL(abc) TRACE_GENERIC(abc,"Not Implemented")

//
// Error macros (typically on in production)
//
#define CHECK_ERROR(cond,reason,failReturn) if (cond) { ERROR(reason); return failReturn;}

#define ERROR(reason) {setReason(reason); if (globalPrintEnabled(DBGP_E)) fprintf(stderr,"Error: Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);if (globalAbortEnabled(DBGA_E)){ fprintf(stderr,"Aborting...\n"); abort();}}
#define ERROR_G(reason) { globalSetReason(reason); if (globalPrintEnabled(DBGP_E)) fprintf(stderr,"Error: Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);if (globalAbortEnabled(DBGA_E)){ fprintf(stderr,"Aborting...\n"); abort();}}
#define ERROR_LTD(limit,reason) {setReason(reason);  static int pCount=0; pCount++; if ((pCount <= limit) && globalPrintEnabled(DBGP_E)) fprintf(stderr,"Error(%d): Reason=%s in method %s at %s:%d\n", pCount,STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__); \
			if ((pCount >= limit) && globalAbortEnabled(DBGA_E)){ fprintf(stderr,"ErrorLimit Reached!!! Aborting... Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__); abort();}}

// Warning Macros (typically on in production )
#define WARNING(reason) { setReason(reason); if (globalPrintEnabled(DBGP_W))fprintf(stderr,"WARNING! Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);  }
#define WARNING_G(reason) { globalSetReason(reason); if (globalPrintEnabled(DBGP_W))fprintf(stderr,"WARNING! Reason=%s in method %s at %s:%d\n", STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__);  }
#define WARNING_LTD(unfilteredCount,filterRate,reason) { setReason(reason); static int pCount=0; pCount++; if ((pCount <= unfilteredCount) || ( (filterRate>0 && (!(pCount % filterRate)))))\
		if (globalPrintEnabled(DBGP_W)) fprintf(stderr,"WARNING(%d)! Reason=%s in method %s at %s:%d\n", pCount,STRINGIFY(reason),__FUNCTION__,__FILE__,__LINE__); }

// Printing Macros (typically for production .... typically on in production)
#define PRINT(x...)		{if (globalPrintEnabled(DBGP_P)) fprintf(stderr,x); }
#define PRINT_LTD(unfilteredCount,filterRate,x...) { static int pCount=0; pCount++; if ((pCount <= unfilteredCount) || ( (filterRate>0 && (!(pCount % filterRate))))) if (globalPrintEnabled(DBGP_P)) fprintf(stderr,x); }

// Debugging Macros (typically for debugging... off in production)
#define DEBUG(x...)		if (globalPrintEnabled(DBGP_D)) { char tmpLine[256]; snprintf(tmpLine,256,x); fprintf(stderr,"%s in method %s at %s:%d\n",tmpLine,__FUNCTION__,__FILE__,__LINE__); }
#define DEBUG_LTD(unfilteredCount,filterRate,x...) { static int pCount=0; pCount++; if ((pCount <= unfilteredCount) || ( (filterRate>0 && (!(pCount % filterRate))))) DEBUG(x);

#define LOG_TEST(testName,result) fprintf(stderr,"Test %s: %s\n",testName,passFailAsStr(result))
#define TTY_CHK_TEST(testNoStr,inStr,PassChar,failureSum) {if (strlen(inStr)==1) inStr[0]=PassChar; int testPassedTmp = (toupper(inStr[0]) == PassChar ); LOG_TEST(testNoStr,testPassedTmp); failureSum += !testPassedTmp;}
#define CHECK_TEST(testNoStr,passCondition,failureSum) {int testPassedTmp = (passCondition); LOG_TEST(testNoStr,testPassedTmp); failureSum += !testPassedTmp;}

#endif //__ABC_DEBUG_MACROS_H__
 // EOF abcDebugMacros.h

