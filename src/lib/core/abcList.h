/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com
// File:  abcList.h
// 
// abcNode_s is the most basic node of the list construct
// abcListNode_s is the base class of the list object's list node
//

#ifndef __ABC_LIST_H__
#define __ABC_LIST_H__

#include "abcNodes.h"

class abcList_c : public abcListNode_c	// a list is also a listNode
{
  private:



  protected:
	char			*name;
	abcListNode_c *headNode;
	abcListNode_c *tailNode;
	abcListNode_c *currNode;

  	// storage for locking
  	uint8_t				lockingEnabled;
	uint8_t				writerIsWaiting;
	uint8_t				readerIsWaiting;
	int8_t				registeredReaders; 	// count of active readers.  multiple simultaneous readers allowed	(128 limit !)
  	pthread_mutex_t     mutex;
	pthread_cond_t      readerCondVar;		// reader signals this var.  Writer waits on it
	pthread_cond_t      writerCondVar;		// writer signals this var.  Reader waits on it
	

	int64_t		nodeCount;
	uint8_t		isSorted;		// safe flag to stop mixing usage

	abcResult_e		lock();		// lock the mutex
	abcResult_e		unlock();	// unlock the mutex





  public:

	// lifecyle stuff
	abcList_c(const char *setName = NULL);
	~abcList_c();

	// baseClass only
	// multi-threading api.  Register for reads and lock for writes.
	// PLEASE NOTE: 
	//		Registration and locking are for multi-threading only.  Different spots in the same
	// 		thread cannot use multithread registration or locking to avoid state corruption and must use stateless operation
	// 		to avoid changing the "curr" "head" or "tail" pointers behind the back of a different bit of code.  The only exception
	//		to this principal is to use NoWait version which will return with status if currently locked.
	//
	abcResult_e	enableLocking();	// turn on locking ability.  Read/Write registrastion require locking.
	abcResult_e	disableLocking();	// turn off locking ability
	abcResult_e	readRegister();
	abcResult_e	readRegisterNoWait();
	abcResult_e	readRelease();
	abcResult_e	writeLock();
	abcResult_e	writeLockNoWait();
	abcResult_e	writeRelease();

    virtual char        *getObjType();  // the class name
    virtual char        *getObjName();  // the instance name when available
    virtual abcResult_e print(abcPrintStyle_e printStyle);
    abcResult_e         printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle);

	////////////////
	// stateful operation  ... just these will remain stateful.  All the rest are stateless (do not effect or use currNode variable)
	abcListNode_c		*head();		// get and set currNode <= headNode
	abcListNode_c		*tail();		// get and set currNode <= tailNode
	abcListNode_c		*next();		// get and set currNode <= currNode->nextNode
	abcListNode_c		*prev();		// get and set currNode <= currNode->prevNode
	abcListNode_c		*getCurr();

	// stateless operation
	abcListNode_c		*getHead();
	abcListNode_c		*getTail();	
	int64_t				 getNodeCount();					// get count of nodes on list.

	virtual abcResult_e		addHead(abcListNode_c *nodeToAdd);							 // add a new node at head and update head.  Do not update current.
	virtual abcResult_e		addHeadPriv(abcListNode_c *nodeToAdd);			 // Internal add a new node at head and update head.  Do not update current.
	virtual abcResult_e		addTail(abcListNode_c *nodeToAdd);							 // add a new node at tail and update tail.  Do not update currenty
	virtual abcResult_e		addTailPriv(abcListNode_c *nodeToAdd);			 //  Internal/ add a new node at tail and update tail.  Do not update currenty
	virtual abcResult_e		addMiddle(abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow ); // insert new node after specified noed
	virtual abcResult_e		insert(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow);	// insert without disturbing curr.  return "next" node
	virtual abcResult_e		insertPriv(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow);	// Internal insert without disturbing curr.  return "next" node

	virtual abcResult_e		remove(abcListNode_c *nodeToRemove);						//  remove specifified node
	virtual abcListNode_c	*pullHead();												// return the head of the list after removing it. 
	virtual abcListNode_c	*pullTail();												// return the tail of the list after removing it.

	// simple linear sorting..... abcListNode_c must already have its key properly set.
	virtual abcResult_e 	addSorted(abcListNode_c *nodeToAdd, const uint8_t towardsNext ); // control start positonal for search (head for towardsNext, tail for !towardsNext)

	// search stateless only
	virtual abcListNode_c	*findFirst(nodeKey_s *searchKey,uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	  					 	// towardsNext controls search direction   towardsNext==1 starts from head.  ==0 starts from tail
	virtual abcListNode_c	*findNext(nodeKey_s *searchKey,abcListNode_c *startNode,  uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	// doesn't re-examine startnode... move right(next) if towardsNext==1.
	virtual abcListNode_c	*findActual(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext, abcResult_e *resultOut=NULL);	// doesn't re-examine startnode... move right(next) if towardsNext==1.

	// full list operations
	virtual abcResult_e empty();
	virtual abcResult_e diff(class abcList_c *otherList);
	virtual abcList_c *clone();	 	// make a copy of this list
	abcResult_e copyOut(abcList_c *targetOfCopy);
	virtual abcList_c *extractCommonList(class abcList_c *otherList);	// make a third list which is all the commom nodes.  remove common nodes from both lists. (using diffNode not diffKey)


};

/////////////////////////// /////////////////////////// /////////////////////////// /////////////////////////// 
//
// List subclass abcSlicedList_c and its support data structures.
//
//
typedef struct abcListSlice_s
{
	abcListNode_c	*headNode;
	abcListNode_c	*tailNode;
	int64_t		 	nodeCount;
} abcListSlice_s;

//
// a list that is divided into slices.  Allocate a fixed number of slices at construction.
// when adding a node to this, must specify the slice #
//
class abcSlicedList_c  : public abcList_c	
{
  protected:
  	abcListSlice_s	*sliceArray;	// record keeping for the slices.
	int				sliceCount;		// when zero, sliceArray not initalized

  public:


	// lifecyle stuff
	abcSlicedList_c(const char *setName = NULL);
	~abcSlicedList_c();
	virtual abcResult_e init(int sliceCount);	// must set the number of slices being used

	// print stuff
    virtual char        *getObjType();  // the class name
    virtual abcResult_e print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_WITH_NODE_DETAILS);
    abcResult_e         printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle=PRINT_STYLE_LIST_WITH_NODE_DETAILS);


	// full abcSlicedList_c list operations
	virtual abcResult_e empty();
	virtual abcResult_e diff(class abcList_c *otherList);
	virtual abcList_c *clone();	 										// make a copy of this list
	abcResult_e copyOut(abcSlicedList_c *targetOfCopy);						// this one is never virtual

	virtual abcList_c *extractCommonList(class abcList_c *otherList);	// make a third list which is all the commom nodes.  remove common nodes from both lists. (using diffNode not diffKey)


	// *******************************************  
	// stateful operation  ... base class behavior retained but not recommended
	//abcListNode_c		*head();		// abcList_c head used
	//abcListNode_c		*tail();		// abcList_c tail used
	//abcListNode_c		*next();		// abcList_c prev used
	//abcListNode_c		*prev();		// abcList_c next used
	//abcListNode_c		*getCurr();

	// stateless operation
	//abcListNode_c		*getHead();			// abcList_c getHead used
	//abcListNode_c		*getTail();			// abcList_c getTail used
	//int64_t			 getNodeCount();	// abcList_c getNodeCount used

	// for abcHashList these really can't work
	virtual abcResult_e		addHead(abcListNode_c *nodeToAdd);							 			// overloaded to fail
	virtual abcResult_e		addTail(abcListNode_c *nodeToAdd);							 			// overloaded to fail
	virtual abcResult_e		addMiddle(abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow );		// overloaded to fail
	virtual abcResult_e		insert(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow);		// overloaded to fail
	virtual abcResult_e		insertPriv(abcListNode_c *nodeToInsert, abcListNode_c *nodeToFollow);	// overloaded to fail

	virtual abcResult_e		remove(abcListNode_c *nodeToRemove);									// overloaded for avcSlicedList_c... also uses baseclass remove for the base list

	// these can work straight from the baseclass
	//virtual abcListNode_c	*pullHead();															// baseclass works as needed because remove is overloaded to work with sliced list
	//virtual abcListNode_c	*pullTail();															// baseclass works as needed because remove is overloaded to work with sliced list

	// simple linear sorting =  can't work correctly
	virtual abcResult_e 	addSorted(abcListNode_c *nodeToAdd, const uint8_t towardsNext ); 		// overloaded to fail

	// search stateless only - works directly from the baseclass ... even if it makes no sense !!
	//virtual abcListNode_c	*findFirst(nodeKey_s *searchKey,uint8_t towardsNext, abcResult_e *resultOut=NULL);	  					 		// uses abcList_c findFirst
	//virtual abcListNode_c	*findNext(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext, abcResult_e *resultOut=NULL);	// uses abcList_c findNext
	//virtual abcListNode_c	*findActual(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext, abcResult_e *resultOut=NULL);	// uses abcList_c findActual


	// *******************************************  
	// slice versions of the basic methods

	virtual abcListNode_c	*getSliceHead(int64_t sliceNum, abcResult_e *resultOut = NULL);								
	virtual abcListNode_c	*getSliceTail(int64_t sliceNum, abcResult_e *resultOut = NULL);
	virtual int64_t			getSliceNodeCount(int64_t sliceNum, abcResult_e *resultOut = NULL);

	virtual abcResult_e		addSliceHead(int64_t sliceNum, abcListNode_c *nodeToAdd);									 // add a new node at head and update head.  Do not update current.
	virtual abcResult_e		addSliceTail(int64_t sliceNum, abcListNode_c *nodeToAdd);									 // add a new node at tail and update tail.  Do not update currenty
	virtual abcResult_e		addSliceMiddle(int64_t sliceNum, abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow );  	 // insert new node after specified noed
	virtual abcListNode_c	*pullSliceHead(int64_t sliceNum);															 // getSliceHead and if not null remove and return it;
	virtual abcListNode_c	*pullSliceTail(int64_t sliceNum);															 // getSliceHead and if not null remove and return it;
	

	virtual abcListNode_c	*findSliceFirst(int64_t sliceNum, nodeKey_s *searchKey,uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	  					 	// towardsNext controls search direction   towardsNext==1 starts from head.  ==0 starts from tail
	virtual abcListNode_c	*findSliceNext(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	// doesn't re-examine startnode... move right(next) if towardsNext==1.
	virtual abcListNode_c	*findSliceActual(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	// doesn't re-examine startnode... move right(next) if towardsNext==1.
}; // end abcSlicedList_c definition

//////////////////     ======================       /////////////////////      ========================     /////////////////////////

// a hash with collision is implemented with list.  This list can be resized (automatically ?) when the to many nodes share a hashIndex.
class abcHashList_c  : public abcSlicedList_c	// a list is also a listNode
{
  protected:
  	friend class abcMemMon_c;

	int			specialInit;					// flag initProtected
	int			startingSliceCount;
	int			startingResizeThreshold;
  	int			resizeHashBucketThreshold; 		// the size of a slice needed to invoke hash resizing;
  	int			resizeHashGrowthPercentage; 	// the amount to grow by when above threshold is met.
	abcResult_e	initProtected(int startingSliceCount=1000,int missThresholdForResiz=5, int growthPercent=220);


  public:

	// lifecyle stuff
	abcHashList_c(const char *setName = NULL);
	~abcHashList_c();
	virtual abcResult_e init(int sliceCount);				

	// print stuff
    virtual char        *getObjType();  // the class name
    virtual abcResult_e print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_WITH_NODE_DETAILS);
    abcResult_e         printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e  printStyle); //nonVirtual prinfbuff for this level of the object hiearchy

	abcResult_e resizeHashTable();			// rebuild the list using a new hash array.  This is bruteforce.  we can do it elecgantly later.

	//full abcHashList_c list operations
	// virtual abcResult_e empty();									// SlicedList empty used
	//virtual abcResult_e diff(class abcList_c *otherList);			// SlicedLlist diff used
	virtual abcList_c *clone();	
	abcResult_e copyOut(abcHashList_c *targetOfCopy);
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

	// search stateless only - works directly from the baseclass
	virtual abcListNode_c	*findFirst(nodeKey_s *searchKey,uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	  					 		// implemented using abcSlicedList_c::findSliceFirst
	virtual abcListNode_c	*findNext(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	// implemented using abcSlicedList_c::findSliceNext
	//virtual abcListNode_c	*findActual(nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext=TRUE, abcResult_e *resultOut=NULL);	// directly use      abcSlicedList_c::findSliceActual

	// *******************************************  
	// slice versions of the basic methods

	virtual abcListNode_c	*getSliceHead(int64_t sliceNum, abcResult_e *resultOut = NULL);				// overloaded to fail
	virtual abcListNode_c	*getSliceTail(int64_t sliceNum, abcResult_e *resultOut = NULL);				// overloaded to fail
	virtual int64_t			getSliceNodeCount(int64_t sliceNum, abcResult_e *resultOut = NULL);			// overloaded to fail

	virtual abcResult_e		addSliceHead(int64_t sliceNum, abcListNode_c *nodeToAdd);	 											 // overloaded to fail
	virtual abcResult_e		addSliceTail(int64_t sliceNum, abcListNode_c *nodeToAdd);	 											 // overloaded to fail
	virtual abcResult_e		addSliceMiddle(int64_t sliceNum, abcListNode_c *nodeToAdd, abcListNode_c *nodeToFollow );  				 // overloaded to fail
	virtual abcListNode_c	*pullSliceHead(int64_t sliceNum);															 			 // overloaded to fail
	virtual abcListNode_c	*pullSliceTail(int64_t sliceNum);															 			 // overloaded to fail

	virtual abcListNode_c	*findSliceFirst(int64_t sliceNum, nodeKey_s *searchKey,uint8_t towardsNext,  abcResult_e *resultOut = NULL);	  		 				 // overloaded to fail
	virtual abcListNode_c	*findSliceNext(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext,  abcResult_e *resultOut = NULL);	 // overloaded to fail
	virtual abcListNode_c	*findSliceActual(int64_t sliceNum, nodeKey_s *searchKey, abcListNode_c *startNode, uint8_t towardsNext,  abcResult_e *resultOut = NULL); // overloaded to fail

	///////////  **********  Thisngs unique abcHashList_c ************  //////////////

	virtual abcResult_e add(abcListNode_c *nodeToAdd);

}; // end abcHashList_c class definition

#endif //__ABC_LIST_H__

////////////////////////////
// EOF abcList.h

