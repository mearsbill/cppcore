/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author Bill Mears   bill@imears.com  2017-1201
 // abcMemory.cpp   
 //
#include "abcMemory.h"
#include "abcCore.h"

///////////////////////////////////
//
// abcMemNameNode_c
//
///////////////////////////////////

abcMemNameNode_c::abcMemNameNode_c(uint64_t hash, const char *setName, int setUseCount)
{	
	key.type = KEYTYPE_INT;
	key.value.intgr = hash;
	useCount = setUseCount;
	if (setName)
	{
		name = strdup(setName);
	}
	else
	{
		name = NULL;
	}
}
abcMemNameNode_c::~abcMemNameNode_c()
{
	if (name)
	{
		free(name);
		name = NULL;
	}
}
//
//// standard stuff needed for all list node subclasses..
//

// infrastructure support for subclasses pf abcListNode_c
char *abcMemNameNode_c::getObjType()
{
	return (char *)"abcMemNameNode_c";
}
char *abcMemNameNode_c::getObjName()
{
	return name;
}
abcResult_e abcMemNameNode_c::print(abcPrintStyle_e printStyle)
{
	char bPuff[1024];
	printBuff(bPuff,1024,printStyle);
	fprintf(stderr,"%s\n",bPuff);
	return ABC_PASS;
}
abcResult_e abcMemNameNode_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char tBuff[1024];
	abcListNode_c::printBuff(tBuff,1024,printStyle);
	snprintf(pBuff,pbuffSize,"%s used:%lld",tBuff,useCount);
	return ABC_PASS;
}
abcListNode_c *abcMemNameNode_c::clone()
{
	return NULL;
}
abcResult_e abcMemNameNode_c::copyOut(abcMemNameNode_c *targetOfCopy)
{
	return ABC_PASS;
}
abcResult_e	abcMemNameNode_c::diffNode(class abcListNode_c *other)
{
	return ABC_PASS;
}


//
// Instance Name
//    fileName:fileLine[function]"
//
// Object Name
// for raw	  = "R:storageSize"
// for struct = "S:structName"
// for classo = "C:className"
//
char *abcMemNameNode_c::buildLocationString(const char *fileName, const int fileLine, const char *function)
{
	char buildStr[256];
	snprintf(buildStr,256,"%s@%s:%d",function,fileName,fileLine);
	return strdup(buildStr);
}
char	*abcMemNameNode_c::buildObjectClassString(const char *className)
{
	char buildStr[256];
	snprintf(buildStr,256,"C:%s",className);
	return strdup(buildStr);
}
char	*abcMemNameNode_c::buildObjectStructString(const char *structName)
{
	char buildStr[256];
	snprintf(buildStr,256,(char *)"S:%s",structName);
	return strdup(buildStr);
}
char	*abcMemNameNode_c::buildObjectRawString(const int size)
{
	char buildStr[256];
	snprintf(buildStr,256,"R:%d",size);
	return strdup(buildStr);
}
uint64_t	abcMemNameNode_c::calcNameHash(const char *string)
{
	int strLen = strlen(string);
	return globalCore->computeCrc64((uint8_t *)string,strLen);
}
int64_t	abcMemNameNode_c::incrUseCount()
{
	useCount++;
	return useCount;
}
int64_t	abcMemNameNode_c::getUseCount()
{
	return useCount;
}


///////////////////////////////////
//
// abcMemStatsNode_c
//
///////////////////////////////////

abcMemStatsNode_c::abcMemStatsNode_c(uint64_t setObjTypeHash, const char *setObjTypeName, int setSize, int setCreateCount)
{
	key.type = KEYTYPE_INT;
	key.value.intgr = setObjTypeHash;
	objTypeName = strdup(setObjTypeName);
	size = setSize;
	createCount = setCreateCount;
	destroyCount = 0;
}
abcMemStatsNode_c::~abcMemStatsNode_c()
{
	free(objTypeName);
}

// infrastructure support for subclasses of abcListNode_c
char *abcMemStatsNode_c::getObjType()
{
	return (char *)"abcMemStatsNode_c";
}
char *abcMemStatsNode_c::getObjName()
{
	return objTypeName;
}
abcResult_e abcMemStatsNode_c::print(abcPrintStyle_e printStyle)
{
	char bPuff[1024];
	printBuff(bPuff,1024,printStyle);
	fprintf(stderr,"%s\n",bPuff);
	return ABC_PASS;
}
abcResult_e abcMemStatsNode_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char tBuff[1024];
	abcListNode_c::printBuff(tBuff,1024,printStyle);
	snprintf(pBuff,pbuffSize,"%s sz:%d cCnt:%lld dCnt:%lld",tBuff,size,createCount,destroyCount);
	return ABC_PASS;
}
abcListNode_c *abcMemStatsNode_c::clone()
{
	return NULL;
}
abcResult_e abcMemStatsNode_c::copyOut(abcMemStatsNode_c *targetOfCopy)
{
	return ABC_PASS;
}
abcResult_e	abcMemStatsNode_c::diffNode(class abcListNode_c *other)
{
	return ABC_PASS;
}


abcResult_e		abcMemStatsNode_c::incrCreates(int count)
{
	createCount += count;
	return ABC_PASS;
}
abcResult_e		abcMemStatsNode_c::incrDestroys(int count)
{
	destroyCount += count;
	return ABC_PASS;
}

// report out
int abcMemStatsNode_c::getCreateCount()
{
	return createCount;
}
int abcMemStatsNode_c::getDestroyCount()
{
	return destroyCount;
}
int abcMemStatsNode_c::getSize()
{
	return size;
}



///////////////////////////////////
//
// abcMmapNode_c
//
///////////////////////////////////

abcMmapNode_c::abcMmapNode_c(void *locPtr)
{
	key.type = KEYTYPE_PTR;
	key.value.ptr = locPtr;
	statsPtr=NULL;
	locNewPtr=NULL;
	locDelPtr=NULL;
}
abcMmapNode_c::~abcMmapNode_c()
{
}

// infrastructure support for subclasses pf abcListNode_c
char *abcMmapNode_c::getObjType()
{
	return (char *)"abcMmapNode_c";
}
char *abcMmapNode_c::getObjName()
{
	return NULL;
}
abcResult_e abcMmapNode_c::print(abcPrintStyle_e printStyle)
{
	char bPuff[1024];
	printBuff(bPuff,1024,printStyle);
	fprintf(stderr,"%s\n",bPuff);
	return ABC_PASS;
	return ABC_PASS;
}
abcResult_e abcMmapNode_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char tBuff[1024];
	abcListNode_c::printBuff(tBuff,1024,printStyle);
	snprintf(pBuff,pbuffSize,"%s stats:%p locNew:%p locDel:%p",tBuff,statsPtr,locNewPtr,locDelPtr);
	return ABC_PASS;
}
abcListNode_c *abcMmapNode_c::clone()
{
	return NULL;
}
abcResult_e abcMmapNode_c::copyOut(abcMmapNode_c *targetOfCopy)
{
	return ABC_FAIL;
}
abcResult_e	abcMmapNode_c::diffNode(class abcListNode_c *other)
{
	return ABC_FAIL;
}


abcResult_e	abcMmapNode_c::setStatsNode(abcMemStatsNode_c *stats)
{
	statsPtr = stats;
	return ABC_PASS;
}
abcMemStatsNode_c *abcMmapNode_c::getStatsNode()
{
	return statsPtr;
}
abcResult_e	abcMmapNode_c::setLocNewPtr(abcMemNameNode_c *newLoc)
{
	locNewPtr = newLoc;
	return ABC_PASS;
}
abcMemNameNode_c *abcMmapNode_c::getLocNewPtr()
{
	return locNewPtr;
}
abcResult_e	abcMmapNode_c::setLocDelPtr(abcMemNameNode_c *delLoc)
{
	locDelPtr = delLoc;
	return ABC_PASS;
}
abcMemNameNode_c *abcMmapNode_c::getLocDelPtr()
{
	return locDelPtr;
}

///////////////////////////////////
//
// abcMemMon_c
//
///////////////////////////////////


abcMemMon_c::abcMemMon_c(const char *setName)
{
	name = NULL;
	if (setName) 
	{
		name = strdup(setName);
	}
	mmapList = new abcHashList_c("mmapList");
	abcResult_e res = mmapList->initProtected(1000,3,300);
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	res = mmapList->enableLocking();
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	delmapList = new abcHashList_c("delmapList");
	res = delmapList->initProtected(1000,3,300);
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	res = delmapList->enableLocking();
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	nameList = new abcHashList_c("nameList");
	res = nameList->initProtected(2000,6,300);
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	res = nameList->enableLocking();
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	statsList = new abcHashList_c("statsList");;
	res =statsList->initProtected(400,6,200);
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	res = statsList->enableLocking();
	CHECK_ERROR(res, ABC_REASON_LIST_INIT_FAILED,);

	reason = ABC_REASON_NONE;

	// zero our incremental stats
	netBytesInUse = 0;
	maxMemoryUsed = 0;
	totalBytesMalloced = 0;
	totalBytesFreed = 0;
	netMallocsInUse = 0;
	maxMallocsUsed = 0;
	totalMallocCount = 0;
	totalFreeCount = 0;


}

abcMemMon_c::~abcMemMon_c()
{
	if (name)
	{
		free(name);
		name = NULL;
	}
	if (statsList)
	{
		delete statsList;
		statsList = NULL;
	}
	if (nameList)
	{
		delete nameList;
		nameList = NULL;
	}
	if (delmapList)
	{
		delete delmapList;
		delmapList = NULL;
	}
	if (mmapList)
	{
		delete mmapList;
		mmapList = NULL;
	}
}

void abcMemMon_c::resetReason()
{
	reason = ABC_REASON_NONE;
}
void abcMemMon_c::setReason(abcReason_e setReason)
{
	reason = setReason;
}
abcReason_e abcMemMon_c::getReason()
{
	return reason;
}

void *abcMemMon_c::interceptClassNew(void *objAddr, char *className, int objectSize, char *fileName, int fileLine, char *fileFunction)
{
	// build the right hasName based on the object type... we're a class
	char *objHashName = abcMemNameNode_c::buildObjectClassString(className);
	return interceptCommonNew(objAddr, objHashName, objectSize, fileName, fileLine, fileFunction);
}

void *abcMemMon_c::interceptStructNew(void *structAddr, char *structName, int objectSize, char *fileName, int fileLine, char *fileFunction)
{
	// build the right hasName based on the object type... we're a class
	char *objHashName = abcMemNameNode_c::buildObjectStructString(structName);
	return interceptCommonNew(structAddr, objHashName, objectSize, fileName, fileLine, fileFunction);
}

void *abcMemMon_c::interceptRawNew(void *rawAddr, int objectSize, char *fileName, int fileLine, char *fileFunction)
{
	// build the right hasName based on the object type... we're a class
	char *objHashName = abcMemNameNode_c::buildObjectRawString(objectSize);
	return interceptCommonNew(rawAddr, objHashName, objectSize, fileName, fileLine, fileFunction);
}

void *abcMemMon_c::interceptCommonNew(void *objAddr, char *objHashName, int objectSize, char *fileName, int fileLine, char *fileFunction)
{
	nodeKey_s searchKey;
	abcMmapNode_c *mmapNode;

	///////////////////////////========================
	abcResult_e res = mmapList->writeLock();
	CHECK_ERROR(res,ABC_REASON_LIST_LOCK_FAILED,NULL);

#ifdef ABC_MEM_STRICT_CHECKING
	// build a search key from the memory address
	// and verify that we don't have this address.
	// Caution... we can malloc and free the same address over and over again !
	nodeKey_setPtr(&searchKey,objAddr);

	mmapNode = (abcMmapNode_c *)mmapList->findFirst(&searchKey,1,&res);
	if (res)
	{
		ERROR(ABC_REASON_LIST_FIND_FAILED);
		 mmapList->writeRelease();
		 return NULL;
	}
	if (mmapNode)
	{
		// serious error to find the address we're mallocing on our list already
		ERROR(ABC_REASON_MMAP_DUPLICATE_ADDR_INVALID);
		mmapList->writeRelease();
		return NULL;
	}
#endif

	// now we've confirmed no dup, lets make the node
	mmapNode = new abcMmapNode_c(objAddr);
	abcResult_e result = mmapList->add(mmapNode);
	if (result != ABC_PASS)
	{
		ERROR(ABC_REASON_MMAP_ADD_FAILED);
		mmapList->writeRelease();
		return NULL;
	}

	//
	// build the objName string and hashKey.  Use this to find or build our MemStatsNode
	//
	uint64_t nameHash = abcMemNameNode_c::calcNameHash(objHashName);
	nodeKey_setInt(&searchKey,nameHash);

	//////////////////////////////////////////////////
	// write lock stats list for this critical section
	res = statsList->writeLock();
	if (res)
	{
		ERROR(ABC_REASON_LIST_LOCK_FAILED);
		mmapList->writeRelease();
		return NULL;
	}

	abcMemStatsNode_c *statsNode = (abcMemStatsNode_c *)statsList->findFirst(&searchKey,1,&res);
	if (res)
	{
		ERROR(ABC_REASON_LIST_FIND_FAILED);
		mmapList->writeRelease();
		return NULL;
	}
	if (!statsNode)
	{
		// we didn't find it, so we'll build it here
		statsNode = new abcMemStatsNode_c(nameHash, objHashName, objectSize,1);
		statsList->add(statsNode);
	}
	else
	{
		statsNode->incrCreates();
	}
	// unblock the statsList asap
	statsList->writeRelease();	

	free(objHashName);	// its now stored in the nameNode
	mmapNode->setStatsNode(statsNode);
	//////////////////////////////////////////////////

	//
	// we now have a stats record... still need the nameNode for the newLoc
	//
	char *newLoc = abcMemNameNode_c::buildLocationString(fileName, fileLine, fileFunction);
	uint64_t locHash = abcMemNameNode_c::calcNameHash(newLoc);
	nodeKey_setInt(&searchKey,locHash);

	res = nameList->writeLock();	// lock the list for all access
	if (res)
	{
		ERROR(ABC_REASON_LIST_FIND_FAILED);
		nameList->writeRelease();
		mmapList->writeRelease();
		return NULL;
	}
	abcMemNameNode_c *nameNode = (abcMemNameNode_c *)nameList->findFirst(&searchKey);
	if (nameNode)
	{
		nameNode->incrUseCount();		// another object create from this source file/line spot.
	}
	else
	{
		// didn't find newLoc on the hashList, lets make it and add it to our list
		nameNode = new abcMemNameNode_c (locHash,newLoc,1);
		nameList->add(nameNode);
	}
	free(newLoc);
	mmapNode->setLocNewPtr(nameNode);

	nameList->writeRelease(); // release the writelock
	mmapList->writeRelease();

	// update incremental stats
	totalBytesMalloced += objectSize;
	netBytesInUse += objectSize;
	if (netBytesInUse > maxMemoryUsed)
	{
		maxMemoryUsed = netBytesInUse;
	}
	totalMallocCount++;
	netMallocsInUse++;
	if (netMallocsInUse > maxMallocsUsed)
	{
		maxMallocsUsed = netMallocsInUse;
	}

	return objAddr;
}
//
// common delete for all object types.
void abcMemMon_c::interceptDelete(void *objAddr, char *fileName, int fileLine, char *fileFunction)
{
	// we'll make a search key based on the object address and confirm we have a node for that address.
	nodeKey_s searchKey;
	nodeKey_setPtr(&searchKey,objAddr);

	// lock the mmap list before manipulation
	mmapList->writeLock(); 
	abcMmapNode_c *mmapNode = (abcMmapNode_c *)mmapList->findFirst(&searchKey);
	if (!mmapNode)
	{
		// serious error to delete something that isn't there
		ERROR(ABC_REASON_MMAP_DEL_FAILED);
		mmapList->writeRelease();
		return;
	}
	// check for double delete
	if (mmapNode->locDelPtr)
	{
		ERROR(ABC_REASON_MMAP_ALREADY_DELETED);
		mmapList->writeRelease();
		return;
	}


	// we're going to cheat and not lock the stats list
	// even thought technically we're writing to it (to one of its nodes)
	//statsList->writeLock(); 
	// now get to the stats record and record the delete
	abcMemStatsNode_c *statsNode = mmapNode->getStatsNode();
	if (!statsNode)
	{
		ERROR(ABC_REASON_MMAP_STATS_NODE_MISSING);
		//statsList->writeRelease(); 
		mmapList->writeRelease(); 
		return;
	}
	statsNode->incrDestroys();	// tabulate the destroy here
	int64_t objectSize = statsNode->size;
	//statsList->writeRelease(); 


	// we're going to cheat and not lock the name list
	// even thought technically we're writing to it (to one of its nodes)
	// nameList->writeLock(); 

	// now get to the stats record and record the delete
	// lets handle the delete location... the location might exists
	// but not have been used for this node [ actually its hightly likely ]
	abcMemNameNode_c *delNameNode = mmapNode->getLocDelPtr();
	if (!delNameNode)
	{
		// need to build a location string to try to find the delete location on the name list
		char *delLoc = abcMemNameNode_c::buildLocationString(fileName, fileLine, fileFunction);
		uint64_t locHash = abcMemNameNode_c::calcNameHash(delLoc);
		nodeKey_setInt(&searchKey,locHash);

		// lock before search... write lock because we may add to list
		nameList->writeLock();  // LOCK
		abcMemNameNode_c *nameNode = (abcMemNameNode_c *)nameList->findFirst(&searchKey);
		if (!nameNode)
		{
			// didn't find deLoc on the hashList, lets make it and add it to our list
			delNameNode = new abcMemNameNode_c(locHash,delLoc,0);
			nameList->add(delNameNode);
		}
		free(delLoc);
		mmapNode->setLocDelPtr(delNameNode);

		nameList->writeRelease(); //UNLOCK
	}
	delNameNode->incrUseCount();		// another object destroy from this source file/line spot.

	// mmap still locked.... lets move the mmapNode to the deleted list
	abcResult_e res = mmapList->remove(mmapNode);
	if (res)
	{
		ERROR(ABC_REASON_MMAP_DEL_FAILED);
		return;
	}
	res = mmapList->writeRelease();
	if (res)
	{
		ERROR(ABC_REASON_MMAP_LOCK_FAILED);
		return;
	}

	// now lock and add mmapNode to the deleted list
	res = delmapList->writeLock();
	if (res)
	{
		ERROR(ABC_REASON_MMAP_LOCK_FAILED);
		return;
	}
	res = mmapList->add(mmapNode);
	if (res)
	{
		ERROR(ABC_REASON_MMAP_ADD_FAILED);
		return;
	}

	// writeRelease the delmapList 
	res = delmapList->writeRelease();
	CHECK_ERROR(res,ABC_REASON_MMAP_LOCK_FAILED,);

	// update incremental stats
	totalBytesFreed += objectSize;
	netBytesInUse -= objectSize;
	totalFreeCount++;
	netMallocsInUse--;
}

void abcMemMon_c::printMemoryMap()
{
	fprintf(stderr,"\nMemoryMap... in no particular order\n");
	mmapList->readRegister();
	abcMmapNode_c *mapWalkNode = (abcMmapNode_c *)mmapList->getHead();
	abcMemStatsNode_c *statsNode;
	abcMemNameNode_c *locNewPtr, *locDelPtr;
	int64_t netOutstanding = 0;
	while (mapWalkNode)
	{

//mapWalkNode->print(PRINT_STYLE_LIST_MMAP_LIST);
		//
		statsNode = mapWalkNode->statsPtr;
		locNewPtr = mapWalkNode->locNewPtr;
		locDelPtr = mapWalkNode->locDelPtr;

		char *deletedStr = (char *)"No";
		int size = statsNode->size;
		if (locDelPtr)
		{
			deletedStr = locDelPtr->name;
		}
		else
		{
			netOutstanding += size;
		}
	
		// print addr, class name,size file,line, function <deleted>
		fprintf(stderr,"%p:  %15s sz:%-3d New'd: %-20s  Del'd: %-20s\n",
			mapWalkNode->key.value.ptr,statsNode->objTypeName,statsNode->size, locNewPtr->name,deletedStr);


		mapWalkNode = (abcMmapNode_c *)mapWalkNode->next();
	}
	mmapList->readRelease();
	fprintf(stderr,"======== Total Outstanding Memory is %lld\n",netOutstanding);
}

/****************************  printout stuff **************************/
void abcMemMon_c::printMemoryStats()
{
	fprintf(stderr,"\nMemory Stats... in no particular order\n");
	abcMemStatsNode_c *statsWalkNode = (abcMemStatsNode_c *)statsList->getHead();
	int64_t netOutstanding = 0;
	while (statsWalkNode)
	{
		int64_t netObjs = statsWalkNode->createCount - statsWalkNode->destroyCount;
		int64_t netMem = netObjs * statsWalkNode->size;
		// print addr, class name,size file,line, function <deleted>
		fprintf(stderr,"%-15s sz:%-3d New'd: %-4lld  Del'd: %-4lld NetMemory:%lld\n",
			statsWalkNode->objTypeName,statsWalkNode->size, statsWalkNode->createCount, statsWalkNode->destroyCount,netMem);

		netOutstanding += netMem;

		statsWalkNode = (abcMemStatsNode_c *)statsWalkNode->next();
	}
	fprintf(stderr,"======== Total Outstanding Memory is %lld\n",netOutstanding);
}

void abcMemMon_c::shutdown(int shutdownVal)
{
	char netBytes[32]; abcStrings_c::snprintf_eng(netBytes,32,(char *)"%5.1f",(double)netBytesInUse,(char *)"Bytes");
	char maxBytes[32]; abcStrings_c::snprintf_eng(maxBytes,32,(char *)"%5.1f",(double)maxMemoryUsed,(char *)"Bytes");
	char totBytes[32]; abcStrings_c::snprintf_eng(totBytes,32,(char *)"%5.1f",(double)totalBytesMalloced,(char *)"Bytes");
	char freedBytes[32]; abcStrings_c::snprintf_eng(freedBytes,32,(char *)"%5.1f",(double)totalBytesFreed,(char *)"Bytes");
	char netMallocs[32]; abcStrings_c::snprintf_eng(netMallocs,32,(char *)"%5.1f",(double)netMallocsInUse,(char *)"Mallocs");
	char maxMallocs[32]; abcStrings_c::snprintf_eng(maxMallocs,32,(char *)"%5.1f",(double)maxMallocsUsed,(char *)"Mallocs");
	char totMallocs[32]; abcStrings_c::snprintf_eng(totMallocs,32,(char *)"%5.1f",(double)totalMallocCount,(char *)"Mallocs");
	char totFrees[32]; abcStrings_c::snprintf_eng(totFrees,32,(char *)"%5.1f",(double)totalFreeCount,(char *)"Mallocs");
	fprintf(stderr,"API Memory Leak Report:  Outstanding Memory: %s  Outstanding Mallocs: %s\n ",netBytes, netMallocs);
	fprintf(stderr,"Other Stats:  MaxMalloced:%s TotalMallocs:%s TotalFrees:%s\n",maxBytes,totBytes,freedBytes);
	fprintf(stderr,"Other Stats:  MaxMalloced:%s TotalMallocs:%s TotalFrees:%s\n",maxMallocs,totMallocs,totFrees);
	abcMemMon_c::printMemoryStats();
}

////////////////////////////
// EOF abcMemory.cpp
