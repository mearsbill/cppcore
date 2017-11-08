/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author Bill Mears   bill@imears.com
 // abcCore.h   This is the core .h file.   Everyone should include this.
 //
 // acknowledgement to GNU Free Documentation License 1.2 for CRC32
 //

 // 

 #ifndef __ABC_CORE_H__
 #define __ABC_CORE_H__

// Common system includes here.
extern "C" 
{
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
}


// Common includes for lib/core here
#include "abcBase.h"
#include "abcList.h"		// list sits below core.
#include "abcMemory.h"		// our memory system sits below core as well.

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

		abcReason_e errorReason;			// this is for small objects that can't afford 8 bytes for this field

		// our private locking
		abcResult_e  mutexInit();
		abcResult_e  mutexLock();
		abcResult_e  mutexUnlock();


		// for prime numbers reentrancy
		abcPrime_s	*sieveArray;
		int32_t		sieveArraySize;
		int32_t		sievePrimeCount;
		int32_t		sieveWorkingPrime;
		abcResult_e	initPrimes(int maxValue=1000);		

		// for crc calculations... the static tables for our 2 popular CRCs
		uint64_t	crc32Table[256];
		uint64_t	crc64Table[256];

	public:
		// lifecycle and control
		abcCore_c();
		~abcCore_c();
		abcResult_e	init(int initVal);
		abcResult_e	shutdown(int shutdownVal);

		// depricate this.... or restrict usage to just this object....
		// except its a singleton, so its not of much value
		void		setErrorReason(abcReason_e reasonSet);
		abcReason_e	getErrorReason();

		abcResult_e	updatePrimes(int maxValue);
		int			findPrime(int target);
		abcResult_e printPrimes(int low=2, int high=100);

		// standard crc geberators for hashing etc;
		abcResult_e generateCrc32Table();
		uint32_t	computeCrc32(uint32_t seed, uint8_t *strPtr,int strLen);
		abcResult_e	generateCrc64Table();	// make the crc table
		uint64_t	computeCrc64(uint8_t *strPtr,int strLen);
}; // end class abcCore_c

// ==========   global interfaces =============
extern abcCore_c *abcGlobalCore;
extern abcMemMon_c *abcGlobalMem;

// abc api interface.... with init and shutdown
//
abcResult_e abcInit(int initVal);
abcResult_e abcShutdown(int initVal);
//
// global error reporting.... (depricate this !!)
void abcGlobalSetErrorReason(abcReason_e reason);
void abcGlobalResetErrorReason();

// random things of value
double  randomPercentage();
abcTime1m_t getTime1m();

#endif //__ABC_CORE_H__
///////////////////////
// EOF abcCore.h
