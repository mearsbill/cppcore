/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com
// File:  abcThreads.cpp
// 
//

#include "abcThreads.h"	
#include "abcCore.h"

// this c-like function is called by pthread_create to access our main
// this is a virtual overload, so we pass "this" in and call main from here
void *abcVirtualThreadMain(void *vThis)
{
	//fprintf(stderr,"Got to here in %s\n",__FUNCTION__);
	abcThread_c *myThreadObj = (abcThread_c *)vThis;
	myThreadObj->main();
	return NULL;
}
char *abcThreadStateAsStr(abcThreadState_e state)
{
	switch (state)
	{
		case ABC_THREADSTATE_UNKNOWN:
			return (char *)"ABC_THREADSTATE_UNKNOWN";
		case ABC_THREADSTATE_UNCONFIGURED:
			return (char *)"ABC_THREADSTATE_UNCONFIGURED";
		case ABC_THREADSTATE_CONFIGURED:
			return (char *)"ABC_THREADSTATE_CONFIGURED";
		case ABC_THREADSTATE_STARTING:
			return (char *)"ABC_THREADSTATE_STARTING";
		case ABC_THREADSTATE_RUNNING:
			return (char *)"ABC_THREADSTATE_RUNNING";
		case ABC_THREADSTATE_PAUSING:
			return (char *)"ABC_THREADSTATE_PAUSING";
		case ABC_THREADSTATE_PAUSED:
			return (char *)"ABC_THREADSTATE_PAUSED";
		case ABC_THREADSTATE_RESUMING:
			return (char *)"ABC_THREADSTATE_RESUMING";
		case ABC_THREADSTATE_STOPPING:
			return (char *)"ABC_THREADSTATE_STOPPING";
		case ABC_THREADSTATE_STOPPED:
			return (char *)"ABC_THREADSTATE_STOPPED";
		case ABC_THREADSTATE_KILLING:
			return (char *)"ABC_THREADSTATE_KILLING";
		case ABC_THREADSTATE_KILLED:
			return (char *)"ABC_THREADSTATE_KILLED";
		case ABC_THREADSTATE_ERROR:
			return (char *)"ABC_THREADSTATE_ERROR";
		default:
			return (char *)"unknown ABC_THREADSTATE";
	}
}




//
// base constructor for abcThread_c
// call here for all thread subclasses (first)
//
abcThread_c::abcThread_c(const char *setName )
{
	if (setName)
	{
		name = strdup(setName);
	}
	abcResult_e res = initLocks();
	CHECK_ERROR(res != ABC_PASS,ABC_REASON_MUTEX_INIT_FAILED,);
	threadState = ABC_THREADSTATE_UNKNOWN;
	waitingForStateChange = FALSE;

} // end abcThread_c::abcThread_c

//
// base class destructor
//
abcThread_c::~abcThread_c()
{
	// more things here... be order sensitive.... 

	// kill the thread if running....
	// should already be stopped by now.

	// destoy thread attribute once thread is confirmed stopped
	int res = pthread_attr_destroy(&threadAttr);
	CHECK_ERROR(res,ABC_REASON_THREAD_INIT_FAILED,);
} // end abcThread_c::~abcThread_c()

char *abcThread_c::getObjType()
{
	return (char *)"abcThread_c";
} // end abcThread_c::getObjType()

char *abcThread_c::getObjName()
{
	return name;
} // end abcThread_c::getObjName()

// Error handling stuff & print stuff
void abcThread_c::setErrorReason(abcReason_e reason)
{
	errorReason = reason;
}
abcReason_e abcThread_c::getErrorReason()
{
	return errorReason;
}

abcResult_e abcThread_c::print(abcPrintStyle_e printStyle)
{
	TRACE_NONIMPL("abcThread_c");
	return ABC_FAIL;
} // end abcThread_c::print()

abcResult_e abcThread_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	TRACE_NONIMPL("abcThread_c");
	return ABC_FAIL;
} // end abcThread_c::printBuff()

//
//////////////////////////////////////////////////////////
// mutex and lock stuff
////////////////////////////////////////////////////////
//
// here to init all mutex and condition variables
// some users will sleep on these condition variables,
// so we'll need to anticipate destroying them to unblock users.
//
abcResult_e abcThread_c::initLocks()
{
	pthread_mutexattr_t attrs;
	pthread_mutexattr_init(&attrs);
	pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_TYPE);

	int res = pthread_mutex_init(&mutex,&attrs);
	CHECK_ERROR(res,ABC_REASON_MUTEX_INIT_FAILED,ABC_FAIL);

	// now init the condition variables
	res = pthread_cond_init(&condVar,NULL);
	CHECK_ERROR(res,ABC_REASON_CONDVAR_INIT_FAILED,ABC_FAIL);

	res = pthread_cond_init(&wfscCondVar,NULL);
	CHECK_ERROR(res,ABC_REASON_CONDVAR_INIT_FAILED,ABC_FAIL);

	return ABC_PASS;

} // end abcThread_c::initLocks()

abcResult_e abcThread_c::lock()
{
	int res = pthread_mutex_lock(&mutex);
	CHECK_ERROR(res,ABC_REASON_THREAD_LOCK_FAILED,ABC_FAIL);

	return ABC_PASS;
} // end abcThread_c::lock()
abcResult_e     abcThread_c::unlock()
{
	int res = pthread_mutex_unlock(&mutex);
	CHECK_ERROR(res,ABC_REASON_THREAD_UNLOCK_FAILED,ABC_FAIL);

	return ABC_PASS;
} // end abcThread_c::lock()

abcThreadState_e abcThread_c::getThreadState()
{
	return threadState;
}
abcThreadState_e abcThread_c::lock_N_getState()
{
	abcResult_e res = lock();
	CHECK_ERROR(res,ABC_REASON_THREAD_LOCK_FAILED,ABC_THREADSTATE_ERROR);
	return threadState;
} // end abcThread_c::lock_N_getState()
//
abcResult_e	abcThread_c::setState_N_Unlock(abcThreadState_e setState)
{
	threadState = setState;
	if (waitingForStateChange)
	{
		// wake'em all up.  
		int iRes = pthread_cond_broadcast(&wfscCondVar);
		if (iRes)
		{
			ERROR(ABC_REASON_CONDVAR_BROADCAST_FAILED);
			unlock();
			return ABC_FAIL;
		}
	}
	return unlock();
} // end abcThread_c::setStat_N_Unlock(abcThreadState_e setState)
//
abcResult_e	abcThread_c::setState(abcThreadState_e setState)
{
	abcResult_e res = lock();
	CHECK_ERROR(res,ABC_REASON_THREAD_LOCK_FAILED,res);
	return setState_N_Unlock(setState);

} // end abcThread_c::setState(abcThreadState_e setState)
//
abcResult_e	abcThread_c::test_N_Set(abcThreadState_e confirmState, abcThreadState_e setState)
{
	abcResult_e res;
	res = lock();
	CHECK_ERROR(res,ABC_REASON_THREAD_LOCK_FAILED,res);
	if (threadState != confirmState)
	{
		unlock();
		return ABC_RETRY;
	}
	threadState = setState;
	if (waitingForStateChange)
	{
		// wake'em all up.  
		int iRes = pthread_cond_broadcast(&wfscCondVar);
		if (iRes)
		{
			ERROR(ABC_REASON_CONDVAR_BROADCAST_FAILED);
			unlock();
			return ABC_FAIL;
		}
	}
	res = unlock();
	CHECK_ERROR(res,ABC_REASON_THREAD_UNLOCK_FAILED,res);
	return res;
} // end abcThread_c::testNSet(abcThreadState_e confirmState, abcThreadState_e setState)
//
//  Wait for threadState to become desiredState.  Return after wait time with status as ABC_RETRY unless a failure occurs
abcResult_e	abcThread_c::waitForState(abcThreadState_e desiredState, int millisecondsWait)	// zero means no wait
{
	abcResult_e res = lock();
	CHECK_ERROR(res,ABC_REASON_THREAD_LOCK_FAILED,ABC_FAIL);

	if (threadState != desiredState)
	{
		waitingForStateChange = TRUE;
		int res;
		if (millisecondsWait)
		{
			// wait until a state change occurs
			// or until we timeout
			//
			//
			// awkward !! get uSec... param milliseconds... convert all to nsec
			struct timeval tv;
			struct timespec ts;
			gettimeofday(&tv,NULL);
			int64_t nsecWait = ((int64_t)tv.tv_usec *1000LL) + ((int64_t)millisecondsWait *1000000LL);
			ts.tv_nsec = nsecWait % 1000000000LL;
			ts.tv_sec = tv.tv_sec + (nsecWait / 1000000000LL );
			//fprintf(stderr,"about to wait %lld:%lld\n",(int64_t)ts.tv_sec-(int64_t)tv.tv_sec,((int64_t)(ts.tv_nsec)/1000LL)-(int64_t)tv.tv_usec);
			res = pthread_cond_timedwait(&wfscCondVar,&mutex,&ts);
		}
		else
		{
			// wait until a state change occurs (timeout = forever)
			res = pthread_cond_wait(&wfscCondVar,&mutex);
		}
		waitingForStateChange = FALSE;
		if (res)
		{
			if (res == ETIMEDOUT)
			{
				unlock();
				return ABC_RETRY;
			}
			unlock();
			return ABC_FAIL;
		}
		if (threadState != desiredState)
		{
			unlock();
			return ABC_RETRY;
		}
		unlock();
		return ABC_PASS;
	}
	unlock();
	return ABC_PASS;

} // end abcResult_e	abcThread_c::waitForState()
//
//////////////////////////T/////////////////////////////////////////////////
//////////////////  Thread config stuff
//////////////////  Thread config stuff
///////////////////////////////////////////////////////////////////////////

// pre-start configuration of the thread itself
abcResult_e abcThread_c::configure()
{
	abcThreadState_e myThreadState = lock_N_getState();
	if ((myThreadState != ABC_THREADSTATE_UNCONFIGURED) && (myThreadState != ABC_THREADSTATE_UNKNOWN))
	{
		unlock();
		ERROR(ABC_REASON_THREAD_INIT_FAILED);
		return ABC_FAIL;
	}
	// set up our thread defaults
	int res = pthread_attr_init(&threadAttr);
	if (res != 0)
	{
		unlock();
		ERROR(ABC_REASON_THREAD_ATTR_INIT_FAILED)
		return ABC_FAIL;
	}
	res = pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
	if (res != 0)
	{
		unlock();
		ERROR(ABC_REASON_THREAD_ATTR_INIT_FAILED)
		return ABC_FAIL;
	}
	res = setpriority(PRIO_PROCESS,0,ABC_BASE_PRIORITY);
	if (res != 0)
	{
		unlock();
		ERROR(ABC_REASON_THREAD_SETPRIORITY_FAILED);
		return ABC_FAIL;
	}
	//
	// for a good time check out this cool parameters  // cpu_set_t // CPU_SET // sched_setaffinity // sched_setscheduler
	// TODO.... set other things like stack size 

	// set state to configured... not using locking yet
	return setState_N_Unlock(ABC_THREADSTATE_CONFIGURED);

} // end abcThread_c::configure()

abcResult_e abcThread_c::start()
{
	abcThreadState_e myThreadState = lock_N_getState(); // note: going locked !!!
	if (myThreadState != ABC_THREADSTATE_CONFIGURED)
	{
		ERROR(ABC_REASON_THREAD_NOT_CONFIGURED);
		unlock();
		return ABC_FAIL;
	}

	// this will start the actual thread and fetch the process threadId
	int res = pthread_create(&threadId,&threadAttr,&abcVirtualThreadMain,this);
	if (res != 0)
	{
		ERROR(ABC_REASON_THREAD_INIT_FAILED);
		unlock();
		return ABC_FAIL;
	}
	abcResult_e result = setState_N_Unlock(ABC_THREADSTATE_STARTING);
	return result;
} // end abcThread_c::start()
//
abcResult_e abcThread_c::mainStartup()
{
	abcThreadState_e myThreadState = lock_N_getState(); // note: going locked !!!
	if (myThreadState != ABC_THREADSTATE_STARTING)
	{
		ERROR(ABC_REASON_THREAD_NOT_CONFIGURED);
		unlock();
		return ABC_FAIL;
	}

	// block popular signals from this thread object... move this to after the thread is started !
	sigset_t sigMask;
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGINT);
	sigaddset(&sigMask, SIGHUP);
	sigaddset(&sigMask, SIGUSR1);
	sigaddset(&sigMask, SIGUSR2);
	sigaddset(&sigMask, SIGPIPE);
	sigaddset(&sigMask, SIGTERM);
	sigaddset(&sigMask, SIGALRM);
	int res = pthread_sigmask (SIG_BLOCK, &sigMask, NULL);
	if (res != 0)
	{
		ERROR(ABC_REASON_THREAD_SETSIGNALMASK_FAILED);
		unlock();
		return ABC_FAIL;
	}
	// get the lightweight processId.
	lwpTid = 0;
	DEBUG("No syscall on Darwin... need \"lwpTid = syscall(SYS_gettid)\" here\n");

	abcResult_e result = setState_N_Unlock(ABC_THREADSTATE_RUNNING);
	return result;
} // end abcThread_c::mainStartup()

abcResult_e abcThread_c::mainShutdown()
{
	abcThreadState_e myThreadState = lock_N_getState(); // note: going locked !!!
	if (myThreadState != ABC_THREADSTATE_STOPPING)
	{
		ERROR(ABC_REASON_THREAD_EXPECTED_STOPPING_STATE);
		unlock();
		return ABC_FAIL;
	}
	return setState_N_Unlock(ABC_THREADSTATE_STOPPED);
} // end abcThread_c::mainShutdown()


abcResult_e abcThread_c::stop()
{
	TRACE_NONIMPL("abcThread_c");
	return ABC_PASS;
} // end abcThread_c::stop()

abcResult_e abcThread_c::kill()
{
	TRACE_NONIMPL("abcThread_c");
	return ABC_FAIL;
} // end abcThread_c::kill()

abcResult_e abcThread_c::pause()
{
	TRACE_NONIMPL("abcThread_c");
	return ABC_FAIL;
} // end abcThread_c::pause()

abcResult_e abcThread_c::resume()
{
	TRACE_NONIMPL("abcThread_c");
	return ABC_FAIL;
} // end abcThread_c::resume()

uint8_t abcThread_c::isRunning()
{
	switch(threadState)
	{
		case ABC_THREADSTATE_RUNNING:
		case ABC_THREADSTATE_PAUSING:
		case ABC_THREADSTATE_PAUSED:
		case ABC_THREADSTATE_RESUMING:
			return TRUE;

		default:
			return FALSE;
	}
}

///////////////////////

void abcThread_c::main()
{
	//fprintf(stderr,"Got to here:%s %s:%d\n",__FUNCTION__,__FILE__,__LINE__);
	// 
	abcResult_e res = mainStartup();	// always call... sets critical state information
	CHECK_ERROR(res,ABC_REASON_THREAD_INIT_FAILED,);
	int loopCount = 10;
	while (isRunning())
	{
		fprintf(stderr,"In main... stopping in %d... \n", loopCount);
		sleep(1);
		loopCount--;
		if (loopCount < 1)
		{
			fprintf(stderr,"Initiating shutdown ... \n");
			setState(ABC_THREADSTATE_STOPPING);
		}
	}
	fprintf(stderr,"Exiting main loop ... \n");
	//
	mainShutdown();  // call this last before the thread dies
	//
	fprintf(stderr,"Thread dying\n");
} // end abcThread_c::main()

////////////////////////////
// EOF abcThreads.cpp

