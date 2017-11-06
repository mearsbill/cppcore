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

// There are global defines, but placed here to 
// allow quick checking of stuff without a massive recompile
#define _TRACE_METHOD_ENTRIES_
#define _TRACE_METHOD_EXITS_
#define _TRACE_CONSTRUCTION_
#define _TRACE_DESTRUCTION_
#define _TRACE_NONIMPL_
#define _PRINT_DEBUG_ERROR_
#define _PRINT_DEBUG_A_

#include "abcDebugMacros.h"
#include "abcCore.h"

abcList_c::abcList_c(const char *setName)
{
	name = setName ? strdup(setName) : NULL; 

	//TRACE_CONSTRUCT("abcList_c");
	headNode = NULL;
	tailNode = NULL;
	currNode = NULL;

	nodeCount=0;
	errorReason = ABC_REASON_NONE;
	isSorted = FALSE;
}
abcList_c::~abcList_c()
{
	TRACE_DESTROY("abcList_c");
	empty();	// this will delete all listNodes and their contents
	if (name)
	{
		free(name);
		name = NULL;
	}
	TRACE_EXIT("abcList_c");
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
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}
abcResult_e	abcList_c::disableLocking()
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}
abcResult_e	abcList_c::readRegister()
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}
abcResult_e	abcList_c::readRegisterNoWait()
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}
abcResult_e	abcList_c::readRelease()
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}
abcResult_e	abcList_c::writeLock()
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}
abcResult_e	abcList_c::writeLockNoWait()
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}
abcResult_e	abcList_c::writeRelease()
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}


// Error handling stuff & print stuff
void abcList_c::setErrorReason(abcReason_e reason)
{
	errorReason = reason;
}

abcReason_e abcList_c::getErrorReason()
{
	return errorReason;
}

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
	TRACE_ENTRY("abcList_c");
	currNode = headNode;
	return currNode;
}
abcListNode_c		*abcList_c::tail()
{
	TRACE_ENTRY("abcList_c");
	currNode = tailNode;
	return currNode;
}
abcListNode_c		*abcList_c::next()
{
	TRACE_ENTRY("abcList_c");
	if (!currNode) return NULL;
	currNode = (abcListNode_c *)currNode->next();
	return currNode;
}
abcListNode_c		*abcList_c::prev()
{
	TRACE_ENTRY("abcList_c");
	if (!currNode) return NULL;
	currNode = (abcListNode_c *)currNode->prev();
	return currNode;
}

abcListNode_c		*abcList_c::getCurr()
{
	TRACE_ENTRY("abcList_c");
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
	TRACE_ENTRY("abcList_c");
	return headNode;
}
abcListNode_c *abcList_c::getTail()
{
	TRACE_ENTRY("abcList_c");
	return tailNode;
}

int64_t abcList_c::getNodeCount()
{
	TRACE_ENTRY("abcList_c");
	return nodeCount;
}


abcResult_e abcList_c::addHead(abcListNode_c *nodeToAdd)
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	if (!nodeToAdd)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		return ABC_FAIL;
	}
	// confirm there is no owner of the new node
	if (nodeToAdd->owner)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL);
		return ABC_FAIL;
	}
	if (isSorted && (nodeCount > 0))
	{
		// list already has unsorted elements
		FATAL_ERROR(ABC_REASON_LIST_IS_SORTED);
	}
#endif
	return  abcList_c::addHeadPriv(nodeToAdd);
}
abcResult_e abcList_c::addHeadPriv(abcListNode_c *nodeToAdd)
{
	// everything pre-validated from here.
	// add additional checks to public routines or other callers
	TRACE_ENTRY("abcList_c");

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
	TRACE_EXIT("abcList_c");
	return ABC_PASS;
} // end abcList_c::addHeadPriv()

abcResult_e abcList_c::addTail(abcListNode_c *nodeToAdd)							 // add a new node at tail and update tail.  Do not update current
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	if (!nodeToAdd)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		return ABC_PASS;
	}
	// confirm there is no owner of the new node
	if (nodeToAdd->owner) {
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL);
		return ABC_PASS;
	}
	if (isSorted && nodeCount > 0)
	{
		// list already has unsorted elements
		FATAL_ERROR(ABC_REASON_LIST_IS_SORTED);
	}
#endif
	return abcList_c::addTailPriv(nodeToAdd);							 // add a new node at tail and update tail.  Do not update current
}
abcResult_e abcList_c::addTailPriv(abcListNode_c *nodeToAdd)							 // add a new node at tail and update tail.  Do not update current
{
	TRACE_ENTRY("abcList_c");
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
	TRACE_EXIT("abcList_c");
	return ABC_PASS;
} // end abcList_c::addTail()

//
// common routine for insertion... set state if indicated
//
abcResult_e abcList_c::addMiddle(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	if (!nodeToInsert)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		return ABC_FAIL;
	}
	// confirm there is no owner of the new node
	if (nodeToFollow->owner != this)
	{
		FATAL_ERROR(ABC_REASON_NODE_NOT_ON_LIST); // its on a different list
		return ABC_FAIL;
	}
	if (nodeToInsert->owner != NULL)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL); // its on a different list
		return ABC_FAIL;
	}
	if (isSorted && nodeCount > 0)
	{
		// list already has unsorted elements
		FATAL_ERROR(ABC_REASON_LIST_IS_SORTED);
	}
#endif
	return abcList_c::insertPriv(nodeToInsert, nodeToFollow);
}

abcResult_e abcList_c::insert(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	if (!nodeToInsert)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		return ABC_FAIL;
	}
	// confirm there is no owner of the new node
	if (nodeToFollow->owner != this)
	{
		FATAL_ERROR(ABC_REASON_NODE_NOT_ON_LIST); // its on a different list
		return ABC_FAIL;
	}
	if (nodeToInsert->owner != NULL)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL); // its on a different list
		return ABC_FAIL;
	}
	if (isSorted && nodeCount > 0)
	{
		// list already has unsorted elements
		FATAL_ERROR(ABC_REASON_LIST_IS_SORTED);
	}
#endif
	return abcList_c::insertPriv(nodeToInsert, nodeToFollow);
}
abcResult_e abcList_c::insertPriv(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
	TRACE_ENTRY("abcList_c");
	nodeToInsert->owner = this;
	if (nodeCount)
	{
		// list already exists
#ifdef _LIST_RIGOROUS_TESTING_
		// sanity test on the node being added
		if (!nodeToFollow)
		{
			FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL); // its on a different list
			return ABC_FAIL;
		}
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
		if (nodeToFollow)
		{
			FATAL_ERROR(ABC_REASON_NODE_PARAM_SHOULD_BE_NULL); // its on a different list
			return ABC_FAIL;
		}
#endif
		headNode = tailNode = nodeToInsert;
		nodeCount = 1;
	}
	TRACE_EXIT("abcList_c");
	return ABC_PASS;
} // end abcList_c::insertCommon()

abcResult_e abcList_c::remove(abcListNode_c *nodeToRemove)
{
	TRACE_ENTRY("abcList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	// sanity test on the node being added
	if (!nodeToRemove)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL); //nodeToRemove can't be null
		return ABC_FAIL;
	}
	// confirm there is no owner of the new node
	if (nodeToRemove->owner != this)
	{
		FATAL_ERROR(ABC_REASON_NODE_NOT_ON_LIST); // its on a different list
		return ABC_FAIL;
	}
#endif
	if (nodeCount < 2)
	{
#ifdef _LIST_RIGOROUS_TESTING_
		if (!nodeCount)
		{
			// nodeCount zero... no way to delete a node
			FATAL_ERROR(ABC_REASON_NODE_BAD_LINK_STRUCTURE);
			return ABC_FAIL;
		}
		// node shoud be both head and tail
		if (nodeToRemove != headNode)
		{
			FATAL_ERROR(ABC_REASON_NODE_BAD_LINK_STRUCTURE);
			return ABC_FAIL;
		}
		if (nodeToRemove != tailNode)
		{
			FATAL_ERROR(ABC_REASON_NODE_BAD_LINK_STRUCTURE);
			return ABC_FAIL;
		}
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
	TRACE_EXIT("abcList_c");
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
	if (!nodeToAdd)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		return ABC_FAIL;
	}
	if (nodeToAdd->owner != NULL)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL); // its on a different list
		return ABC_FAIL;
	}
	if (!isSorted && nodeCount > 0)
	{
		// list already has unsorted elements
		FATAL_ERROR(ABC_REASON_LIST_NOT_SORTED);
		return ABC_FAIL;
	}
#endif
	nodeToAdd->owner = this;
	isSorted = TRUE;
	nodeKey_s *searchKey = &(nodeToAdd->key);
	abcListNode_c *walkNode = towardsNext ? headNode : tailNode;
	abcListNode_c *lastWalkNode = NULL;
	while (walkNode)
	{
		abcResult_e diffResult = walkNode->diffKey(searchKey);
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
abcListNode_c *abcList_c::findFirst(nodeKey_s *searchKey,uint8_t towardsNext)
{
	// get start pointer
	abcListNode_c *startNode = towardsNext ? headNode : tailNode;
	if (startNode == NULL)
	{
		// ran to end
		return NULL;
	}
	return findActual(searchKey,startNode,towardsNext);
} // end abcList_c::findFirst()

abcListNode_c *abcList_c::findNext(nodeKey_s *searchKey, abcListNode_c *previouslyFoundNode, uint8_t towardsNext)
{
	if (!previouslyFoundNode)
	{
		return NULL;	// an error, but not fatal
	}
	
	// move past the node we previously found and returned to the user
	abcListNode_c *startNode = (abcListNode_c *)(towardsNext ? previouslyFoundNode->next() : previouslyFoundNode->prev());

	// check for end of search
	if (startNode == NULL)
	{
		// ran to end
		return NULL;
	}

	return findActual(searchKey,startNode,towardsNext);
} // end abcList_c::findNext()


// if null returned... means we've run off the end of the list.
// this is the place where real work is done.  First possible match is at "startNode"... continuing on in the
// desired dirction
abcListNode_c		*abcList_c::findActual(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext)
{
	// startNode already validated as non-null
	abcListNode_c *walkNode = startNode;
	while (walkNode != NULL)
	{
		// read the question as "is the external key >= this node's key
		// if so, just past this node is the pace to do an insert
		abcResult_e diffResult = walkNode->diffKey(searchKey);
		if (diffResult ==  ABC_EQUAL)
		{
			// gone past it
			// if returnNode is NULL means we're before the startpoint
			return walkNode;
		}
		walkNode = (abcListNode_c *)(towardsNext ? walkNode->next() : walkNode->prev());
	}
	return NULL; // didn't find a match... we ran off the end of the list
} // end abcList_c::findActual()

//
// ============  Full list operations =============
//
abcResult_e abcList_c::empty()
{
	TRACE_ENTRY("abcList_c");

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
		FATAL_ERROR(ABC_REASON_LIST_BAD_NODE_COUNT);
		nodeCount = 0;
		return ABC_FAIL;
	}
#else
	nodeCount = 0;
#endif
	TRACE_EXIT("abcList_c");
	return ABC_PASS;
}
abcResult_e abcList_c::diff(class abcList_c *otherList)
{
	TRACE_NONIMPL("abcList_c");
	return ABC_FAIL;
}

// make a new list and copy each abcListNode_c to it.
// this will use the virtual clone function of the abcListNode_c object
abcList_c *abcList_c::clone()
{
	TRACE_ENTRY("abcList_c");


	//  check to avoid object size change  disaster.
	#define APPROVED_SIZE 104
	#define SIZE_CHECK_OBJ abcList_c
	if (sizeof(SIZE_CHECK_OBJ) - APPROVED_SIZE)
	{
		char errStr[128];
		snprintf(errStr,128,"sizeof \"%s\" changed from %d to %d at %s:%d",STRINGIFY(SIZE_CHECK_OBJ),(int)APPROVED_SIZE,(int)sizeof(SIZE_CHECK_OBJ),__FILE__,__LINE__);
		PROD_ERROR(errStr);
		FATAL_ERROR(ABC_REASON_OBJECT_SIZE_WRONG);
		return NULL;
	}
	#undef APPROVED_SIZE
	#undef SIZE_CHECK_OBJ

	// the list has a name but this nodes typically don't
	abcList_c *newList = new abcList_c();
	abcResult_e cloneResult = abcList_c::copyOut(newList);
	if (cloneResult != ABC_PASS)
	{
		delete newList;
		FATAL_ERROR(ABC_REASON_LIST_CLONE_FAILED);
		return NULL;
	}
	TRACE_EXIT("abcList_c");
	return newList;
}

abcResult_e abcList_c::copyOut(abcList_c *targetOfCopy)
{
	// first handle the abcListNode_c copyOut.
	// this will bring across the search key in the event
	// that it has been initialized
	abcResult_e ncoResult = abcListNode_c::copyOut(targetOfCopy);
	if ( ncoResult != ABC_PASS)
	{
		return ncoResult;
	}

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
		if (walkNode->owner != this)
		{
			FATAL_ERROR(ABC_REASON_NODE_NOT_OWNED);
			return ABC_FAIL;
		}
		#endif

		abcListNode_c *copyNode = walkNode->clone();
		if (copyNode == NULL)
		{
			FATAL_ERROR(ABC_REASON_NODE_CLONE_FAILED);
			return ABC_FAIL;
		}
		
		targetOfCopy->addTail(copyNode);
		walkNode = (abcListNode_c *)walkNode->next();	// next returns an abcNode_c*
	}

	// TODO: all locks and other additions must find their way here.
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
	TRACE_ENTRY("abcSlicedList_c");
	if (sliceCount != 0)
	{
		FATAL_ERROR(ABC_REASON_LIST_ALREADY_INITIALIZED);
		return ABC_FAIL;
	}

	if ((setSliceCount <2) || (setSliceCount > _LIST_MAX_SLICE_COUNT_))
	{
		FATAL_ERROR(ABC_REASON_LIST_BAD_SLICE_COUNT);
		return ABC_FAIL;
	}
	sliceCount = setSliceCount;
	sliceArray = (abcListSlice_s *)calloc(setSliceCount, sizeof(abcListSlice_s));
	TRACE_EXIT("abcSlicedList_c");
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
	TRACE_ENTRY("abSlicedcList_c");

	abcListNode_c *tmpNode = NULL;
	abcListNode_c *walkNode = headNode;
#ifdef _LIST_RIGOROUS_TESTING_
	int sliceNumBeingEmptied = (walkNode != NULL) ? walkNode->sliceIndex : 0;
	if ((sliceNumBeingEmptied <0) || (sliceNumBeingEmptied >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return ABC_FAIL;
	}
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
		if ((walkSliceNum < 0) || (walkSliceNum >= sliceCount))
		{
			FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
			return ABC_FAIL;
		}
#endif
		if (walkSliceNum != sliceNumBeingEmptied )
		{
			// sliceNum changed.... sliceBeingEmptied should be empty now
#ifdef _LIST_RIGOROUS_TESTING_
			if (sliceBeingEmptied->nodeCount != 0)
			{
				// bad slice accounting
				char errStr[128];
				snprintf(errStr,128,"Slice %d should be empty but is not at %s::%d\n",sliceNumBeingEmptied,__FILE__,__LINE__);
				PROD_ERROR(errStr);
				FATAL_ERROR(ABC_REASON_LIST_BAD_SLICE_ACCOUNTING);
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
		snprintf(errStr,128,"Slice %d should be empty but is not at %s:%d\n",sliceNumBeingEmptied,__FILE__,__LINE__);
		PROD_ERROR(errStr);
		FATAL_ERROR(ABC_REASON_LIST_BAD_SLICE_ACCOUNTING);
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
		FATAL_ERROR(ABC_REASON_LIST_BAD_NODE_COUNT);
		nodeCount = 0;
		return ABC_FAIL;
	}
#else
	nodeCount = 0;
#endif
	TRACE_EXIT("abcSlicedList_c");
	return ABC_PASS;
}

abcResult_e abcSlicedList_c::diff(class abcList_c *otherList)
{
	return ABC_EQUAL;
}

// (virtual) make a copy of this list
abcList_c *abcSlicedList_c::clone()
{
	TRACE_ENTRY("abcSlicedList_c");

	//  check to avoid object size change  disaster.
	#define APPROVED_SIZE 120
	#define SIZE_CHECK_OBJ abcSlicedList_c
	if (sizeof(SIZE_CHECK_OBJ) - APPROVED_SIZE)
	{
		char errStr[128];
		snprintf(errStr,128,"sizeof \"%s\" changed from %d to %d at %s:%d",STRINGIFY(SIZE_CHECK_OBJ),(int)APPROVED_SIZE,(int)sizeof(SIZE_CHECK_OBJ),__FILE__,__LINE__);
		PROD_ERROR(errStr);
		FATAL_ERROR(ABC_REASON_OBJECT_SIZE_WRONG);
		return NULL;
	}
	#undef APPROVED_SIZE
	#undef SIZE_CHECK_OBJ

	// the list has a name but this nodes typically don't
	abcSlicedList_c *newList = new abcSlicedList_c();
	abcResult_e cloneResult = abcSlicedList_c::copyOut(newList);
	if (cloneResult != ABC_PASS)
	{
		delete newList;
		FATAL_ERROR(ABC_REASON_LIST_CLONE_FAILED);
		return NULL;
	}
	TRACE_EXIT("abcSlicedList_c");
	return newList;
}
// come here to copy the contents of the freshly allocated list object
abcResult_e abcSlicedList_c::copyOut(abcSlicedList_c *targetOfCopy)
{
	// we won't use abcList_c::copyOut to copy all of the nodes.
	// it will cause a problem with slice handlng so we'll do it manually here

	// but we will use abcListNode_c's copyOut
	abcResult_e coResult = abcListNode_c::copyOut(targetOfCopy);

	// now do abcList_c's copyOut work
	if (name)
	{
		int len= strlen(name) + 5;
		char cloneName[len];
		snprintf(cloneName,len,"%s[cl]",name);
		targetOfCopy->name = strdup(cloneName);
	}
	// won't copu nodeCount,headNode,tailNode,currNode.
	targetOfCopy->isSorted = isSorted;
	targetOfCopy->errorReason = errorReason;

	// need to build the slice array now so we can use it
	targetOfCopy->sliceCount = sliceCount;
	targetOfCopy->sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));	// buid it zeroed
	

	// walk through the listNodes, copy eacn one and put it on the other list
	abcListNode_c *walkNode = headNode;
	while (walkNode != NULL)
	{
		#ifdef _LIST_RIGOROUS_TESTING_		// check owner value when defined
		if (walkNode->owner != this)
		{
			FATAL_ERROR(ABC_REASON_NODE_NOT_OWNED);
			return ABC_FAIL;
		}
		#endif

		abcListNode_c *copyNode = walkNode->clone();
		if (copyNode == NULL)
		{
			FATAL_ERROR(ABC_REASON_NODE_CLONE_FAILED);
			return ABC_FAIL;
		}
		
		// rebuild the new list by letting it build the slice array 
		targetOfCopy->abcSlicedList_c::addSliceTail(copyNode->sliceIndex,copyNode);
		walkNode = (abcListNode_c *)walkNode->next();	// next returns an abcNode_c*
	}
	// our work is already done (just sliceCount and sliceArray

	TRACE_ENTRY("abcSlicedList_c");
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
	TRACE_ENTRY("abcSlicedList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
// add a new node at head and update head.  Do not update current.
abcResult_e		abcSlicedList_c::addTail(abcListNode_c *nodeToAdd)
{
	TRACE_ENTRY("abcSlicedList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}

// add a new node at tail and update tail.  Do not update currenty
// insert new node after specified noed
abcResult_e		abcSlicedList_c::addMiddle(abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow )
{
	TRACE_ENTRY("abcSlicedList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
// can't do an insert on a slicedList.   not publiclly anyway.
abcResult_e abcSlicedList_c::insert(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
	TRACE_ENTRY("abcSlicedList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcResult_e abcSlicedList_c::insertPriv(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow)
{
	TRACE_ENTRY("abcSlicedList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}



//  remove specifified node
//  same as base class remove except that we need to do slice accounting
abcResult_e		abcSlicedList_c::remove(abcListNode_c *nodeToRemove)
{
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if (!nodeToRemove)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		return ABC_FAIL;
	}
	// sliceNum is valid for both hashList and slicedList
	int sliceNum = nodeToRemove->sliceIndex;
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return ABC_FAIL;
	}
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
	TRACE_EXIT("abcSlicedList_c");
	return abcList_c::remove(nodeToRemove);
}
// ::pullHead and ::pullTail can work out of the baseclass b
// because remove is virtual and overloaded, the bass class function will work correctly (it uses remove)
// abcListNode_c	*abcSlicedList_c::pullHead()
// abcListNode_c	*abcSlicedList_c::pullTail()



// simple linear sorting =  can't work correctly
abcResult_e 	abcSlicedList_c::addSorted(abcListNode_c *nodeToAdd, const uint8_t towardsNext )
{
	TRACE_ENTRY("abcSlicedList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
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

abcListNode_c *abcSlicedList_c::getSliceHead(int64_t sliceNum)
{
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return NULL;
	}
#endif
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];
	return sliceRec->headNode;
	
}
abcListNode_c			*abcSlicedList_c::getSliceTail(int64_t sliceNum)
{
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return NULL;
	}
#endif
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];
	return sliceRec->tailNode;
}
int64_t					abcSlicedList_c::getSliceNodeCount(int64_t sliceNum)
{
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return 0;
	}
#endif
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];
	return sliceRec->nodeCount;
}

// some concern here.... about the API... should we stuff the slice reference into the node before calling here, or pass it in here and then push it into the node from here
// currently we are passing it in and then this routine writes it into the node record.
// this is more consistent with how we'll want the hashList to work... since Hashlist will not have an easyily creaded hashIndex
abcResult_e				abcSlicedList_c::addSliceHead(int64_t sliceNum, abcListNode_c *nodeToAdd) // add a new node at head and update head.  Do not update current.
{
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return ABC_FAIL;
	}
	if (!nodeToAdd)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL); //nodeToAdd can't be null
		return ABC_FAIL;
	}
	if (nodeToAdd->owner != NULL)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL); // node must not be on a list
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
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return ABC_FAIL;
	}
	if (!nodeToAdd)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL); //nodeToAdd can't be null
		return ABC_FAIL;
	}
	if (nodeToAdd->owner != NULL)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL); // node must not be on a list
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
	TRACE_ENTRY("abcSlicedList_c");
	if (nodeToFollow == NULL)
	{
		// then we must be adding at the head of the slice
		return addSliceHead(sliceNum, nodeToAdd);
	}

#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return ABC_FAIL;
	}
	if (!nodeToAdd)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL); //nodeToAdd can't be null
		return ABC_FAIL;
	}
	if (nodeToAdd->owner != NULL)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL); // node must not be on a list
		return ABC_FAIL;
	}
#endif

	// if nodeToFollow is null, then we're inserting at the the head.
	// if the list is empty, then we must be inserting at the head (and the tail, and the middle to !)
	abcListSlice_s *sliceRec = &sliceArray[sliceNum];

#ifdef _LIST_RIGOROUS_TESTING_
	// lets verify that the sliceNum matches the nodeToFollow's sliceNum
	if (sliceNum != nodeToFollow->sliceIndex)
	{
		// busted !
		FATAL_ERROR(ABC_REASON_LIST_NODE_SLICE_MISMATCH);
		return ABC_FAIL;
	}
#endif
	if (nodeToFollow == sliceRec->tailNode)
	{
		return addSliceTail(sliceNum, nodeToAdd);
	}
	// we're doing an insert in the middle of a slice
	// that is as long as we actually have a slice
	if (sliceRec->nodeCount == 0)
	{
		// this is an error because nodeToFollow should have been NULL
		// which as a case handled above.
		FATAL_ERROR(ABC_REASON_LIST_BAD_SLICE_ACCOUNTING);
		return ABC_FAIL;
	}
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

abcListNode_c		*abcSlicedList_c::findSliceFirst(int64_t sliceNum, nodeKey_s *searchKey,uint8_t towardsNext) // towardsNext controls search direction   towardsNext==1 starts from head.  ==0 starts from tail
{
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return NULL;
	}
#endif
	// get start pointer for this slice
	abcListSlice_s *mySlice = &sliceArray[sliceNum];
	abcListNode_c *startNode = towardsNext ? mySlice->headNode : mySlice->tailNode;
	if (startNode == NULL)
	{
		// ran to end
		return NULL;
	}
	TRACE_EXIT("abcSlicedList_c");
	return abcSlicedList_c::findSliceActual(sliceNum,searchKey,startNode,towardsNext);
}

abcListNode_c		*abcSlicedList_c::findSliceNext(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *previouslyFoundNode, uint8_t towardsNext) // doesn't re-examine startnode... move right(next) if towardsNext==1.
{
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return NULL;
	}
#endif
	if (!previouslyFoundNode)
	{
		return NULL;	// an error, but not fatal
	}
	
	// move past the node we previously found and returned to the user
	abcListNode_c *startNode = (abcListNode_c *)(towardsNext ? previouslyFoundNode->next() : previouslyFoundNode->prev());

	// check for end of search
	if (startNode == NULL)
	{
		// ran to end
		return NULL;
	}

	TRACE_EXIT("abcSlicedList_c");
	return abcSlicedList_c::findSliceActual(sliceNum,searchKey,startNode,towardsNext);
}
abcListNode_c		*abcSlicedList_c::findSliceActual(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext) // doesn't re-examine startnode... move right(next) if towardsNext==1.
{
	TRACE_ENTRY("abcSlicedList_c");
#ifdef _LIST_RIGOROUS_TESTING_
	if ((sliceNum < 0 ) || (sliceNum >= sliceCount))
	{
		FATAL_ERROR(ABC_REASON_LIST_INVALID_SLICE_NUMBER);
		return NULL;
	}
	if (sliceNum != startNode->sliceIndex)
	{
		// sanity test failed
		FATAL_ERROR(ABC_REASON_NODE_BAD_LINK_STRUCTURE);
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
			return NULL;
		}

		// read the question as "is the external key >= this node's key
		// if so, just past this node is the pace to do an insert
		abcResult_e diffResult = walkNode->diffKey(searchKey);
		if (diffResult ==  ABC_EQUAL)
		{
			// gone past it
			// if returnNode is NULL means we're before the startpoint
			return walkNode;
		}
		if ((diffResult == ABC_ERROR) || (diffResult == ABC_FAIL))
		{
			FATAL_ERROR(ABC_REASON_NODE_KEY_TYPE_INVALID);
			return NULL;
		}
		walkNode = (abcListNode_c *)(towardsNext ? walkNode->next() : walkNode->prev());
	}
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
abcResult_e abcHashList_c::init(int sliceCount)
{
	return abcHashList_c::init(sliceCount,10,700);
}
// hashList init sets up parameters for automatic growth [ which is not currently implemented ]
abcResult_e abcHashList_c::init(int startingSliceCount,int missThresholdForResize, int growthPercent)
{
	TRACE_ENTRY("abcHashList_c");
	if (sliceCount != 0)
	{
		FATAL_ERROR(ABC_REASON_LIST_ALREADY_INITIALIZED);
		return ABC_FAIL;
	}

	if ((startingSliceCount <2) || (startingSliceCount > _LIST_MAX_SLICE_COUNT_))
	{
		FATAL_ERROR(ABC_REASON_LIST_BAD_SLICE_COUNT);
		return ABC_FAIL;
	}

	sliceCount = abcGlobalCore->findPrime(startingSliceCount);	// always use a prime sized array for better/even spread of indexes
	sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));
	resizeHashBucketThreshold = missThresholdForResize;
	resizeHashGrowthPercentage = growthPercent;

	TRACE_EXIT("abcHashList_c");
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
	TRACE_ENTRY("abcHashList_c");

	//  check to avoid object size change  disaster.
	#define APPROVED_SIZE 128
	#define SIZE_CHECK_OBJ abcHashList_c
	if (sizeof(SIZE_CHECK_OBJ) - APPROVED_SIZE)
	{
		char errStr[128];
		snprintf(errStr,128,"sizeof \"%s\" changed from %d to %d at %s:%d",STRINGIFY(SIZE_CHECK_OBJ),(int)APPROVED_SIZE,(int)sizeof(SIZE_CHECK_OBJ),__FILE__,__LINE__);
		PROD_ERROR(errStr);
		FATAL_ERROR(ABC_REASON_OBJECT_SIZE_WRONG);
		return NULL;
	}
	#undef APPROVED_SIZE
	#undef SIZE_CHECK_OBJ

	// the list has a name but this nodes typically don't
	abcHashList_c *newList = new abcHashList_c();
	abcResult_e cloneResult = abcHashList_c::copyOut(newList);
	if (cloneResult != ABC_PASS)
	{
		delete newList;
		FATAL_ERROR(ABC_REASON_LIST_CLONE_FAILED);
		return NULL;
	}
	TRACE_EXIT("abcSlicedList_c");
	return newList;
}

abcResult_e abcHashList_c::copyOut(abcHashList_c *targetOfCopy)
{
	TRACE_ENTRY("abcHashList_c");

	// but we will use abcListNode_c's copyOut
	abcResult_e coResult = abcSlicedList_c::copyOut(targetOfCopy);

	targetOfCopy->resizeHashBucketThreshold = resizeHashBucketThreshold;
	targetOfCopy->resizeHashGrowthPercentage = resizeHashGrowthPercentage;

	TRACE_ENTRY("abcSlicedList_c");
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

abcListNode_c	*abcHashList_c::findFirst(nodeKey_s *searchKey,uint8_t towardsNext)
{
	TRACE_ENTRY("abcHashList_c");
	uint64_t keyHash = abcListNode_c::calcKeyHash(searchKey);
	int64_t  keyIndex = ((int64_t)keyHash) % sliceCount;
	keyIndex = keyIndex < 0 ? keyIndex+ sliceCount : keyIndex;
	return abcSlicedList_c::findSliceFirst(keyIndex, searchKey,towardsNext);
}

abcListNode_c	*abcHashList_c::findNext(nodeKey_s *searchKey, abcListNode_c *startNode,uint8_t towardsNext)
{
	TRACE_ENTRY("abcHashList_c");
	int64_t  keyIndex = startNode->sliceIndex;
	return abcSlicedList_c::findSliceNext(keyIndex, searchKey, startNode, towardsNext);
} // end abcHashList_c::findNext()

abcListNode_c	*abcHashList_c::getSliceHead(int64_t sliceNum)
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;
}
abcListNode_c	*abcHashList_c::getSliceTail(int64_t sliceNum) // disabled
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;
}
int64_t			abcHashList_c::getSliceNodeCount(int64_t sliceNum)
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return -1;
}
abcResult_e		abcHashList_c::addSliceHead(int64_t sliceNum, abcListNode_c *nodeToAdd)
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcResult_e		abcHashList_c::addSliceTail(int64_t sliceNum, abcListNode_c *nodeToAdd)
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcResult_e		abcHashList_c::addSliceMiddle(int64_t sliceNum, abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow )
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return ABC_FAIL;
}
abcListNode_c   *abcHashList_c::pullSliceHead(int64_t sliceNum)
{
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;;
}
abcListNode_c   *abcHashList_c::pullSliceTail(int64_t sliceNum)
{
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;;
}
abcListNode_c	*abcHashList_c::findSliceFirst(int64_t sliceNum, nodeKey_s *searchKey,uint8_t towardsNext)
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;
}
abcListNode_c	*abcHashList_c::findSliceNext(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext)
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
	return NULL;
}
abcListNode_c	*abcHashList_c::findSliceActual(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext) // disabled
{ // disabled
	TRACE_ENTRY("abcHashList_c");
	FATAL_ERROR(ABC_REASON_LIST_DISABLED_OPERATION);
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
	if (!nodeToAdd)
	{
		FATAL_ERROR(ABC_REASON_NODE_PARAM_IS_NULL);
		return ABC_FAIL;
	}
	// confirm there is no owner of the new node
	if (nodeToAdd->owner)
	{
		FATAL_ERROR(ABC_REASON_NODE_OWNER_NOT_NULL);
		return ABC_FAIL;
	}
	if (sliceCount == 0)
	{
		FATAL_ERROR(ABC_REASON_LIST_BAD_SLICE_COUNT);
		return ABC_FAIL;
	}
#endif
	
	// figure which slice to use for this hash
	// this function must be able to fail... so we'll use the global reason code to help us
	abcGlobalCore->setErrorReason(ABC_REASON_NONE);
	uint64_t fullHash = nodeToAdd->calcKeyHash();
	// TODO  this is not thread safe !!!
	if (abcGlobalCore->getErrorReason() != ABC_REASON_NONE)
	{
		FATAL_ERROR(ABC_REASON_NODE_KEY_TYPE_NOT_INITIALIZED);
		return ABC_FAIL;
	}
	int64_t keyIndex = (int64_t)fullHash % sliceCount;	// modulo for slice index. possibly < 0
	keyIndex = (keyIndex < 0) ? keyIndex+sliceCount : keyIndex;  // index can't be negative
	
	abcListSlice_s *mySlice = &sliceArray[keyIndex];
	if (mySlice->nodeCount >= resizeHashBucketThreshold)
	{	
		DEBUG_A("ABOUT TO RESIZE THE HASH TABLE\n");
		resizeHashTable();
		///PERROR_LTD(5,3,ABC_REASON_LIST_HASH_SLICE_OVERFULL);
		//PERROR_FAIL(ABC_REASON_LIST_HASH_SLICE_OVERFULL);
		//return ABC_FAIL;
	}
	nodeToAdd->sliceIndex = keyIndex;
	return abcSlicedList_c::addSliceTail(keyIndex, nodeToAdd);
} // end abcHashList_c::add(abcListNode_c *nodeToAdd)	// MUST have the key already properly initialized

abcResult_e abcHashList_c::resizeHashTable()
{
	// come here to completely rebuild the hashLlist with a bigger hash array.  We'll do it by disconnecting the entire list from the anchoring
	// data structs.... effectively re-adding all nodes to an empty (larger) list structure.

	abcListNode_c *localHeadNode = headNode;
	abcListNode_c *localTailNode = tailNode;
	int64_t localNodeCount = nodeCount;
	abcListSlice_s *localSliceArray = sliceArray;
	int				localSliceCount = sliceCount;
	DEBUG_A("Resizing params:  Threshold=%d  resizePrecentage=%d\n", resizeHashBucketThreshold,resizeHashGrowthPercentage);

	// figure new array Size.
	// we'll add some internal knobs
	int64_t primeEst = ((int64_t)sliceCount * (int64_t)resizeHashGrowthPercentage) / 100LL;
	sliceCount = abcGlobalCore->findPrime(primeEst);
	sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));
	double increasedMissThreshold = pow((double)resizeHashBucketThreshold,(1.09));
	resizeHashBucketThreshold = (int)(increasedMissThreshold + 0.5);
	resizeHashGrowthPercentage = (resizeHashGrowthPercentage * 80 ) / 100;
	DEBUG_A("New resizing params:  Threshold=%d  resizePrecentage=%d\n", resizeHashBucketThreshold,resizeHashGrowthPercentage);

	sliceArray = (abcListSlice_s *)calloc(sliceCount, sizeof(abcListSlice_s));

	headNode = tailNode = NULL;
	nodeCount = 0;

	int loopCount = 0;
	abcListNode_c *walkNode = headNode;
	abcListNode_c *freeNode;
	while (walkNode)
	{
		// pullHead
		freeNode = walkNode;
		walkNode = (abcListNode_c *)walkNode->nextNode;
		walkNode->prevNode = NULL;
		freeNode->nextNode = NULL;
		freeNode->owner = NULL;

		// now add the freeNode.
		abcResult_e addResult = add(freeNode);
		if (addResult != ABC_PASS)
		{
			FATAL_ERROR(ABC_REASON_LIST_HASH_RESIZE_FAILED)
			return ABC_FAIL;
		}
		loopCount++;
	}
	if (loopCount != localNodeCount)
	{
		FATAL_ERROR(ABC_REASON_LIST_HASH_RESIZE_FAILED)
		return ABC_FAIL;
	}
	if (freeNode != localTailNode)
	{
		FATAL_ERROR(ABC_REASON_LIST_HASH_RESIZE_FAILED)
		return ABC_FAIL;
	}

	// new list should be all set.  lets dump the old slice array
	free(localSliceArray);

	return ABC_PASS;
}


///////////////////
// EOF abcList.cpp
