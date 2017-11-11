/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com
// File:  abcThreads.h
// 
//

#ifndef __ABC_THREADS_H__
#define __ABC_THREADS_H__

#include "abcNodes.h"		// listNode is our baseclass

typedef enum abcThreadState_e
{
	ABC_THREADSTATE_UNKNOWN = 0,
	ABC_THREADSTATE_UNCONFIGURED,
	ABC_THREADSTATE_CONFIGURED,
	ABC_THREADSTATE_STARTING,
	ABC_THREADSTATE_RUNNING,
	ABC_THREADSTATE_PAUSING,
	ABC_THREADSTATE_PAUSED,
	ABC_THREADSTATE_RESUMING,
	ABC_THREADSTATE_STOPPING,
	ABC_THREADSTATE_STOPPED,
	ABC_THREADSTATE_KILLING,
	ABC_THREADSTATE_KILLED,
	ABC_THREADSTATE_ERROR,
} abcThreadState_e;


class abcThread_c : public abcListNode_c	// a node just for names and indexed by nameHash
{
	private:
		char				*name;
		abcThreadState_e	threadState;
		abcReason_e			errorReason;

		// thread stuff
		pthread_attr_t		threadAttr;
		pthread_t			threadId;
		int					lwpTid;

		//mutex stuff
		pthread_mutex_t     mutex;
		pthread_cond_t      condVar;
		pthread_cond_t      wfscCondVar;			// exclusive use condition variable to "wait for state changes"
		uint8_t				waitingForStateChange;


		// private methods
		abcThreadState_e	lock_N_getState();								// enter unlocked.. lock, getState, and return locked
		abcResult_e			setState_N_Unlock(abcThreadState_e setState);		// enter locked...  set state and unlock
		abcResult_e			setState(abcThreadState_e setState);			// set state using above
		abcResult_e			test_N_Set(abcThreadState_e confirmState, abcThreadState_e setState);	// use above to test for confirmState and if == set state to setState... while locked
		// moved to public for testing
		//abcResult_e			waitForState(abcThreadState_e confirmState, int millisecondsToWait = 0);	// zero means wait forever  



	protected:
		abcResult_e		initLocks();
		abcResult_e		lock();
		abcResult_e		unlock();
		abcResult_e		mainStartup();			// just after the thread starts
		abcResult_e		mainShutdown();			// just before the thread dies
		uint8_t			isRunning();			// TRUE or FALSE based on valid running threadState


	public:
		abcThread_c(const char *setName = NULL);
		~abcThread_c();


		// actually private.
		virtual void main();			// base class main is a trivial timer loop

		// standard node things for diagnostic printing... no promise that they work (yet)
		void setErrorReason(abcReason_e reason);
		abcReason_e getErrorReason();

		virtual char       		 *getObjType();  // the class name
		virtual char       		 *getObjName();  // the instance name when available
		virtual abcResult_e		 print(abcPrintStyle_e printStyle);
		abcResult_e        		 printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); //nonVirtual prinfbuff for this level of the object hiearchy

		abcThreadState_e		getThreadState();

		
		abcResult_e configure();		// this is the pre-start configuration.  put things here that effect the pthread before its startd
		abcResult_e	start();
		abcResult_e	stop();
		abcResult_e	kill();
		abcResult_e	pause();
		abcResult_e	resume();
		abcResult_e			waitForState(abcThreadState_e confirmState, int millisecondsToWait = 0);	// zero means wait forever

}; // end class abcThread_c

char *abcThreadStateAsStr(abcThreadState_e state);

#endif //__ABC_THREADS_H__

////////////////////////////
// EOF abcThreads.h

