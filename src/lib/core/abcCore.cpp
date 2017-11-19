/************************************************************
 ****   abcCorp Copyright 2017-2017 All Rights reserved *****
 ************************************************************/
// Author: Bill Mears   bill@imears.com
// abcCore.cpp      2017-1201
//
// acknowledgement to GNU Free Documentation License 1.2 for CRC32.  https://rosettacode.org/wiki/CRC-32#Implementation_2
// 64-bit CRC implementation - andrewl from.... http://andrewl.dreamhosters.com/filedump/crc64.cpp.... didn't see a copyright notice.
//

#include "abcCore.h"

abcCore_c		  *globalCore = NULL;
abcMemMon_c 	  *globalMem = NULL;
abcGlobalConfig_s *globalConfig = NULL;



double  randomPercentage()
{
	return ((double)rand())/((double)((int64_t)(RAND_MAX)+1));
}


abcTime1m_t getTime1m()
{
	struct timeval nowTimeVal;
	abcTime1m_t now1m;
	gettimeofday(&nowTimeVal,NULL);
	now1m = (SEC_AT_1M * nowTimeVal.tv_sec) + nowTimeVal.tv_usec;
	return now1m;
}

abcResult_e abcInit(int initVal)
{
	if (!globalCore)
	{
		globalCore = new abcCore_c();
		globalMem = new abcMemMon_c("globalMemMon");	// memory tracker always on
		return globalCore->init(initVal);
	}
	else
	{
		fprintf(stderr,"%s already initialized\n",__FUNCTION__);
		return ABC_FAIL;
	}
}
abcResult_e abcShutdown(int displayVal)
{
	if (!globalCore)
	{
		return ABC_FAIL;
	}
	else
	{
		globalMem->shutdown(displayVal);
		globalCore->shutdown(displayVal);
		delete globalCore;
		delete globalMem;
		globalCore = NULL;
		globalMem = NULL;
	}
	return ABC_PASS;
}
void globalSetReason(abcReason_e reason)
{
	if (!globalCore)
	{
		abcInit(0);
	}
	globalCore->setReason(reason);
}
void globalResetReason() 
{
	globalSetReason(ABC_REASON_NONE);
}

///==========================================================T
// convenience methods used my the print and debugging macros
uint8_t globalPrintEnabled(debugPrint_e type)
{
	switch(type)
	{
		case DBGP_E:
			return globalConfig->printErrors;

		case DBGP_W:
			return globalConfig->printWarnings;

		case DBGP_P:
			return globalConfig->printPrint;

		case DBGP_D:
			return globalConfig->printDebug;

		case DBGP_UNKNOWN:
		default: 
			fprintf(stderr,"Unknown debugPrint_e type:%d\n",type);
			return TRUE;
	}
}
uint8_t globalAbortEnabled(debugAbort_e type)
{
	switch(type)
	{

		case DBGA_E:
			return globalConfig->abortErrors;

		case DBGA_UNKNOWN:
		default: 
			fprintf(stderr,"Unknown debugAbort_e type:%d\n",type);
			return TRUE;
	}
}

///=================================================================================
// signal handling.... bring all handled signals to a single routine for dispatching
void abcCoreHandleSignal(int signal)
{
	//globalCore->mutexLock();
	fprintf(stderr,"abcCore got signal %d\n",signal);

	switch (signal)
	{
		case SIGINT:
			exit(1);
		case SIGTERM:
			exit(1);
		default:
			fprintf(stderr,"Not handled.... continuing\n");
	}
	//globalCore->mutexUnlock();
}

// end signal handling
///=================================================================================

///////////////////////////////////////////
abcCore_c::abcCore_c()
{
	reason = ABC_REASON_NONE;
	config = (abcGlobalConfig_s *)calloc(1,sizeof(abcGlobalConfig_s));	// zero being the default state... we'll set some one's in init as better defaults
	mutexInit();
}
abcCore_c::~abcCore_c()
{
	pthread_mutex_destroy(&mutex);
	free(config);
}

abcResult_e abcCore_c::init(int initVal)
{
	mutexLock();

	abcResult_e res = initPrimes();	// can pass in a non-default number
	CHECK_ERROR(res,ABC_REASON_THREAD_INIT_FAILED,res);
	res = initSignalHandling();
	CHECK_ERROR(res,ABC_REASON_THREAD_INIT_FAILED,res);
	res = generateCrc32Table();
	CHECK_ERROR(res,ABC_REASON_THREAD_INIT_FAILED,res);
	res = generateCrc64Table();
	CHECK_ERROR(res,ABC_REASON_THREAD_INIT_FAILED,res);

	// setup printfing defaults as completely noise.  Let the user turn them off as needed
	config->printErrors = TRUE;
	config->printWarnings = TRUE;
	config->printPrint = TRUE;
	config->printDebug = TRUE;

	// setup abort defaults as strict.  Let the user tune down the strictness.
	config->abortErrors = TRUE;

	mutexUnlock();

	return ABC_PASS;
}

abcResult_e abcCore_c::shutdown(int shutdownVal)
{
	return ABC_PASS;
}

abcGlobalConfig_s *abcCore_c::getConfig()
{
	return config;
}


/// mutex handling
abcResult_e abcCore_c::mutexInit()
{
	pthread_mutexattr_t attrs;
	pthread_mutexattr_init(&attrs);
	pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_TYPE);
	int initFail=pthread_mutex_init(&mutex,&attrs);
	if (initFail)
	{
		return ABC_FAIL;
	}
	return ABC_PASS;
}
abcResult_e abcCore_c::mutexLock()
{
	int lockFail=pthread_mutex_lock(&mutex);
	if (lockFail)
	{
		return ABC_FAIL;
	}
	return ABC_PASS;
}
abcResult_e abcCore_c::mutexUnlock()
{
	int unlockFail=pthread_mutex_unlock(&mutex);
	if (unlockFail)
	{
		return ABC_FAIL;
	}
	return ABC_PASS;
}


void abcCore_c::setReason(abcReason_e reasonSet)
{
	reason = reasonSet;
};

abcReason_e abcCore_c::getReason()
{
	return reason;
};



abcResult_e abcCore_c::initSignalHandling()
{
	struct  sigaction  sigAct;
	struct  sigaction *saPtr = &sigAct;

	// handle sigInt and SigTerm the same way
	sigemptyset(&saPtr->sa_mask);
	saPtr->sa_flags = SA_RESTART;
	saPtr->sa_handler = abcCoreHandleSignal;
	sigaction(SIGINT,  saPtr, 0);
	sigaction(SIGTERM, saPtr, 0);
	sigaction(SIGHUP,  saPtr, 0);

	sigemptyset(&saPtr->sa_mask);
	saPtr->sa_flags = SA_RESTART;
	saPtr->sa_handler = abcCoreHandleSignal;
	sigaction(SIGABRT, saPtr, 0);
	return ABC_PASS;
}

abcResult_e abcCore_c::initPrimes(int maxValue)
{
	// enter here locked please
	sieveArray = (abcPrime_s *)calloc(maxValue,sizeof(abcPrime_s));
	sieveArray[1].primeRank = 1;	// special case
	sieveArraySize = maxValue;
	sievePrimeCount = 0;
	sieveWorkingPrime = 2;
	int testNumber = sieveWorkingPrime;
	while (testNumber <maxValue)
	{
		if ( ! sieveArray[testNumber].isNotPrime)
		{
			// this is a prime number.   Lets mark all of its multiples not prime
			sieveWorkingPrime = testNumber;
			sievePrimeCount++;

			int32_t markingMultiple = 1;
			sieveArray[sieveWorkingPrime].primeRank = sievePrimeCount;
			sieveArray[sieveWorkingPrime].isNotPrime = FALSE;	// should already be this value, this is for documentation
			sieveArray[sieveWorkingPrime].lastMultipleMarked = markingMultiple;
			sieveArray[sieveWorkingPrime].lastSeriesValue = sieveWorkingPrime;;

			int32_t seriesStart = 2 * sieveWorkingPrime;
			int i;
			for (i=seriesStart; i<maxValue;i += sieveWorkingPrime)
			{
				markingMultiple++;
				sieveArray[i].isNotPrime = TRUE;
				sieveArray[sieveWorkingPrime].lastMultipleMarked = markingMultiple;
				sieveArray[sieveWorkingPrime].lastSeriesValue = i;
			}
			sieveArray[sieveWorkingPrime].primeRank = sievePrimeCount;
			// printf("%d: %d is prime lastMultiple:%d val:%d \n",sievePrimeCount,sieveWorkingPrime, sieveArray[sieveWorkingPrime].lastMultipleMarked,sieveArray[sieveWorkingPrime].lastSeriesValue);
		}
		testNumber++;
	}
	//printf("Last prime = %d, with rank %d on arraySize of %d\n",sieveWorkingPrime, sievePrimeCount,sieveArraySize);
	return ABC_PASS;
};

// we'll update the sieve by copying the old work
// and then extending to to the new max
abcResult_e  abcCore_c::updatePrimes(int newSieveArraySize)
{
	mutexLock();
	if (newSieveArraySize < sieveArraySize)
	{
		ERROR(ABC_REASON_PARAM_TOO_SMALL);
		return ABC_FAIL;
	}
	// make a new array and copy old to new.
	abcPrime_s *newSieveArray = (abcPrime_s *)calloc(newSieveArraySize,sizeof(abcPrime_s));
	for (int i=0;i<sieveArraySize;i++)
	{
		newSieveArray[i] = sieveArray[i];
	}
	

	// go through the array and extend all prime multiples to the target value in the new array
	int oldArrayMember;
	int elem;
	for (oldArrayMember=2;oldArrayMember <= sieveWorkingPrime;oldArrayMember++)
	{
		// update all the reentrancy data as we go.
		int oldMarkingMultiple = newSieveArray[oldArrayMember].lastMultipleMarked;
		for (elem = newSieveArray[oldArrayMember].lastSeriesValue; elem < newSieveArraySize; elem += oldArrayMember)
		{
			newSieveArray[elem].isNotPrime = TRUE;
			newSieveArray[oldArrayMember].lastMultipleMarked = oldMarkingMultiple;
			newSieveArray[oldArrayMember].lastSeriesValue = elem;
			oldMarkingMultiple++;
		}
	}

	// now extend the sieve starting just past the last found prime
	// and continuing through the end of the sieveArray
	int testNumber = sieveWorkingPrime+1;
	while (testNumber <newSieveArraySize)
	{
		if ( ! newSieveArray[testNumber].isNotPrime)
		{
			// this is a prime number.   Lets mark all of its multiples not prime
			sieveWorkingPrime = testNumber;
			sievePrimeCount++;

			int32_t markingMultiple = 1;
			newSieveArray[sieveWorkingPrime].primeRank = sievePrimeCount;
			newSieveArray[sieveWorkingPrime].isNotPrime = FALSE;	// should already be this value, this is for documentation
			newSieveArray[sieveWorkingPrime].lastMultipleMarked = markingMultiple;
			newSieveArray[sieveWorkingPrime].lastSeriesValue = sieveWorkingPrime;;

			int32_t seriesStart = 2 * sieveWorkingPrime;
			int i;
			for (i=seriesStart; i<newSieveArraySize;i += sieveWorkingPrime)
			{
				markingMultiple++;
				newSieveArray[i].isNotPrime = TRUE;
				newSieveArray[sieveWorkingPrime].lastMultipleMarked = markingMultiple;
				newSieveArray[sieveWorkingPrime].lastSeriesValue = i;
			}
			newSieveArray[sieveWorkingPrime].primeRank = sievePrimeCount;
			//printf("%d: %d is prime lastMultiple:%d val:%d \n",sievePrimeCount,sieveWorkingPrime, newSieveArray[sieveWorkingPrime].lastMultipleMarked,newSieveArray[sieveWorkingPrime].lastSeriesValue);
		}
		testNumber++;
	}
	free(sieveArray);
	sieveArray  = newSieveArray;
	sieveArraySize = newSieveArraySize;
	//printf("Last prime = %d, with rank %d on arraySize of %d\n",sieveWorkingPrime, sievePrimeCount,sieveArraySize);

	// unlock before exit
	mutexUnlock();
	return ABC_PASS;
} // abcCore_c::updatePrimes()

// return a first prime bigger than targetVal
int  abcCore_c::findPrime(int targetVal)
{
	int testSize = targetVal;
	for (int retry = 0; retry < 4; retry ++)
	{
		int i = targetVal;
		while (i < sieveArraySize)
		{
			if ( ! sieveArray[i].isNotPrime)
			{
				return i;
			}
			i++;
		}
		testSize = (testSize * 110) / 100;	// over charge array with 15%
		//DEBUG_A("Retry %d  prime number by growing testArray to %d\n",retry+1,testSize);
		updatePrimes(testSize);
	}
	return 0;
}

abcResult_e  abcCore_c::printPrimes(int low, int high)
{
	if (high > sieveArraySize)  high = sieveArraySize;
	fprintf(stderr,"Primes by Reentrant Sieve Method.\n");
	fprintf(stderr,"Largest Prime:%d Count of Primes:%d  ArraySize:%d.\n", sieveWorkingPrime,sievePrimeCount,sieveArraySize);
	fprintf(stderr,"Printing Primes betweeon %d and %d\n",low,high);

	int i;
	for (i=low; i<high; i++)
	{
		if (!sieveArray[i].isNotPrime)
		{
			fprintf(stderr,"Rank:%5d  Prime:%d\n",sieveArray[i].primeRank,i);
		}
	}
	fprintf(stderr,"\n");
	return ABC_PASS;
}


///////////////////////////////////////////
///  CRC generation  (32 & 64 bit )
///////////////////////////////////////////

abcResult_e abcCore_c::generateCrc32Table()
{
	uint32_t rem;
	/* Calculate CRC table. */
	int i,j;
	for (i = 0; i < 256; i++)
	{
		rem = i;  /* remainder from polynomial division */
		for (j = 0; j < 8; j++) 
		{
			if (rem & 1) 
			{
				rem >>= 1;
				rem ^= 0xedb88320;
			} 
			else
			{
				rem >>= 1;
			}
		}
		crc32Table[i] = rem;
	}
	return ABC_PASS;
}

uint32_t abcCore_c::computeCrc32( uint32_t seed,  uint8_t *buf, int len)
{
	// enter locked please
	uint32_t crc = ~seed;
	uint8_t octet;
	uint8_t *p, *endP;

	endP = buf + len;
	for (p = buf; p < endP; p++)
	{
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ crc32Table[(crc & 0xff) ^ octet];
	}
	return ~crc;
}


//
// 
abcResult_e abcCore_c::generateCrc64Table()
{
	// enter locked please
	uint64_t poly = 0xc96c5795d7870f42LL;	// magic !!
	int i;
    for( i=0; i<256; ++i)
    {
    	uint64_t crc = i;
		int j;
    	for(j=0; j<8; ++j)
    	{
            // is current coefficient set?
    		if(crc & 1)
            { // yes, then assume it gets zero'd (by implied x^64 coefficient of dividend)
                crc >>= 1;
                // and add rest of the divisor
    			crc ^= poly;
            }
    		else
    		{ // no? then move to next coefficient
    			crc >>= 1;
            }
    	}
        crc64Table[i] = crc;
    }
	return ABC_PASS;
}

uint64_t abcCore_c::computeCrc64(uint8_t *strPtr,int strLen)
{

	uint64_t crc = 0;

	int i;
	uint8_t *ptr = strPtr;;
	for(i=0; i<strLen; ++i)
	{
		uint8_t  index = *ptr++  ^ crc;
		//uint8_t  index = strPtr[i]  ^ crc;
		uint64_t lookup = crc64Table[index];
		crc >>= 8;
		crc ^= lookup;
	}
	return crc;
}

////////////////////////
// EOF  abcCore.cpp
