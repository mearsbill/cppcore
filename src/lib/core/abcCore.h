/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author Bill Mears   bill@imears.com
 // abcCore.h   This is the core .h file.   Everyone should include this.
 //
 // acknowledgement to GNU Free Documentation License 1.2 for CRC32
 //
 // This framework uses just a few global variables.   Currently that list is contrained to:
 // extern abcCore_c *globalCore;
 // extern abcMemMon_c *globalMem;
 // 
 // (1) The above global pointers are initalized along with their objects via the following call:
 //
 // abcResult_e abcInit(int initVal);   // you;ll get a return of ABC_FAIL if something is wrong.  Otherwise,
 //										// you can expect the global object to be inited.  DO THIS FIRST !!
 //
 // (2) Peform a shutdown at the end of the program.  This will give you a report on your memory utilization
 //
 // abcResult_e abcShutdown(int shutdownVal);  // review code to understand the value of the shutdownVal
 //
 // (3) To use the memory tracker, you've got to use the macros:
 //
 // ABC_NEW_CLASS(a,b...)	is the same as new a(b...)
 // ABC_NEW_STRUCT(a)		is a calloc(1,sizeof(a)
 // ABC_MALLOC(mSize)  		is a malloc(size)
 // ABC_CALLOC(cCount, cSize)  is a calloc(cCount, cSize)
 // ABC_DEL_CLASS(objPtr)      is a delete objPtr
 // ABC_DEL_STRUCT(structPtr)  is a free(structPtr);
 // ABC_FREE(freePtr)          is a free(freePtr);
 // 
 //  if you use them everywhere, then you can change the macros to make production not use them if you want.
 //
 //
 // (4) There is an globalConfig_s structure defined below intended for distributing command line args throught the application... use it and expand it!
 //

 #ifndef __ABC_CORE_H__
 #define __ABC_CORE_H__

// Common system includes not needed in abcBase.h put here.
extern "C" 
{
#include <ctype.h>
#include <xlocale.h>
}


// Common includes for lib/core here
#include "abcBase.h"
#include "abcList.h"		// list sits below core.
#include "abcMemory.h"		// our memory system sits below core as well.
#include "abcThreads.h"
#include "abcStrings.h"


// abcCore configuration block
//
// this hold global configuration information intended to be setting fetched from the command line.
//
typedef struct abcGlobalConfig_s
{
	uint8_t		printErrors;
	uint8_t		printWarnings;
	uint8_t		printPrint;
	uint8_t		printDebug;

	uint8_t		abortErrors;

} abcGlobalConfig_s;

// prime number sieve storage for reentrancy
typedef struct abcPrime_s
{
	int32_t		primeRank;
	int32_t		lastMultipleMarked;
	int32_t		lastSeriesValue;
	uint8_t		isNotPrime;
} abcPrime_s;

// global singleton with must have globall stuff
class abcCore_c
{
	private:
		pthread_mutex_t     mutex;

		abcReason_e reason;			// this is for small objects that can't afford 8 bytes for this field

		// private locking... 
		abcResult_e  mutexInit();
		abcResult_e  mutexLock();
		abcResult_e  mutexUnlock();


		// for prime numbers reentrancy... we can expand our prime number list after created.
		abcPrime_s	*sieveArray;
		int32_t		sieveArraySize;
		int32_t		sievePrimeCount;
		int32_t		sieveWorkingPrime;
		abcResult_e	initPrimes(int tableSize=1000);		

		// for crc calculations... the static tables for our 2 popular CRCs
		uint64_t	crc32Table[256];
		uint64_t	crc64Table[256];

		abcGlobalConfig_s	*config;			// our global configuration structure.... holds most command line args.

	public:
		// lifecycle and control
		abcCore_c();
		~abcCore_c();
		abcResult_e	init(int initVal);
		abcResult_e	shutdown(int shutdownVal);

		// depricate this.... or restrict usage to just this object....
		// except its a singleton, so its not of much value
		void		setReason(abcReason_e reasonSet);
		abcReason_e	getReason();

		// signal handling... 
		abcResult_e		initSignalHandling();


		//
		// for prime number generation
		//
		abcResult_e	updatePrimes(int maxValue);		// increase the maximum when needed.
		int			findPrime(int target);			// call here to get a prime number, will auto call updatePrimes
		abcResult_e printPrimes(int low=2, int high=100);	// a debug routine to print out primes in a numberical range

		//
		// standard crc geberators for hashing etc;
		// we support a standard crc32 with seed and crc64 with no seed.
		abcResult_e generateCrc32Table();
		abcResult_e	generateCrc64Table();	// make the crc table
		uint32_t	computeCrc32(uint32_t seed, uint8_t *strPtr,int strLen);
		uint64_t	computeCrc64(uint8_t *strPtr,int strLen);

		abcGlobalConfig_s *getConfig();

}; // end class abcCore_c

//
// ==========   global interfaces =============
//
extern abcCore_c		 *globalCore;
extern abcGlobalConfig_s *globalConfig;
extern abcMemMon_c		 *globalMem;

// global signal handler(s)
void abcCoreHandleSignal(int signal);


// abc api interface.... with init and shutdown
//
abcResult_e abcInit(int initVal);
abcResult_e abcShutdown(int initVal);
//
// global error reporting.... (depricate this !!)
uint8_t globalPrintEnabled(debugPrint_e type);
uint8_t globalAbortEnabled(debugAbort_e type);
void globalSetReason(abcReason_e reason);
void globalResetReason();

// random things of value
double  randomPercentage();
abcTime1m_t getTime1m();

#endif //__ABC_CORE_H__
///////////////////////
// EOF abcCore.h
