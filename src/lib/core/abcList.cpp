/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com   
// File:  abcList.cpp	Date:  2017-1201
// 
// abcNode_c is the most basic node of the list construct
// abcListNode_c is the base class of the list object's list node
// abcList_c is the the basic list object
//
#include "abcList.h"

// may want these elsewhere later, but for now
// this is a good spot for all configuration stuff
// changes here don't cause a massive recompile.
#define _LIST_RIGOROUS_TESTING_


#include "abcDebugMacros.h"
#include "abcCore.h"

abcList_c::abcList_c(const char *setName)
{
	name = setName ? strdup(setName) : NULL; 

	headNode = NULL;
	tailNode = NULL;
	currNode = NULL;

	nodeCount=0;
	isSorted = FALSE;

	// init the locks, etc
	lockingEnabled = FALSE;
	writerIsWaiting = FALSE;
	readerIsWaiting = FALSE;
	registeredReaders = 0;
}
abcList_c::~abcList_c()
{
	empty();	// this will delete all listNodes and their contents
	if (name)
	{
		free(name);
		name = NULL;
	}
}

/////////////////////////////////////////////////////////////////
// multi-threading api.  Register for reads and lock for writes.
// PLEASE NOTE: 
//		Registration and locking are for multi-threading only.  Different spots in the same
// 		thread cannot use multithread registration or locking to avoid state corruption and must use stateless operation
// 		to avoid changing the "curr" "head\n" or "tail" pointers behind the back of a different bit of code.  The only exception
//		to this principal is to use NoWait version which will return with status if currently locked.
//
abcResult_e	abcList_c::enableLocking()
{
	int res;

	// init the lock mutex
	pthread_mutexattr_t attrs;
	pthread_mutexattr_init(&attrs);
	pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_TYPE);
	res = pthread_mutex_init(&mutex,&attrs);
	CHECK_ERROR(res,ABC_REASON_MUTEX_INIT_FAILED,ABC_FAIL);

	// now init the condition variables for read and write waiting
	res = pthread_cond_init(&readerCondVar,NULL);
	CHECK_ERROR(res,ABC_REASON_CONDVAR_INIT_FAILED,ABC_FAIL);

	res = pthread_cond_init(&writerCondVar,NULL);
	CHECK_ERROR(res,ABC_REASON_CONDVAR_INIT_FAILED,ABC_FAIL);

	lockingEnabled=TRUE;
	return ABC_PASS;
} // end abcList_c::enableLocking()
//
// disable locking 
abcResult_e	abcList_c::disableLocking()
{
	if (!lockingEnabled)
	{
		// don't fail this silly case... just return
		return ABC_PASS;
	}
	lockingEnabled=FALSE;
	int res = pthread_cond_destroy(&readerCondVar);
	CHECK_ERROR(res,ABC_REASON_MUTEX_DESTROY_FAILED,ABC_FAIL);

	res = pthread_cond_destroy(&writerCondVar);
	CHECK_ERROR(res,ABC_REASON_MUTEX_DESTROY_FAILED,ABC_FAIL);

	res = pthread_mutex_destroy(&mutex);
	CHECK_ERROR(res,ABC_REASON_MUTEX_DESTROY_FAILED,ABC_FAIL);

	return ABC_PASS;
} // end abcList_c::disableLocking()
//
abcResult_e     abcList_c::lock()
{
	CHECK_ERROR(!lockingEnabled,ABC_REASON_MUTEX_NOT_INITIALIZED,ABC_FAIL);

	int res = pthread_mutex_lock(&mutex);
	CHECK_ERROR(res,ABC_REASON_LIST_LOCK_FAILED,ABC_FAIL);
	return ABC_PASS;
} // end abcList_c::lock()
//
abcResult_e     abcList_c::unlock()
{
	CHECK_ERROR(!lockingEnabled,ABC_REASON_MUTEX_NOT_INITIALIZED,ABC_FAIL);

	int res = pthread_mutex_unlock(&mutex);
	CHECK_ERROR(res,ABC_REASON_LIST_UNLOCK_FAILED,ABC_FAIL);
	return ABC_PASS;
} // end abcList_c::lock()
//
abcResult_e	abcList_c::readRegister()
{
	lock();
	// sanity check.  will never get here while write locked!

	if (writerIsWaiting)	// read this as "if writer is waiting for zero registered reads then ..."
	{
		// we must therefore wait for the writer to first get the lock and then finish with it.
		// at which time writer will no longer be waiting... but one or more
		// readers will still be waiting
		readerIsWaiting=TRUE;
		pthread_cond_wait(&writerCondVar,&mutex);	// unlocked while waiting
		readerIsWaiting=FALSE;
	}
	registeredReaders++;
	unlock();
	return ABC_PASS;
} // end abcList_c::readRegister();
//
abcResult_e	abcList_c::readRegisterNoWait()
{
	lock();
	if (writerIsWaiting)	// read this as "if writer waiting for zero registered reads then ..."
	{
		// then we should  not increase the number of readers
		// so we'll return with status of RETRY
		unlock();
		return ABC_RETRY;	// instead tell user to try again
	}
	registeredReaders++;
	unlock();
	return ABC_PASS;
} // end abcList_c::readRegisterNoWait()
//
abcResult_e	abcList_c::readRelease()
{
	lock();
	if (--registeredReaders < 0)
	{
		WARNING(ABC_REASON_LIST_BAD_READ_RELEASE_COUNT);
		registeredReaders = 0;
	}
	if (writerIsWaiting && (registeredReaders == 0))
	{
		// wakeup a single writer (this reader is sending a signal)
		pthread_cond_signal(&readerCondVar);
	}
	unlock();
	return ABC_PASS;
} // end abcList_c::readRelease()
//
abcResult_e	abcList_c::writeLock()
{
	// locking write requires waiting for all readers to be done.
	abcResult_e lockStatus = lock();
	CHECK_ERROR(lockStatus,ABC_REASON_LIST_LOCK_FAILED,ABC_FAIL);
	if (registeredReaders > 0)
	{
		// we must wait for all readers to complete
		// wait for a signal from the readRelease that
		// all readers are done
		writerIsWaiting = TRUE;
		pthread_cond_wait(&readerCondVar,&mutex);
		writerIsWaiting = FALSE;
	}
	// NOTICE WE ARE RETURNING LOCKED !!!
	return ABC_PASS;
} // end abcList_c::writeLock()
//
abcResult_e	abcList_c::writeLockNoWait()
{
	// locking write requires waiting for all readers to be done.
	abcResult_e lockStatus = lock();
	CHECK_ERROR(lockStatus,ABC_REASON_LIST_LOCK_FAILED,ABC_FAIL);
	if (registeredReaders > 0)
	{
		unlock();
		return ABC_RETRY;
	}
	// NOTICE WE ARE RETURNING LOCKED !!!
	return ABC_PASS;
} // end	abcList_c::writeLockNoWait()
abcResult_e	abcList_c::writeRelease()
{
	// WE ENTER LOCKED !!
	if (readerIsWaiting)
	{
		// we're waiting on the writer to be done (which we are now !)
		// if any reader  waiting on the writer (us) broadcast to any and all
		// that we're done.  They will all unblock at once.
		pthread_cond_broadcast(&writerCondVar);
	}
	abcResult_e unlockResult = unlock();
	CHECK_ERROR(unlockResult,ABC_REASON_LIST_UNLOCK_FAILED,ABC_FAIL);
	return ABC_PASS;
} // end abcList_c::writeRelease()

char *abcList_c::getObjType()
{
	return (char *)"abcList_c";
}

char *abcList_c::getObjName()
{
	return name;
}

//  printDepth=depth of "has a" membe to drill into , printStyle=style to print
abcResult_e abcList_c::print(abcPrintStyle_e printStyle)
{
	char pBuff[256];
	printBuff(pBuff,256,printStyle);
	fprintf(stderr,"%s\n",pBuff);

	abcListNode_c *walkNode = headNode;
	while (walkNode != NULL)
	{
		walkNode->print(printStyle);
		walkNode = (abcListNode_c *)walkNode->next();
	}
	return ABC_PASS;
}
//nonVirtual prinfbuff for this level of the object hiearchy
abcResult_e abcList_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char baseBuff[128];
	abcResult_e result;
	result = abcListNode_c::printBuff(baseBuff,128,printStyle);
	snprintf(pBuff,pbuffSize,"ps%d =>%s h:%p t:%p nodeCnt:%lld sorted:%s err:%s",printStyle,baseBuff,headNode,tailNode,(int64_t)nodeCount,yesNoAsStr(isSorted),STRINGIFY(errorReason));
	return ABC_PASS;
}

//////////////////////////////////////////
// stateful operation
//  all these methods maintain currNode

abcListNode_c		*abcList_c::head()
{
	currNode = headNode;
	return currNode;
}
abcListNode_c		*abcList_c::tail()
{
	currNode = tailNode;
	return currNode;
}
abcListNode_c		*abcList_c::next()
{
	if (!currNode) return NULL;
	currNode = (abcListNode_c *)currNode->next();
	return currNode;
}
abcListNode_c		*abcList_c::prev()
{
	if (!currNode) return NULL;
	currNode = (abcListNode_c *)currNode->prev();
	return currNode;
}

abcListNode_c		*abcList_c::getCurr()
{
	return currNode;
}
////////
// thats it for stateful operations... 
// most oerations do not maintain currNode memeber
///////////////////////////////////////////////////
//
// stateless operation
//
abcListNode_c *abcList_c::getHead()
{
	return headNode;
}
abcListNode_c *abcList_c::getTail()
{
	return tailNode;
}

int64_t abcList_c::getNodeCount()
{
	return nodeCount;
}


abcResult_e abcList_c::addHead(abcListNode_c *nodeToAdd)
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	CHECK_ERROR(!nodeToAdd,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR(nodeToAdd->owner,ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);
	CHECK_ERROR((isSorted && (nodeCount > 0)),ABC_REASON_LIST_IS_SORTED,ABC_FAIL);
#endif
	return  abcList_c::addHeadPriv(nodeToAdd);
}
abcResult_e abcList_c::addHeadPriv(abcListNode_c *nodeToAdd)
{
	// everything pre-validated from here.
	// add additional checks to public routines or other callers

	nodeToAdd->owner = this;
	if (nodeCount)
	{
		// list already exists
		// add new node before the current head
		// configure new head node
		nodeToAdd->prevNode = NULL;
		nodeToAdd->nextNode = headNode;

		// fix the old head prev 
		headNode->prevNode = nodeToAdd;

		// now list object 
		headNode = nodeToAdd;
		nodeCount++;
	}
	else
	{
		// brand new list
		headNode = tailNode = nodeToAdd;
		nodeCount = 1;
	}
	return ABC_PASS;
} // end abcList_c::addHeadPriv()

abcResult_e abcList_c::addTail(abcListNode_c *nodeToAdd)							 // add a new node at tail and update tail.  Do not update current
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	CHECK_ERROR(!nodeToAdd,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR(nodeToAdd->owner,ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);
	CHECK_ERROR((isSorted && (nodeCount > 0)),ABC_REASON_LIST_IS_SORTED,ABC_FAIL);
#endif
	return abcList_c::addTailPriv(nodeToAdd);							 // add a new node at tail and update tail.  Do not update current
}
abcResult_e abcList_c::addTailPriv(abcListNode_c *nodeToAdd)							 // add a new node at tail and update tail.  Do not update current
{
	nodeToAdd->owner = this;	// checkOwner

	if (nodeCount != 0)
	{
		// list already exists
		// add new node before the current head
		// configure new head node
		nodeToAdd->prevNode = tailNode;
		nodeToAdd->nextNode = NULL;

		// fix the old head prev 
		tailNode->nextNode = nodeToAdd;

		// now list object 
		tailNode = nodeToAdd;
		nodeCount++;
	}
	else
	{
		// brand new list
		headNode = tailNode = nodeToAdd;
		nodeCount = 1;
	}
	return ABC_PASS;
} // end abcList_c::addTail()

//
// common routine for insertion... set state if indicated
//
abcResult_e abcList_c::addMiddle(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	CHECK_ERROR(!nodeToInsert,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR(nodeToInsert->owner,ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);
	CHECK_ERROR(nodeToFollow->owner!=this,ABC_REASON_NODE_NOT_ON_LIST,ABC_FAIL);
	CHECK_ERROR((isSorted && (nodeCount > 0)),ABC_REASON_LIST_IS_SORTED,ABC_FAIL);
#endif
	return abcList_c::insertPriv(nodeToInsert, nodeToFollow);
}

abcResult_e abcList_c::insert(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	CHECK_ERROR(!nodeToInsert,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR(nodeToInsert->owner,ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);
	CHECK_ERROR(nodeToFollow->owner!=this,ABC_REASON_NODE_NOT_ON_LIST,ABC_FAIL);
	CHECK_ERROR((isSorted && (nodeCount > 0)),ABC_REASON_LIST_IS_SORTED,ABC_FAIL);
#endif
	return abcList_c::insertPriv(nodeToInsert, nodeToFollow);
}
abcResult_e abcList_c::insertPriv(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
	nodeToInsert->owner = this;
	if (nodeCount)
	{
		// list already exists
#ifdef _LIST_RIGOROUS_TESTING_
		// sanity test on the node being added
		CHECK_ERROR(!nodeToFollow,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
#endif

		// hook in all the links
		nodeToInsert->prevNode = nodeToFollow;
		nodeToInsert->nextNode = nodeToFollow->nextNode;

		// handle "prev" link for nodeToFollow's next 
		// if nodeToFollow is the tail... nodeToFollowNext will be null
		abcListNode_c *nodeToFollowNext = (abcListNode_c *)nodeToFollow->nextNode;
		if (nodeToFollowNext)
		{
			// not tail
			nodeToFollowNext->prevNode = nodeToInsert;
		}
		else
		{
			// is tail..... must update tail
			tailNode = nodeToInsert;
			tailNode->nextNode = NULL;			// safety move
		}
		nodeToFollow->nextNode = nodeToInsert;

		// don't foreg the count
		nodeCount++;
	}
	else
	{
		// brand new list
#ifdef _LIST_RIGOROUS_TESTING_
		// sanity test on the node being added
		CHECK_ERROR(nodeToFollow,ABC_REASON_NODE_PARAM_SHOULD_BE_NULL,ABC_FAIL); // its on a different list
#endif
		headNode = tailNode = nodeToInsert;
		nodeCount = 1;
	}
	return ABC_PASS;
} // end abcList_c::insertCommon()

abcResult_e abcList_c::remove(abcListNode_c *nodeToRemove)
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	CHECK_ERROR(!nodeToRemove,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR((nodeToRemove->owner != this),ABC_REASON_NODE_NOT_ON_LIST,ABC_FAIL);
#endif
	if (nodeCount < 2)
	{
#ifdef _LIST_RIGOROUS_TESTING_
		CHECK_ERROR(!nodeCount,ABC_REASON_NODE_BAD_LINK_STRUCTURE,ABC_FAIL);
		CHECK_ERROR((nodeToRemove != headNode),ABC_REASON_NODE_BAD_LINK_STRUCTURE,ABC_FAIL); // node shoud be both head and tail
		CHECK_ERROR((nodeToRemove != tailNode),ABC_REASON_NODE_BAD_LINK_STRUCTURE,ABC_FAIL); // node shoud be both head and tail
#endif
		// with just one node, we're emptying the list
		headNode = tailNode = NULL;
		nodeCount = 0;
	} // end just one node
	else
	{ // more than one node on the list
		if (nodeToRemove == headNode)
		{   // removing at head
			headNode = (abcListNode_c *)headNode->nextNode;
			headNode->prevNode = NULL;
		}
		else if (nodeToRemove == tailNode)
		{   // removing at tail
			tailNode = (abcListNode_c *)tailNode->prevNode;
			tailNode->nextNode = NULL;
		}
		else
		{   // removing not at head or tail
			abcListNode_c *leftNeb = (abcListNode_c *)nodeToRemove->prevNode;
			abcListNode_c *rightNeb = (abcListNode_c *)nodeToRemove->nextNode;
			leftNeb->nextNode = rightNeb;
			rightNeb->prevNode = leftNeb;
		}
		nodeCount--;
	}
	// make sure the removed node can link back into the list
	nodeToRemove->prevNode = nodeToRemove->nextNode = NULL;
	nodeToRemove->owner=NULL;
	return ABC_PASS;
} // end abcList_c::remove(abcListNode_c *nodeToRemove)

// remove and return the head of the list
abcListNode_c	*abcList_c::pullHead()
{
	abcListNode_c *returnVal = getHead();
	if (returnVal)
	{
		remove(returnVal);
	}
	return returnVal;
}

// remove and return the tail of the list
abcListNode_c	*abcList_c::pullTail()
{
	abcListNode_c *returnVal = getTail();
	if (returnVal)
	{
		remove(returnVal);
	}
	return returnVal;
}


////////////////////////
// Here is our simple linear list sorted add... which looks from one end of the
// list or the other and proceeds until it finds the proper slot for insertion
// direction only effects performance.   For example when taking from head
// processing and then adding back to list,  towardsNext should probably be zero (start at tail)
abcResult_e abcList_c::addSorted(abcListNode_c *nodeToAdd, const uint8_t towardsNext )
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	CHECK_ERROR(!nodeToAdd,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR(nodeToAdd->owner,ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL); // its on a different list
	CHECK_ERROR((!isSorted && nodeCount > 0),ABC_REASON_LIST_NOT_SORTED,ABC_FAIL); // its on a different list
#endif
	nodeToAdd->owner = this;
	isSorted = TRUE;
	nodeKey_s *searchKey = &(nodeToAdd->key);
	abcListNode_c *walkNode = towardsNext ? headNode : tailNode;
	abcListNode_c *lastWalkNode = NULL;
	while (walkNode)
	{
		abcResult_e diffResult = walkNode->diffKey(searchKey);
		CHECK_ERROR(diffResult == ABC_ERROR,ABC_REASON_NODE_KEY_DIFF_ERROR,ABC_FAIL);
		if (towardsNext)
		{
			// moving right (towards tail)  Continue while 
			// addNode is >  the walked node
			if (diffResult <= ABC_EQUAL)
			{
				if (!lastWalkNode)
				{
					return addHeadPriv(nodeToAdd);
				}
				else
				{
					return abcList_c::insertPriv(nodeToAdd,lastWalkNode); // to left of walkNode.. ie after lastWalkNode. (before it left to right)
				}
			}
			lastWalkNode = walkNode;
			walkNode = (abcListNode_c *)walkNode->next();
		}
		else
		{
			// moving left (towardsPrev)
			// diffResult > 0 means addNode is greater than walkNode
			// thus its the spot to insert
			if (diffResult >= ABC_EQUAL)
			{
				if (!lastWalkNode)
				{
					return addTailPriv(nodeToAdd);
				}
				else
				{
					return abcList_c::insertPriv(nodeToAdd,walkNode);	// to right of walkNode (before it right to left)
				}
			}
			lastWalkNode = walkNode;
			walkNode = (abcListNode_c *)walkNode->prev();
		}
		// continue on to next node
	}

	// fell off the list... insert goes at the end.
	if (towardsNext)
	{
		return addTailPriv(nodeToAdd);
	}
	else
	{
		return addHeadPriv(nodeToAdd);
	}

} // end abcResult_e abcList_c::addSorted(abcListNode_c *nodeToAdd, const uint8_t towardsNext )

// search stateless only
abcListNode_c *abcList_c::findFirst(nodeKey_s *searchKey,uint8_t towardsNext, abcResult_e *resultOut)
{
	// get start pointer
	abcListNode_c *startNode = towardsNext ? headNode : tailNode;
	if (startNode == NULL)
	{
		// ran to end
		if (resultOut) *resultOut = ABC_PASS;
		return NULL;
	}
	return findActual(searchKey,startNode,towardsNext, resultOut);
} // end abcList_c::findFirst()

abcListNode_c *abcList_c::findNext(nodeKey_s *searchKey, abcListNode_c *previouslyFoundNode, uint8_t towardsNext, abcResult_e *resultOut)
{
	if (!previouslyFoundNode)
	{
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;	// an error, but not fatal
	}
	
	// move past the node we previously found and returned to the user
	abcListNode_c *startNode = (abcListNode_c *)(towardsNext ? previouslyFoundNode->next() : previouslyFoundNode->prev());

	// check for end of search
	if (startNode == NULL)
	{
		// ran to end
		if (resultOut) *resultOut = ABC_PASS;
		return NULL;
	}

	return findActual(searchKey,startNode,towardsNext);
} // end abcList_c::findNext()


// if null returned... means we've run off the end of the list.
// this is the place where real work is done.  First possible match is at "startNode"... continuing on in the
// desired dirction
abcListNode_c		*abcList_c::findActual(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext, abcResult_e *resultOut)
{
	// startNode already validated as non-null
	abcListNode_c *walkNode = startNode;
	abcResult_e diffResult;
	while (walkNode != NULL)
	{
		// read the question as "is the external key >= this node's key
		// if so, just past this node is the pace to do an insert
		diffResult = walkNode->diffKey(searchKey);
		if (diffResult ==  ABC_EQUAL)
		{
			// gone past it
			// if returnNode is NULL means we're before the startpoint
			if (resultOut) *resultOut = ABC_PASS;
			return walkNode;
		}
		walkNode = (abcListNode_c *)(towardsNext ? walkNode->next() : walkNode->prev());
	}
	if (resultOut)
	{
		if (diffResult == ABC_ERROR) *resultOut = ABC_FAIL; else *resultOut = ABC_PASS;
	}
	// doing error check down here because it won't slow down the search, and its here for robustness only.
	CHECK_ERROR(diffResult == ABC_ERROR,ABC_REASON_NODE_KEY_TYPE_INVALID,NULL);
	return NULL; // didn't find a match... we ran off the end of the list
} // end abcList_c::findActual()

//
// ============  Full list operations =============
//
abcResult_e abcList_c::empty()
{

	abcListNode_c *tmpNode = NULL;
	abcListNode_c *walkNode = headNode;
	while (walkNode)
	{
		tmpNode = (abcListNode_c *)walkNode->nextNode;

#ifdef _LIST_RIGOROUS_TESTING_
		walkNode->nextNode = NULL;
		walkNode->prevNode = NULL;
		walkNode->owner = NULL;
		nodeCount--;
#endif
		delete walkNode;
		walkNode = tmpNode;
	}
	headNode = tailNode = NULL;
	isSorted = FALSE;

#ifdef _LIST_RIGOROUS_TESTING_
	if (nodeCount)
	{
		ERROR(ABC_REASON_LIST_BAD_NODE_COUNT);
		nodeCount = 0;
		return ABC_FAIL;
	}
#else
	nodeCount = 0;
#endif
	return ABC_PASS;
}
abcResult_e abcList_c::diff(class abcList_c *otherList)
{
	return ABC_FAIL;
}

// make a new list and copy each abcListNode_c to it.
// this will use the virtual clone function of the abcListNode_c object
abcList_c *abcList_c::clone()
{
	// the list has a name but this nodes typically don't
	abcList_c *newList = new abcList_c();
	abcResult_e cloneResult = abcList_c::copyOut(newList);
	if (cloneResult != ABC_PASS)
	{
		delete newList;
		ERROR(ABC_REASON_LIST_CLONE_FAILED);
		return NULL;
	}
	return newList;
}

abcResult_e abcList_c::copyOut(abcList_c *targetOfCopy)
{
	OBJ_SIZE_CHECK(abcList_c,272);

	// first handle the abcListNode_c copyOut.
	// this will bring across the search key in the event
	// that it has been initialized
	abcResult_e ncoResult = abcListNode_c::copyOut(targetOfCopy);
	CHECK_ERROR(ncoResult,ABC_REASON_LIST_CLONE_FAILED,ncoResult);

	if (name)
	{
		int len= strlen(name) + 10;
		char cloneName[len];
		snprintf(cloneName,len,"%s[cl]",name);
		targetOfCopy->name = strdup(cloneName);
	}

	abcListNode_c *walkNode = headNode;
	while (walkNode != NULL)
	{
		#ifdef _LIST_RIGOROUS_TESTING_		// check owner value when defined
		CHECK_ERROR((walkNode->owner != this),ABC_REASON_NODE_NOT_OWNED,ABC_FAIL);
		#endif

		abcListNode_c *copyNode = walkNode->clone();
		CHECK_ERROR((copyNode == NULL),ABC_REASON_NODE_CLONE_FAILED,ABC_FAIL);
		
		abcResult_e res = targetOfCopy->addTail(copyNode);
		CHECK_ERROR(res,ABC_REASON_NODE_CLONE_FAILED,ABC_FAIL);

		walkNode = (abcListNode_c *)walkNode->next();	// next returns an abcNode_c*
	}

	// handling cloning the locks.
	targetOfCopy->lockingEnabled = FALSE;
	targetOfCopy->writerIsWaiting = FALSE;
	targetOfCopy->readerIsWaiting = FALSE;
	targetOfCopy->registeredReaders = 0;
	if (lockingEnabled)
	{
		abcResult_e res =targetOfCopy->enableLocking();
		CHECK_ERROR(res,ABC_REASON_MUTEX_INIT_FAILED,ABC_FAIL);
	}
	targetOfCopy->isSorted = isSorted;

	
	return ABC_PASS;
}

// take 2 lists and build a third which is all the elements common to both "this" and "other" lists
// remove common nodes from noth source lists.
// result has nodes unique to this on this + nodes unique to other on other.  and items common on the common list.
abcList_c *abcList_c::extractCommonList(class abcList_c *otherList)
{
	TRACE_NONIMPL("abcList_c");
	return NULL;
}

/**********************************************************************/
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// abcSlicedList_c  instantiated here
//
////////////////////////////////////////////////////////////////////////
/**********************************************************************/
//
// lifecyle stuff
//
abcSlicedList_c::abcSlicedList_c(const char *setName)
{
	if (setName)
	{
		name = strdup(setName);
	}
	sliceCount = 0;
	sliceArray = NULL;

}
abcSlicedList_c::~abcSlicedList_c()
{
	if (name)
	{
		free(name);
		name = NULL;
	}
}

// must set the number of slices being used
// this size is fixed for the life of the object.
// hashList is different
abcResult_e abcSlicedList_c::init(int setSliceCount)
{
	CHECK_ERROR((sliceCount != 0),ABC_REASON_LIST_ALREADY_INITIALIZED,ABC_FAIL);
	CHECK_ERROR((setSliceCount <2) || (setSliceCount > _LIST_MAX_SLICE_COUNT_),ABC_REASON_LIST_BAD_SLICE_COUNT,ABC_FAIL);
	sliceCount = setSliceCount;
	// FIXME... use our fully tracked storage
	sliceArray = (abcListSlice_s *)calloc(setSliceCount, sizeof(abcListSlice_s));
	return ABC_PASS;
}
//
// print stuff
//
char *abcSlicedList_c::getObjType()
{
	return (char *)"abcSlicedList_c";
}

//  printDepth=depth of "has a" membe to drill into , printStyle=style to print
abcResult_e abcSlicedList_c::print(abcPrintStyle_e printStyle)
{
	char pBuff[256];
	printBuff(pBuff,256,printStyle);
	fprintf(stderr,"%s\n",pBuff);
	int slice;
	for (slice=0;slice<sliceCount;slice++)
	{
		if (sliceArray[slice].nodeCount)
		{
			fprintf(stderr,"Slice%d: Head:%16p Tail:%16p SliceNodeCount:%lld\n",slice,sliceArray[slice].headNode,
				sliceArray[slice].tailNode,sliceArray[slice].nodeCount);	
		}
	}

	abcListNode_c *walkNode = headNode;
	while (walkNode != NULL)
	{
		walkNode->print(printStyle);
		walkNode = (abcListNode_c *)walkNode->next();
	}
	return ABC_PASS;
}

//nonVirtual prinfbuff for this level of the object hiearchy
abcResult_e abcSlicedList_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char baseBuff[128];
	abcResult_e result;
	result = abcList_c::printBuff(baseBuff,128,printStyle);
	snprintf(pBuff,pbuffSize,"%s SliceCount: %lld",baseBuff,(int64_t)sliceCount);
	return ABC_PASS;
}



/////////////////////////////
// full list operations
////////////
abcResult_e abcSlicedList_c::empty()
{
	abcListNode_c *tmpNode = NULL;
	abcListNode_c *walkNode = headNode;
#ifdef _LIST_RIGOROUS_TESTING_
	int sliceNumBeingEmptied = (walkNode != NULL) ? walkNode->sliceIndex : 0;
	CHECK_ERROR((sliceNumBeingEmptied <0) || (sliceNumBeingEmptied >= sliceCount),ABC_REASON_LIST_INVALID_SLICE_NUMBER,ABC_ERROR);
	abcListSlice_s *sliceBeingEmptied = &sliceArray[sliceNumBeingEmptied];
#endif
	while (walkNode)
	{
		tmpNode = next();

#ifdef _LIST_RIGOROUS_TESTING_
		walkNode->nextNode = NULL;
		walkNode->prevNode = NULL;
		walkNode->owner = NULL;

		int walkSliceNum=walkNode->sliceIndex;
		CHECK_ERROR((walkSliceNum < 0) || (walkSliceNum >= sliceCount),ABC_REASON_LIST_INVALID_SLICE_NUMBER,ABC_FAIL);
#endif
		if (walkSliceNum != sliceNumBeingEmptied )
		{
			// sliceNum changed.... sliceBeingEmptied should be empty now
#ifdef _LIST_RIGOROUS_TESTING_
			if (sliceBeingEmptied->nodeCount != 0)
			{
				// bad slice accounting
				char errStr[128];
				snprintf(errStr,128,"Slice %d should be empty but is not",sliceNumBeingEmptied);
				PRINT(errStr);
				ERROR(ABC_REASON_LIST_BAD_SLICE_ACCOUNTING);
				return ABC_FAIL;
			}
#else
			sliceBeingEmptied->nodeCount = 0;
#endif
			sliceBeingEmptied->headNode = NULL;
			sliceBeingEmptied->tailNode = NULL;

			// start on next slice immediately
			sliceNumBeingEmptied = walkSliceNum;
			sliceBeingEmptied = &sliceArray[walkSliceNum];
		}
#ifdef _LIST_RIGOROUS_TESTING_
		sliceBeingEmptied->nodeCount--;	// individual=lslice accounting
		nodeCount--;	// whole list (base class) accounting 
#endif
		delete walkNode;
		walkNode = tmpNode;
	}
	// finish zeroing out last slice
#ifdef _LIST_RIGOROUS_TESTING_
	if (sliceBeingEmptied->nodeCount != 0)
	{
		// bad slice accounting
		char errStr[128];
		snprintf(errStr,128,"Slice %d should be empty but is not",sliceNumBeingEmptied);
		PRINT(errStr);
		ERROR(ABC_REASON_LIST_BAD_SLICE_ACCOUNTING);
		return ABC_FAIL;
	}
#else
	sliceBeingEmptied->nodeCount = 0;
#endif
	// list should be empty
	headNode = tailNode = NULL;
	isSorted = FALSE;

#ifdef _LIST_RIGOROUS_TESTING_
	if (!nodeCount)
	{
		ERROR(ABC_REASON_LIST_BAD_NODE_COUNT);
		nodeCount = 0;
		return ABC_FAIL;
	}
#else
	nodeCount = 0;
#endif
	return ABC_PASS;
}

abcResult_e abcSlicedList_c::diff(class abcList_c *otherList)
{
	return ABC_EQUAL;
}

// (virtual) make a copy of this list
abcList_c *abcSlicedList_c::clone()
{
	// the list has a name but this nodes typically don't
	abcSlicedList_c *newList = new abcSlicedList_c();
	abcResult_e cloneResult = abcSlicedList_c::copyOut(newList);
	if (cloneResult != ABC_PASS)
	{
		delete newList;
		ERROR(ABC_REASON_LIST_CLONE_FAILED);
		return NULL;
	}
	return newList;
}
// come here to copy the contents of the freshly allocated list object
abcResult_e abcSlicedList_c::copyOut(abcSlicedList_c *targetOfCopy)
{
	OBJ_SIZE_CHECK(abcSlicedList_c,288);

	// we won't use abcList_c::copyOut to copy all of the nodes.
	// it will cause a problem with slice handlng so we'll do it manually here

	// but we will use abcListNode_c's copyOut
	abcResult_e res = abcListNode_c::copyOut(targetOfCopy);
	CHECK_ERROR(res,ABC_REASON_LIST_CLONE_FAILED,ABC_FAIL);

	// now do abcList_c's copyOut work
	if (name)
	{
		int len= strlen(name) + 5;
		char cloneName[len];
		snprintf(cloneName,len,"%s[cl]",name);
		targetOfCopy->name = strdup(cloneName);
	}
	// won't copu nodeCount,headNode,tailNode,currNode.
	targetOfCopy->lockingEnabled = FALSE;
	targetOfCopy->writerIsWaiting = FALSE;
	targetOfCopy->readerIsWaiting = FALSE;
	targetOfCopy->registeredReaders = 0;
	if (lockingEnabled)
	{
		abcResult_e res = targetOfCopy->enableLocking();
		CHECK_ERROR(res,ABC_REASON_LIST_CLONE_FAILED,ABC_FAIL);
	}
	targetOfCopy->isSorted = isSorted;

	// need to build the slice array now so we can use it
	targetOfCopy->sliceCount = sliceCount;
	targetOfCopy->sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));	// buid it zeroed
	

	// walk through the listNodes, copy eacn one and put it on the other list
	abcListNode_c *walkNode = headNode;
	while (walkNode != NULL)
	{
		#ifdef _LIST_RIGOROUS_TESTING_		// check owner value when defined
		CHECK_ERROR((walkNode->owner != this),ABC_REASON_NODE_NOT_OWNED,ABC_FAIL);
		#endif

		abcListNode_c *copyNode = walkNode->clone();
		CHECK_ERROR(!copyNode,ABC_REASON_NODE_CLONE_FAILED,ABC_FAIL);
		
		// rebuild the new list by letting it build the slice array 
		targetOfCopy->abcSlicedList_c::addSliceTail(copyNode->sliceIndex,copyNode);
		walkNode = (abcListNode_c *)walkNode->next();	// next returns an abcNode_c*
	}
	// our work is already done (just sliceCount and sliceArray

	return ABC_PASS;
}

// make a third list which is all the commom nodes.  remove common nodes from both lists. (using diffNode not diffKey)
abcList_c *extractCommonList(class abcList_c *otherList)
{
	TRACE_NONIMPL("abcSlicedList_c");
	return NULL;
}


// these really can't work correclty for a abcSlicedList

// so their overload will return an error
abcResult_e		abcSlicedList_c::addHead(abcListNode_c *nodeToAdd)
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
// add a new node at head and update head.  Do not update current.
abcResult_e		abcSlicedList_c::addTail(abcListNode_c *nodeToAdd)
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}

// add a new node at tail and update tail.  Do not update currenty
// insert new node after specified noed
abcResult_e		abcSlicedList_c::addMiddle(abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow )
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
// can't do an insert on a slicedList.   not publiclly anyway.
abcResult_e abcSlicedList_c::insert(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcResult_e abcSlicedList_c::insertPriv(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}



//  remove specifified node
//  same as base class remove except that we need to do slice accounting
abcResult_e		abcSlicedList_c::remove(abcListNode_c *nodeToRemove)
{
#ifdef _LIST_RIGOROUS_TESTING_
	CHECK_ERROR(!nodeToRemove,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);

	int sliceNum = nodeToRemove->sliceIndex;
	CHECK_ERROR(((sliceNum < 0 ) || (sliceNum >= sliceCount)),ABC_REASON_LIST_INVALID_SLICE_NUMBER,ABC_FAIL);
#else
	int sliceNum = nodeToRemove->sliceIndex;
#endif

	// manage the sliceArray helper data
	abcListSlice_s *actionSlice = &sliceArray[sliceNum];
	if (nodeToRemove == actionSlice->headNode)
	{
		actionSlice->headNode = (abcListNode_c *)nodeToRemove->nextNode;
	}
	if (nodeToRemove == actionSlice->tailNode)
	{
		actionSlice->tailNode = (abcListNode_c *)nodeToRemove->prevNode;
	}
	actionSlice->nodeCount--;

	// use baseClass remove to handle the actual list.
	// will check owner and baseClass nodeCount and head/tail
	return abcList_c::remove(nodeToRemove);
}
// ::pullHead and ::pullTail can work out of the baseclass b
// because remove is virtual and overloaded, the bass class function will work correctly (it uses remove)
// abcListNode_c	*abcSlicedList_c::pullHead()
// abcListNode_c	*abcSlicedList_c::pullTail()



// simple linear sorting =  can't work correctly
abcResult_e 	abcSlicedList_c::addSorted(abcListNode_c *nodeToAdd, const uint8_t towardsNext )
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}

// search stateless only - works directly from the baseclass
//abcListNode_c	*abcSlicedList_c::findFirst(nodeKey_s *searchKey,uint8_t towardsNext) { return ABC_PASS; }
//abcListNode_c	*abcSlicedList_c::findNext(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext) { return ABC_PASS; }
//abcListNode_c	*abcSlicedList_c::findActual(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext) { return ABC_PASS; }

// *****************************************************************************************************************
//
// unique signature slice versions of the base methods
//
// *****************************************************************************************************************

abcListNode_c *abcSlicedList_c::getSliceHead(int64_t sliceNum,abcResult_e *resultOut)
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;
	}
#endif
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];
	if (resultOut) *resultOut = ABC_PASS;
	return sliceRec->headNode;
	
}
abcListNode_c			*abcSlicedList_c::getSliceTail(int64_t sliceNum, abcResult_e *resultOut)
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;
	}
#endif
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];
	if (resultOut) *resultOut = ABC_PASS;
	return sliceRec->tailNode;
}
int64_t					abcSlicedList_c::getSliceNodeCount(int64_t sliceNum, abcResult_e *resultOut)
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		if (resultOut) *resultOut = ABC_FAIL;
		return 0;
	}
#endif
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];
	if (resultOut) *resultOut = ABC_PASS;
	return sliceRec->nodeCount;
}

// some concern here.... about the API... should we stuff the slice reference into the node before calling here, or pass it in here and then push it into the node from here
// currently we are passing it in and then this routine writes it into the node record.
// this is more consistent with how we'll want the hashList to work... since Hashlist will not have an easyily creaded hashIndex
abcResult_e				abcSlicedList_c::addSliceHead(int64_t sliceNum, abcListNode_c *nodeToAdd) // add a new node at head and update head.  Do not update current.
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return ABC_FAIL;
	}
	if (!nodeToAdd)
	{
		ERROR(ABC_REASON_NODE_PARAM_IS_NULL); //nodeToAdd can't be null
		return ABC_FAIL;
	}
	if (nodeToAdd->owner != NULL)
	{
		ERROR(ABC_REASON_NODE_OWNER_NOT_NULL); // node must not be on a list
		return ABC_FAIL;
	}
#endif
	nodeToAdd->sliceIndex = sliceNum;		// install the actual sliceNum into the record.
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];

	if (sliceRec->nodeCount == 0)
	{
		// adding first node to this slice.   is becomes the  head and the tail
		sliceRec->headNode = nodeToAdd;
		sliceRec->tailNode = nodeToAdd;
		sliceRec->nodeCount = 1;

		// we'll put this new slice at the tail of the existing linear (baseclass) list
		// if this node is NULL, then we'd better have an empty lst.
		nodeToAdd->sliceIndex = sliceNum;				// install the actual sliceNum into the record.
		return abcList_c::insertPriv(nodeToAdd,tailNode)	;// insert nodeToAdd immediately after the tail node of the base list.   the whole slice will go there
	}
	else
	{
		// adding node (before) the head of an existing slice
		abcListNode_c *oldSliceHead = sliceRec->headNode;
		abcListNode_c *nodeToInsertAfter = (abcList_c *)oldSliceHead->prevNode;
		sliceRec->headNode = nodeToAdd;
		sliceRec->nodeCount++;

		// adding to the head of a slice is easy until the slice is first on the base list. Then its a special case.
		if (nodeToInsertAfter == NULL)
		{
			// we're at the head of the base list
			return addHeadPriv(nodeToAdd);
		}
		else
		{
			return abcList_c::insertPriv(nodeToAdd,nodeToInsertAfter);
		}
	}
	// can't get here
}
abcResult_e	abcSlicedList_c::addSliceTail(int64_t sliceNum, abcListNode_c *nodeToAdd) // add a new node at tail and update tail.  Do not update currenty
{
#ifdef _LIST_RIGOROUS_TESTING_
	CHECK_ERROR(((sliceNum < 0 ) || (sliceNum >= sliceCount)),ABC_REASON_LIST_INVALID_SLICE_NUMBER,ABC_FAIL);
	CHECK_ERROR(!nodeToAdd,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR((nodeToAdd->owner != NULL),ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);
#endif
	nodeToAdd->sliceIndex = sliceNum;		// install the actual sliceNum into the record.
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];

	if (sliceRec->nodeCount == 0)
	{
		// adding first node to this slice.   is becomes the  head and the tail
		sliceRec->headNode = nodeToAdd;
		sliceRec->tailNode = nodeToAdd;
		sliceRec->nodeCount = 1;

		// we'll put this new slice at the tail of the existing linear (baseclass) list
		// if this node is NULL, then we'd better have an empty lst.
		return abcList_c::insertPriv(nodeToAdd,tailNode)	;// insert nodeToAdd immediately after the tail node of the base list.   the whole slice will go there
	}
	else
	{
		// adding node (after) the tail of an existing slice
		abcListNode_c *oldSliceTail = sliceRec->tailNode;
		sliceRec->tailNode = nodeToAdd;
		sliceRec->nodeCount++;

		// even if base list is empty, this will work with nodeToInsertAfter being NULL
		return abcList_c::insertPriv(nodeToAdd,oldSliceTail);
	}
	// can't get here
}

abcResult_e				abcSlicedList_c::addSliceMiddle(int64_t sliceNum, abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow ) // insert new node after specified node
{
	if (nodeToFollow == NULL)
	{
		// then we must be adding at the head of the slice
		return addSliceHead(sliceNum, nodeToAdd);
	}

#ifdef _LIST_RIGOROUS_TESTING_
	CHECK_ERROR(((sliceNum < 0 ) || (sliceNum >= sliceCount)),ABC_REASON_LIST_INVALID_SLICE_NUMBER,ABC_FAIL);
	CHECK_ERROR(!nodeToAdd,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
	CHECK_ERROR((nodeToAdd->owner != NULL),ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);
#endif

	// if nodeToFollow is null, then we're inserting at the the head.
	// if the list is empty, then we must be inserting at the head (and the tail, and the middle to !)
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];

#ifdef _LIST_RIGOROUS_TESTING_
	// lets verify that the sliceNum matches the nodeToFollow's sliceNum
	CHECK_ERROR((sliceNum != nodeToFollow->sliceIndex),ABC_REASON_LIST_NODE_SLICE_MISMATCH,ABC_FAIL);
#endif
	if (nodeToFollow == sliceRec->tailNode)
	{
		return addSliceTail(sliceNum, nodeToAdd);
	}
	// we're doing an insert in the middle of a slice
	// that is as long as we actually have a slice
	CHECK_ERROR(sliceRec->nodeCount == 0,ABC_REASON_LIST_BAD_SLICE_ACCOUNTING,ABC_FAIL); // / this is an error because nodeToFollow should have been NULL which as a case handled above.

	sliceRec->nodeCount++;
	nodeToAdd->sliceIndex = sliceNum;
	return abcList_c::insertPriv(nodeToAdd,nodeToFollow);
}

abcListNode_c   *abcSlicedList_c::pullSliceHead(int64_t sliceNum)
{
	abcListNode_c *returnVal = getSliceHead(sliceNum);
	if (returnVal)
	{
		remove(returnVal);
	}
	return returnVal;
}
abcListNode_c   *abcSlicedList_c::pullSliceTail(int64_t sliceNum)
{
	abcListNode_c *returnVal = getSliceTail(sliceNum);
	if (returnVal)
	{
		remove(returnVal);
	}
	return returnVal;
}

abcListNode_c		*abcSlicedList_c::findSliceFirst(int64_t sliceNum, nodeKey_s *searchKey,uint8_t towardsNext, abcResult_e *resultOut) // towardsNext controls search direction   towardsNext==1 starts from head.  ==0 starts from tail
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;
	}
#endif
	if (resultOut) *resultOut = ABC_PASS;
	// get start pointer for this slice
	abcListSlice_s *mySlice = &sliceArray[sliceNum];
	abcListNode_c *startNode = towardsNext ? mySlice->headNode : mySlice->tailNode;
	if (startNode == NULL)
	{
		// ran to end
		return NULL;
	}
	return abcSlicedList_c::findSliceActual(sliceNum,searchKey,startNode,towardsNext,resultOut);
}

abcListNode_c		*abcSlicedList_c::findSliceNext(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *previouslyFoundNode, uint8_t towardsNext, abcResult_e *resultOut) // doesn't re-examine startnode... move right(next) if towardsNext==1.
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;
	}
#endif
	if (!previouslyFoundNode)
	{
		ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;	// an error, but not fatal
	}
	if (resultOut) *resultOut = ABC_PASS;
	
	// move past the node we previously found and returned to the user
	abcListNode_c *startNode = (abcListNode_c *)(towardsNext ? previouslyFoundNode->next() : previouslyFoundNode->prev());

	// check for end of search
	if (startNode == NULL)
	{
		// ran to end
		return NULL;
	}

	return abcSlicedList_c::findSliceActual(sliceNum,searchKey,startNode,towardsNext);
}
abcListNode_c		*abcSlicedList_c::findSliceActual(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext, abcResult_e *resultOut) // doesn't re-examine startnode... move right(next) if towardsNext==1.
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;
	}
	if (sliceNum != startNode->sliceIndex)
	{
		// sanity test failed
		ERROR(ABC_REASON_NODE_BAD_LINK_STRUCTURE);
		if (resultOut) *resultOut = ABC_FAIL;
		return NULL;
	}
#endif
	
	// startNode already validated as non-null
	abcListNode_c *walkNode = startNode;
	while (walkNode != NULL)
	{
		if (walkNode->sliceIndex != sliceNum)
		{
			// we're pasted into the next slice... terminate
			if (resultOut) *resultOut = ABC_PASS;
			return NULL;
		}

		// read the question as "is the external key >= this node's key
		// if so, just past this node is the pace to do an insert
		abcResult_e diffResult = walkNode->diffKey(searchKey);
		if (diffResult ==  ABC_EQUAL)
		{
			// gone past it
			// if returnNode is NULL means we're before the startpoint
			if (resultOut) *resultOut = ABC_PASS;
			return walkNode;
		}
		if ((diffResult == ABC_ERROR) || (diffResult == ABC_FAIL))
		{
			ERROR(ABC_REASON_NODE_KEY_TYPE_INVALID);
			if (resultOut) *resultOut = ABC_FAIL;
			return NULL;
		}
		walkNode = (abcListNode_c *)(towardsNext ? walkNode->next() : walkNode->prev());
	}
	if (resultOut) *resultOut = ABC_PASS;
	return NULL; // didn't find a match... we ran off the end of the list
} // end abcSlicedList_c::findSliceActual()

abcList_c *abcSlicedList_c::extractCommonList(class abcList_c *otherList)
{
	TRACE_NONIMPL("abcSlicedList_c");
	return NULL;
}

/**********************************************************************/
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// abcHashList_c  instantiated here
//
////////////////////////////////////////////////////////////////////////
/**********************************************************************/
//
// lifecyle stuff
//
abcHashList_c::abcHashList_c(const char *setName)
{
	if (setName)
	{
		name = strdup(setName);
	}
	abcHashList_c::initProtected(100,5, 220); // slices, resizeThreshold, grownthPrecentage
}
abcHashList_c::~abcHashList_c()
{
	if (name)
	{
		free(name);
		name = NULL;
	}
}

// must set the number of slices being used
// this size is fixed for the life of the object.
// hashList is different

// overloaded to fail... 
abcResult_e abcHashList_c::init(int sliceCount)
{
	return ABC_FAIL;
}
// hashList init sets up parameters for automatic growth [ which is not currently implemented ]
abcResult_e abcHashList_c::initProtected(int startingSliceCount,int missThresholdForResize, int growthPercent)
{
	CHECK_ERROR((sliceCount != 0 && nodeCount > 0),ABC_REASON_LIST_ALREADY_INITIALIZED,ABC_FAIL);
	CHECK_ERROR((startingSliceCount <2) || (startingSliceCount > _LIST_MAX_SLICE_COUNT_),ABC_REASON_LIST_BAD_SLICE_COUNT,ABC_FAIL);

	if (sliceArray)
	{
		free(sliceArray);
		sliceArray = NULL;
	}

	sliceCount = globalCore->findPrime(startingSliceCount);	// always use a prime sized array for better/even spread of indexes
	sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));
	resizeHashBucketThreshold = missThresholdForResize;
	resizeHashGrowthPercentage = growthPercent;

	return ABC_PASS;
}
//
// print stuff
//
char *abcHashList_c::getObjType()
{
	return (char *)"abcHashList_c";
}

//  printDepth=depth of "has a" membe to drill into , printStyle=style to print
abcResult_e abcHashList_c::print(abcPrintStyle_e printStyle )
{
	abcSlicedList_c::print(printStyle);
	return ABC_PASS;
}

//nonVirtual prinfbuff for this level of the object hiearchy
abcResult_e abcHashList_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	TRACE_NONIMPL("abcHashList_c");
	return ABC_PASS;
}

//full abcHashList_c list operations
//virtual abcResult_e empty();									// SlicedList empty used
//virtual abcResult_e diff(class abcList_c *otherList);			// SlicedLlist diff used

abcList_c *abcHashList_c::clone()
{
	// the list has a name but this nodes typically don't
	abcHashList_c *newList = new abcHashList_c();
	abcResult_e cloneResult = abcHashList_c::copyOut(newList);
	if (cloneResult != ABC_PASS)
	{
		delete newList;
		ERROR(ABC_REASON_LIST_CLONE_FAILED);
		return NULL;
	}
	return newList;
}

abcResult_e abcHashList_c::copyOut(abcHashList_c *targetOfCopy)
{
	OBJ_SIZE_CHECK(abcHashList_c,304)

	// but we will use abcListNode_c's copyOut
	abcResult_e coResult = abcSlicedList_c::copyOut(targetOfCopy);
	CHECK_ERROR(coResult,ABC_REASON_NODE_CLONE_FAILED,coResult);

	targetOfCopy->specialInit = specialInit;
	targetOfCopy->startingSliceCount = startingSliceCount;
	targetOfCopy->startingResizeThreshold = startingResizeThreshold;
	targetOfCopy->resizeHashBucketThreshold = resizeHashBucketThreshold;
	targetOfCopy->resizeHashGrowthPercentage = resizeHashGrowthPercentage;

	return ABC_PASS;
}

//virtual abcList_c *extractCommonList(class abcList_c *otherList);	// SlicedList extractCommonList used

// *******************************************  
// stateful operation  ... just these will remain stateful.  All the rest are stateless (do not effect or use currNode variable)
//abcListNode_c		*head();		// get and set currNode <= headNode
//abcListNode_c		*tail();		// get and set currNode <= tailNode
//abcListNode_c		*next();		// get and set currNode <= currNode->nextNode
//abcListNode_c		*prev();		// get and set currNode <= currNode->prevNode
//abcListNode_c		*getCurr();

// stateless operation
//abcListNode_c		*getHead();
//abcListNode_c		*getTail();	
//int64_t			 getNodeCount();					// get count of nodes on list.

// for abcHashList_c these really can't work correclty for a abcHashList
// so their overload will return an error
//virtual abcResult_e		addHead(abcListNode_c *nodeToAdd);							 			// overloaded to fail in abcSlicedList_c
//virtual abcResult_e		addTail(abcListNode_c *nodeToAdd);							 			// overloaded to fail in abcSlicedList_c
//virtual abcResult_e		addMiddle(abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow );		// overloaded to fail in abcSlicedList_c
//virtual abcResult_e		insert(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow);		// overloaded to fail in abcSlicedList_c
//virtual abcResult_e		insertPriv(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow);	// overloaded to fail in abcSlicedList_c

// virtual abcResult_e		remove(abcListNode_c *nodeToRemove);									// overloaded and workds from abcSlicedList_c

// these can work straight from the baseclass
//virtual abcListNode_c	*pullHead();															// baseclass works as needed because remove is overloaded to work with abcSlicedList_c
//virtual abcListNode_c	*pullTail();															// baseclass works as needed because remove is overloaded to work with abcSlicedList_c

//virtual abcResult_e 	addSorted(abcListNode_c *nodeToAdd, const uint8_t towardsNext ); 		// overloaded to fail in abcSlicedList_c

// Hashlist FindFirst.
// 

abcListNode_c	*abcHashList_c::findFirst(nodeKey_s *searchKey,uint8_t towardsNext, abcResult_e *resultOut)
{
	uint64_t keyHash = abcListNode_c::calcKeyHash(searchKey,resultOut);
	if (resultOut && *resultOut != ABC_PASS) return NULL;
	int64_t  keyIndex = ((int64_t)keyHash) % sliceCount;
	keyIndex = keyIndex < 0 ? keyIndex+ sliceCount : keyIndex;	// handle negative keyHash result
	return abcSlicedList_c::findSliceFirst(keyIndex, searchKey,towardsNext,resultOut);
}

abcListNode_c	*abcHashList_c::findNext(nodeKey_s *searchKey, abcListNode_c *startNode,uint8_t towardsNext, abcResult_e *resultOut)
{
	int64_t  keyIndex = startNode->sliceIndex;
	return abcSlicedList_c::findSliceNext(keyIndex, searchKey, startNode, towardsNext,resultOut);
} // end abcHashList_c::findNext()

abcListNode_c	*abcHashList_c::getSliceHead(int64_t sliceNum, abcResult_e *resultOut)
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	if (resultOut) *resultOut = ABC_FAIL;
	return NULL;
}
abcListNode_c	*abcHashList_c::getSliceTail(int64_t sliceNum, abcResult_e *resultOut) // disabled
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	if (resultOut) *resultOut = ABC_FAIL;
	return NULL;
}
int64_t			abcHashList_c::getSliceNodeCount(int64_t sliceNum, abcResult_e *resultOut)
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	if (resultOut) *resultOut = ABC_FAIL;
	return -1;
}
abcResult_e		abcHashList_c::addSliceHead(int64_t sliceNum, abcListNode_c *nodeToAdd)
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcResult_e		abcHashList_c::addSliceTail(int64_t sliceNum, abcListNode_c *nodeToAdd)
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcResult_e		abcHashList_c::addSliceMiddle(int64_t sliceNum, abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow )
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcListNode_c   *abcHashList_c::pullSliceHead(int64_t sliceNum)
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;;
}
abcListNode_c   *abcHashList_c::pullSliceTail(int64_t sliceNum)
{
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;;
}
abcListNode_c	*abcHashList_c::findSliceFirst(int64_t sliceNum, nodeKey_s *searchKey,uint8_t towardsNext, abcResult_e *resultOut)
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	if (resultOut) *resultOut = ABC_FAIL;
	return NULL;
}
abcListNode_c	*abcHashList_c::findSliceNext(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext, abcResult_e *resultOut)
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	if (resultOut) *resultOut = ABC_FAIL;
	return NULL;
}
abcListNode_c	*abcHashList_c::findSliceActual(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext, abcResult_e *resultOut) // disabled
{ // disabled
	ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	if (resultOut) *resultOut = ABC_FAIL;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
////////////////////////// ---   abcHashList_c   --- /////////////////////
//////////////////////////////////// unique //////////////////////////////


// in abcHashList_c::add we use the nodeKey to determine a hash bucket
// from there this is just like an abcSlicedList_c::addSliceTail().  In fact we call it direcly.
// please note that just like SlicedList, we do not expect the slices to appear in order on the base list.
// we do expect the whole slice to sit together, but new slices are added to the tail of the base list, so
// the position will vary based on when a slice is first used.  With a HashedList, we use the slices somewhat randomly.
//
abcResult_e abcHashList_c::add(abcListNode_c *nodeToAdd)	// MUST have the key already properly initialized
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	CHECK_ERROR(!nodeToAdd,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);

	// confirm there is no owner of the new node
	CHECK_ERROR(nodeToAdd->owner != NULL,ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);

	// validate there are slice
	CHECK_ERROR(sliceCount == 0,ABC_REASON_NODE_OWNER_NOT_NULL,ABC_FAIL);
#endif
	
	// figure which slice to use for this hash
	abcResult_e res;	//  using an "out variable" for calcKeyHash
	uint64_t fullHash = nodeToAdd->calcKeyHash(&res);
	CHECK_ERROR(res,ABC_REASON_NODE_KEY_TYPE_NOT_INITIALIZED,ABC_FAIL);

	int64_t keyIndex = (int64_t)fullHash % sliceCount;	// modulo for slice index. possibly < 0
	keyIndex = (keyIndex < 0) ? keyIndex+sliceCount : keyIndex;  // index can't be negative
	
	abcListSlice_s *mySlice = &sliceArray[keyIndex];
	if (mySlice->nodeCount >= resizeHashBucketThreshold)
	{	
		res=resizeHashTable();
		CHECK_ERROR(res,ABC_REASON_LIST_HASH_RESIZE_FAILED,ABC_FAIL);
	}
	nodeToAdd->sliceIndex = keyIndex;
	res = abcSlicedList_c::addSliceTail(keyIndex, nodeToAdd);
	CHECK_ERROR(res,ABC_REASON_LIST_HASH_ADD_FAILED,ABC_FAIL);
	return ABC_PASS;
	
} // end abcHashList_c::add(abcListNode_c *nodeToAdd)	// MUST have the key already properly initialized

abcResult_e abcHashList_c::resizeHashTable()
{
	// come here to completely rebuild the hashLlist with a bigger hash array.  We'll do it by disconnecting the entire list from the anchoring
	// data structs.... effectively re-adding all nodes to an empty (larger) list structure.

	abcListNode_c *localTailNode = tailNode;
	int64_t localNodeCount = nodeCount;
	abcListSlice_s *localSliceArray = sliceArray;
	int				localSliceCount = sliceCount;
	DEBUG("Resizing params:  Threshold=%d  resizePrecentage=%d\n", resizeHashBucketThreshold,resizeHashGrowthPercentage);

	// figure new array Size.... we'll make it a prime number to optimize the distribution of hash keys
	// we'll add some internal knobs
	int64_t primeEst = ((int64_t)localSliceCount * ((100LL + (int64_t)resizeHashGrowthPercentage))) / 100LL;
	sliceCount = globalCore->findPrime(primeEst);
	sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));
	resizeHashBucketThreshold += 1;
	DEBUG("After resizing, new resizing params: SliceCount=%d Threshold=%d  resizePrecentage=%d\n", sliceCount, resizeHashBucketThreshold,resizeHashGrowthPercentage);

	sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));


	abcListNode_c *walkNode = headNode;
	abcListNode_c *freeNode;
	headNode = tailNode = NULL;
	nodeCount = 0;
	int loopCount = 0;
	while (walkNode)
	{
		// pullHead
		freeNode = walkNode;
		walkNode = (abcListNode_c *)walkNode->nextNode;
		freeNode->prevNode = NULL;
		freeNode->nextNode = NULL;
		freeNode->owner = NULL;

		// now add the freeNode.
		abcResult_e addResult = add(freeNode);
		CHECK_ERROR(addResult,ABC_REASON_LIST_HASH_RESIZE_FAILED,ABC_FAIL);

		loopCount++;
	}
	CHECK_ERROR((loopCount != localNodeCount),ABC_REASON_LIST_HASH_RESIZE_FAILED,ABC_FAIL);
	CHECK_ERROR((freeNode != localTailNode),ABC_REASON_LIST_HASH_RESIZE_FAILED,ABC_FAIL);

	// new list should be all set.  lets dump the old slice array
	free(localSliceArray);

	return ABC_PASS;
}


///////////////////
// EOF abcList.cpp
