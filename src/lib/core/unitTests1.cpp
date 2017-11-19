
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
}

#define _PRINT_PRINT_
#include "abcCore.h"



// this is for DEBUG_LTD.... to to help it work outside a proper object
void setReason(abcReason_e setReason)
{
	globalSetReason(setReason);
}

abcResult_e testMemMon()
{
	int i;

	// test the creation  and printing of the name node
	PRINT("TESTING abcMemNameNode_c\n");

	abcMemNameNode_c *tmn = new abcMemNameNode_c(1234,"T1_MemNameNode1",2);
	tmn->print();
	tmn->print(PRINT_STYLE_LIST_MEM_NAME_LIST);

	abcHashList_c *hList1 = new abcHashList_c("T1 - TestHashList-abcMemNameNode_c");
	for (i=0;i<10;i++)
	{
		char tstr[128];
		snprintf(tstr,128,"T1_MemNameNode-%d",i);
		uint64_t stringHash = abcMemNameNode_c::calcNameHash(tstr);
		abcListNode_c *t1 = new abcMemNameNode_c(stringHash,tstr,i);
		t1->print(PRINT_STYLE_LIST_MEM_NAME_LIST);
		hList1->add(t1);
	}
	hList1->print(PRINT_STYLE_LIST_MEM_NAME_LIST);

	// test the creation and printing of the stats node
	PRINT("TESTING abcMemStatsNode_c\n");

	abcListNode_c *tsn = new abcMemStatsNode_c(1234,"T2_StatsNode-32",123);
	tsn->print(PRINT_STYLE_LIST_MEM_STATS_LIST);
	tsn->print();

	abcHashList_c *hList2 = new abcHashList_c("T2_TestHashList-testStatsNode");
	for (i=0;i<10;i++)
	{
		char tstr[128];
		snprintf(tstr,128,"T2_MemNameNode-%d",i);
		uint64_t stringHash = abcMemNameNode_c::calcNameHash(tstr);
		abcMemStatsNode_c *t2 = new abcMemStatsNode_c(stringHash,tstr,1);
		t2->print(PRINT_STYLE_LIST_MEM_STATS_LIST);
		hList2->add(t2);
	}
	hList2->print(PRINT_STYLE_LIST_MEM_STATS_LIST);


	// test the creation  and printing of the mmap node
	PRINT("TESTING abcMmapNode_c\n");

	abcMmapNode_c *tmmn = new abcMmapNode_c((void *)tsn);
	tmmn->print(PRINT_STYLE_LIST_MMAP_LIST);
	tmmn->print();

	abcHashList_c *hList3 = new abcHashList_c("T3_TestHashList-testMmapNode");
	for (i=0;i<10;i++)
	{
		int64_t addrHash = (int64_t)hList3 + (1234 * i);
		void *addrPtr = (void *)addrHash;
		abcListNode_c *t3 = new abcMmapNode_c(addrPtr);
		t3->print(PRINT_STYLE_LIST_MMAP_LIST);
		hList3->add(t3);
	}
	hList3->print(PRINT_STYLE_LIST_MMAP_LIST);
	PRINT("Done with memMon node tests\n");
	PRINT("Deleting all objects\n");
	delete  tmmn;
	delete tsn;
	delete tmn;
	delete hList1;
	delete hList2;
	delete hList3;


	PRINT("TESTING abcMemMon_c\n");
	abcMemMon_c *mMon = new abcMemMon_c("testMemMon");
	char  *j1= (char *)mMon + 0x123450;
	void *returnPtr = mMon->interceptClassNew(j1, (char *)"abcJunkClass_c",sizeof(abcMemMon_c),(char *)"unitTest.cpp",89,(char *)"testFn_name");
	if (returnPtr != j1)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return ABC_FAIL;
	}
	// should fail with same address
	returnPtr = mMon->interceptClassNew(j1, (char *)"abcJunkClass_c",sizeof(abcMemMon_c),(char *)"unitTest.cpp",89,(char *)"testFn_name");
	if (returnPtr)
	{
		PRINT("Should have failed but didn't\n");
		return ABC_FAIL;
	}
	// check for error
	abcReason_e reason = mMon->getReason();
	if (reason != ABC_REASON_NONE && reason != ABC_REASON_MMAP_DUPLICATE_ADDR_INVALID)
	{
		PRINT("interceptClassNew failed reason=%d\n",reason);
		return ABC_FAIL;
	}
	PRINT("Resetting reason and continuing\n"); globalResetReason(); mMon->resetReason();

	char *j2 = j1+1;
	char *j3 = j2+1;
	returnPtr = mMon->interceptClassNew(j2, (char *)"abcJunkClass_c",sizeof(abcMemMon_c),(char *)"unitTest.cpp",89,(char *)"testFn_name");
	returnPtr = mMon->interceptClassNew(j3, (char *)"abcJunkClass_c",sizeof(abcMemMon_c),(char *)"unitTest.cpp",89,(char *)"testFn_name");
	mMon->interceptDelete(j2, (char *)"unitTest.cpp",189,(char *)"testFn_deleteSpot");
	mMon->interceptDelete(j2, (char *)"unitTest.cpp",189,(char *)"testFn_deleteSpot");

	mMon->printMemoryStats();




	delete mMon;
	return ABC_PASS;
}

abcResult_e testHashList()
{

	// make a list with 5 slices.   Fill it and print it.
	abcHashList_c *tList = new abcHashList_c("hashTestList");
	testNode_c *t1 = new testNode_c();
	t1->setName("t1");
	t1->setValue(9);
	t1->setKeyInt(9);
	abcResult_e result =tList->add(t1);		// should fail... list not initialized
	if (result != ABC_PASS)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}

	// initialize with 6(=>7) slices, rebuild threshold 10, 400% growth on rebuild
	PRINT("Adding Single Node to list\n");
	result = tList->addSliceTail(6,t1);		// should fail... disabled method
	if (result != ABC_FAIL)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->resetReason(); PRINT("resetting expected error condition at %s:%d\n",__FILE__,__LINE__);
	tList->addTail(t1);		// should fail...method defeated
	if (result != ABC_FAIL)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->resetReason(); PRINT("resetting expected error condition at %s:%d\n",__FILE__,__LINE__);
	result = tList->add(t1);		
	if (result != ABC_FAIL)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->resetReason(); PRINT("resetting expected error condition at %s:%d\n",__FILE__,__LINE__);
	tList->abcSlicedList_c::print();

	int i;;
	for (i=0;i<100;i++)
	{
		testNode_c *t2= new testNode_c  ();
		//testNode_c *fn = NULL;
		char name[32];
		snprintf(name,32,"v%d",i);
		t2->setValue(i);
		t2->setKeyInt(i);
		t2->setName(name);
		result = tList->add(t2);
		if (result != ABC_PASS)
		{
			ERROR_G(ABC_REASON_TEST_FAILED);
			return(ABC_FAIL);
		}
	} // finished building list

	tList->abcSlicedList_c::print();


	PRINT("testing find\n");
	for (i=0;i<31;i+=2)
	{
		nodeKey_s sKey;
		nodeKey_setInt(&sKey,i);
		t1 = (testNode_c *)tList->findFirst(&sKey);
		if (!t1)
		{
			WARNING_G(ABC_REASON_TEST_FAILED);
			return(ABC_FAIL);
		}
		t1->print();
	}

#if 0
	PRINT("printing Test List\n");
	tList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);
#endif

	PRINT("Cloning hashedList\n");
	abcHashList_c *cList = (abcHashList_c *)tList->clone();
	if (!cList)
	{
		WARNING_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}

	PRINT("Deleting original\n");
	delete tList;

	PRINT("testing find on cloned list\n");
	for (i=0;i<31;i+=2)
	{
		nodeKey_s sKey;
		nodeKey_setInt(&sKey,i);
		t1 = (testNode_c *)cList->findFirst(&sKey);
		if (!t1)
		{
			WARNING_G(ABC_REASON_TEST_FAILED);
			return(ABC_FAIL);
		}
		t1->print();
	}

	//
	PRINT("Test Pull Head\n");



	//abcListNode_c *dn;
	// pull 16 nodes [ slice 2 + 1 from slice 0 ]
	for (i=0;i<14;i++)
	{
		abcListNode_c *dn = cList->pullHead();	// should be the head of slice 3
		dn->print();
		delete dn;
	}
	
	// add back a slice 2 @ value = 72
	t1 = new testNode_c();
	t1->setValue(72);
	t1->setKeyInt(72);
	t1->setName("Late add");
	cList->add(t1);

#if 0
	PRINT("printing cloned Hash list\n");
	cList->abcSlicedList_c::print();
#endif

	PRINT("Deleting the clone\n");
	delete cList;

	PRINT("Done with HashList test\n");
	return ABC_PASS;
}
abcResult_e testSlicedList()
{
	int i;

	// make a list with 5 slices.   Fill it and print it.
	abcSlicedList_c *tList = new abcSlicedList_c("slicedTestList");
	testNode_c *t1 = new testNode_c();
	t1->setName("t1");
	abcResult_e result =tList->addSliceTail(1,t1);		// should fail... list not initialized
	if (result != ABC_FAIL)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->resetReason(); PRINT("resetting error condition\n");
	tList->init(6);
	tList->addSliceTail(6,t1);		// should fail... slice# out of range
	if (result != ABC_FAIL)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->resetReason(); PRINT("resetting error condition\n");
	tList->addTail(t1);		// should fail...method defeated
	if (result != ABC_FAIL)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->resetReason(); PRINT("resetting error condition\n");
	tList->addSliceTail(3,t1);		

	// fix up t1 for later searching
	t1->setValue(99);
	t1->setKeyInt(99);

	// repeatable but odd add patter for testing addHead, addTail, addMiddle and to different slices.
	char name[32];
	int sliceMap[30] = {0,1,1,1,3,3,3,3,5,5,5,4,3,2,1,0,0,2,4,5,5,2,3,1,4,2,4,4,0,2};
	for (i=0;i<30;i++)
	{
		testNode_c *t2= new testNode_c  ();
		testNode_c *fn = NULL;
		int slice= sliceMap[i];
		snprintf(name,32,"s%d v%d",slice,i);
		t2->setValue(i);
		t2->setKeyInt(i);
		t2->setName(name);
		switch(i & 3)
		{
			case 0:
				fprintf(stderr,"Adding Head for slice %d value %d\n",slice,i);
				tList->addSliceHead(slice,t2);
				break;
			case 1:
				fprintf(stderr,"Adding Tail for slice %d value %d\n",slice,i);
				tList->addSliceTail(slice,t2);
				break;
			case 2:
				fprintf(stderr,"Adding Middle for slice %d value %d\n",slice,i);
				fn=(testNode_c *)tList->getSliceHead(slice);
				tList->addSliceMiddle(slice,t2,fn);
				break;
			case 3:
				fprintf(stderr,"Adding Middle for slice %d value %d\n",slice,i);
				fn=(testNode_c *)tList->getSliceTail(slice);
				tList->addSliceMiddle(slice,t2,fn);
				break;
		}
	} // finished building list

	PRINT("printing Test List\n");
	tList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	abcSlicedList_c *t2List = (abcSlicedList_c *)tList->clone();
	if (t2List == NULL)
	{
		ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	t2List->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	PRINT("Deleting Test SlicedList\n");
	delete tList;

	// test remove some
	// int sliceMap[30] = {0,1,1,1,3,3,3,3,5,5,5,4,3,2,1,0,0,2,4,5,5,2,3,1,4,2,4,4,0,2};
	// slice counts =>  0:4  1:5  2:6  3:7  4:4 5:5

	//abcListNode_c *dn;
	// remove slice 3
	for (i=0;i<7;i++)
	{
		abcListNode_c *dn = t2List->pullHead();	// should be the head of slice 3
		delete dn;
	}
	t1 = new testNode_c();
	t1->setName("slice3 at tail");
	t1->setValue(99);
	t2List->addSliceHead(3,t1);		// should end up at tail of list

	// now lets remove some from the middle.
	testNode_c *t0 = (testNode_c *)t2List->getSliceHead(5);
	if (!t1)
	{
		ERROR_G(ABC_REASON_NODE_PARAM_IS_NULL);
		exit(1);
	}
	PRINT("Deleting some nodes..... slice 3 nodes 2 & 3\n");
	t1 = (testNode_c *)t0->next();	// t1 sb 2 of 5 
	testNode_c *t2 = (testNode_c *)t1->next();	// t2 sb 3 of 5
	PRINT("read... go!\n");
	t2List->remove(t1);
	t2List->remove(t2);
	delete t1;
	delete t2;
	t2List->remove(t0);
	delete t0;

	// now use slice2 to test findSliceFirst and findSliceNext
	// lets fail first, then find first on list(13) then 3rd as next
	nodeKey_s testKey;
	nodeKey_setInt(&testKey,13);
	PRINT("looking for 13 and expecting to fail\n");
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey);
	if (t1)
	{
		PRINT("Wasn't upposed to find anything\n");
		exit(1);
	}
	PRINT("Failed as expected\n");
	PRINT("looking for 24 and expecting to find\n");
	nodeKey_setInt(&testKey,24); // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey);
	t1->print();
	if (!t1)
	{
		PRINT("Wasn't supposed to fail\n");
		exit(1);
	}
	PRINT("looking for 26 and expecting to succeed\n");
	nodeKey_setInt(&testKey,26); // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceNext(4,&testKey,t1);
	if (!t1)
	{
		PRINT("Wasn't supposed to fail\n");
		exit(1);
	}
	PRINT("passed as expected\n");
	t1->print();

	PRINT("Now testing searching backwards\n");
	PRINT("looking for 19 and expecting to fail\n");
	nodeKey_setInt(&testKey,19); 
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey,FALSE/*backwards*/);
	if (t1)
	{
		PRINT("Wasn't upposed to find anything\n");
		exit(1);
	}
	PRINT("Failed as expected\n");
	PRINT("looking for 27 and expecting to find\n");
	testKey.value.intgr = 27; // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey,FALSE);
	if (!t1)
	{
		PRINT("Wasn't supposed to fail\n");
		exit(1);
	}
	t1->print();
	PRINT("looking for 27 again and  expecting to fail\n");
	nodeKey_setInt(&testKey,27); 
	testNode_c *t1a = (testNode_c *)t2List->findSliceNext(4,&testKey,t1,FALSE);
	if (t1a)
	{
		PRINT("Didn't expect to find\n");
		exit(1);
	}
	PRINT("Failed as expected\n");
	PRINT("looking for 18 and expecting to succeed\n");
	nodeKey_setInt(&testKey,18); 
	t1 = (testNode_c *)t2List->findSliceNext(4,&testKey,t1,FALSE);
	if (!t1)
	{
		PRINT("Wasn't supposed to fail\n");
		exit(1);
	}
	PRINT("passed as expected\n");
	t1->print();

	PRINT("looking for 24 and expecting to succeed\n");
	testKey.value.intgr = 24; // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceNext(4,&testKey,t1,FALSE);
	if (!t1)
	{
		PRINT("Wasn't supposed to fail\n");
		exit(1);
	}
	PRINT("passed as expected\n");
	t1->print();




	PRINT("\n\nDone testing slice find\n");









	t2List->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);
	PRINT("Deleting Clone SlicedList\n");
	delete t2List;



	#if 0
	if (tList->getSliceNodeCount(slice+1))
	{
		tList->addSliceTail(slice+1,t2);		// should fail...method defeated
		if (result != ABC_FAIL)
		{
			ERROR_G(ABC_REASON_TEST_FAILED);
			return(ABC_FAIL);
		}
		tList->resetReason(); PRINT("resetting error condition\n");
	}
	#endif

	return ABC_PASS;
}

abcResult_e testPrimes()
{
	//globalCore->initPrimes(3600);
	globalCore->printPrimes(1,20);
	globalCore->updatePrimes(8000);

	int primeTry=200;
	int  myPrime = globalCore->findPrime(primeTry);
	PRINT("my prime try:%d  number is %d\n",primeTry, myPrime);
	primeTry=104729;
	myPrime = globalCore->findPrime(primeTry);
	PRINT("my prime try:%d  number is %d\n",primeTry, myPrime);
	globalCore->printPrimes(104700,104750);
	globalCore->printPrimes(1,20);
	//globalCore->initPrimes(10000);
	//globalCore->initPrimes(1000000);
	return ABC_PASS;
}
abcResult_e testCrc()
{
    globalCore->generateCrc32Table();
    globalCore->generateCrc64Table();

	PRINT("Testing CRC32\n");
	uint8_t  *t9=(uint8_t *)"The quick brown fox jumps over the lazy dog";
	uint32_t ref1=0x414FA339;
	int t9l= strlen((char *)t9);
	uint32_t r1 = globalCore->computeCrc32(0,t9,t9l);
	PRINT("CRC32..... seed 0, string=\"%s\" crc32=%08x ref=%08x diff=%d\n",t9,r1,ref1,r1-ref1);

	t9=(uint8_t *)"123456789";
	t9l= strlen((char *)t9);
	r1 = globalCore->computeCrc32(0,t9,t9l);
	ref1=0xCBF43926;
	PRINT("CRC32..... seed 0, string=\"%s\" crc32=%08x ref=%08x diff=%d\n",t9,r1,ref1,r1-ref1);


    uint64_t  result;
	typedef unsigned char* PBYTE ;

	unsigned char t1[8]; t1[0]=0x80;
    printf("taking CRC64 of \"\x80\" (should be 0xC96C5795D7870F42)\n");
    result = globalCore->computeCrc64(t1, 1);
	printf("result0: %016llx\n", result);

    printf("taking CRC64 of \"\\xDE\\xAD\\xBE\\xEF\" (should be FC232C18806871AF)\n");
    result = globalCore->computeCrc64((PBYTE)"\xDE\xAD\xBE\xEF", 4);
    printf("result: %016llxX\n", result);

    printf("taking CRC64 of \"99eb96dd94c88e975b585d2f28785e36\" (should be DB7AC38F63413C4E)\n");
    result = globalCore->computeCrc64((PBYTE)"\x99\xEB\x96\xDD\x94\xC8\x8E\x97\x5B\x58\x5D\x2F\x28\x78\x5E\x36", 16);
    printf("result: %016llx\n", result);

    printf("taking CRC64 of \"\\DE\\xAD\" (should be 44277F18417C45A5\n");
    result = globalCore->computeCrc64((PBYTE)"\xDE\xAD", 2);
    printf("result: %016llx\n", result);

	return ABC_PASS;
}


abcResult_e timeSortedListTest(int nodeCount, int loopCount)
{
	abcTime1m_t setupStartTime1m = getTime1m();

	// init everything and build list
	abcList_c *testList = new abcList_c("timeSortedList");
	testNode_c *timeNode;
	abcTime1m_t now1m;

	int i;
	for (i=0;i<nodeCount;i++)
	{
		now1m = getTime1m();
		char tmpStr[128];
		snprintf(tmpStr,128,"record-%d",i);
		timeNode = new testNode_c();
		timeNode->setValue(i);
		timeNode->setName(tmpStr);
		timeNode->setKeyInt(i);
		testList->addSorted(timeNode,FALSE);	// add from tail;
	}
	abcTime1m_t initComplete1m = getTime1m();
	abcTime1m_t setupTime1m = initComplete1m - setupStartTime1m;
	PRINT("setup time: %6.2f milliseconds for %d nodes or (%7.6f uSec/node) \n", (double)setupTime1m/1000.0,nodeCount, ((double)setupTime1m/(double)nodeCount));


	int64_t runningTail = nodeCount;


	abcTime1m_t testStartTime1m = getTime1m();
	for (i=0;i<loopCount;i++)
	{
		timeNode = (testNode_c *)testList->pullHead();
		if (!timeNode)
		{
			ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
			exit(1);
		}

		runningTail++;
		int64_t newKeyVal = runningTail - (int64_t)((double)nodeCount * randomPercentage());
		timeNode->setKeyInt(newKeyVal);
		timeNode->setValue(newKeyVal);
		testList->addSorted(timeNode,FALSE);
	}

	abcTime1m_t testStopTime1m = getTime1m();
	abcTime1m_t runTime1m = testStopTime1m - testStartTime1m;
	PRINT("Run time: %6.2f milliseconds for %d loops or (%7.6f uSec/sortedAdd) \n", (double)runTime1m/1000.0,loopCount,((double)runTime1m/(double)loopCount));

	delete testList;	// also deletes all listNodes

	return ABC_PASS;
} // end abcResult_e timeSortedListTest(int nodeCount, int loopCount)

abcResult_e listTests()
{
	PRINT("TESTING LISTS\n");

	// make a new list
	abcList_c *tList = new abcList_c("testList");
	
	// add things to tail
	int i;
	for (i=0;i<10;i++)
	{
		testNode_c *tn = new testNode_c();

		tn->setValue(i);
		char tmpStr[128];
		if (i & 1 )
		{
			snprintf(tmpStr,128,"testStr%d",i);
		}
		else
		{
			snprintf(tmpStr,128,"TESTSTR%d",i);
		}
		tn->setName(tmpStr);


		int keyStyle=(i%4) + 1;
		switch(keyStyle)
		{
			case 1:
				tn->setKeyInt(i);
				break;
			case 2:
				tn->setKeyDbl((double)i *1.16);
				break;
			case 3:
				tn->setKeyPtr(tn);
				break;
			case 4:
				tn->setKeyString(tmpStr);
				break;
		}


		abcResult_e addTailResult = tList->addTail(tn);

		if ( addTailResult == ABC_FAIL) // don't expect an error
		{
			fprintf(stderr,"expected result ABC_PASS but got %s\n",abcResultAsStr(addTailResult));
			ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
			exit(1);
		}
		if (i == 0)
		{
			abcResult_e aResult = tList->addHead(tn);
			if ( aResult != ABC_FAIL )	// had better fail !
			{
				fprintf(stderr,"expected result ABC_FAIL but got %s\n",abcResultAsStr(aResult));
				ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
				exit(1);
			}
			PRINT("No problem... previous error  expected... clearing error condition at %s:%d\n",__FILE__,__LINE__);
			tList->resetReason();	// reset the error we intentionally caused
		}
	}
	PRINT("generating list print\n");
	//tList->print(1,1);

	PRINT("Cloning the list\n");
	abcList_c *cloneList = tList->clone();

	PRINT("Printing the Clone \n");
	cloneList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	// test the stateless calls
	if (tList->getHead() !=  tList->head())
	{
		PRINT("FAIL getHead != head \n");
		exit(1);
	}

	if (tList->getTail() !=  tList->tail())
	{
		PRINT("FAIL getTail != tail \n");
		exit(1);
	}

	PRINT("deleting the clone and emptying the testList\n");
	delete cloneList;
	tList->empty();

	PRINT("preparing list for find test\n");
	int j=0;
	for (i=0;i<18;i++)
	{
		j = (j+1) %7;
		testNode_c *tn = new testNode_c();
		tn->setValue(i);
		tn->setKeyInt(j);
		if (i < 9)
		{
			tn->setName("addTail");
			tList->addTail(tn);
		}
		else
		{
			tn->setName("addHead");
			tList->addHead(tn);
		}
	}

	PRINT("Building search key\n");
	nodeKey_s sKey;
	sKey.type = KEYTYPE_INT;
	sKey.value.intgr = 5;
	testNode_c *fn = (testNode_c *)tList->findFirst(&sKey,1/*towardsNext*/);
	while (fn)
	{
		PRINT("ADDING %d in the middle\n",i);
		testNode_c *tt = new testNode_c();
		tt->setValue(i);
		tt->setKeyInt(j);
		tt->setName("addMiddle");
		tList->addMiddle(tt,fn);
/*
		abcResult_e  resultAm = tList->addMiddle(tt,fn);
		if (resultAm != ABC_FAIL)
		{
			ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
			exit(1);
		}
*/
		i++;
		j = (j+1)%7;
		fn = (testNode_c *)tList->findNext(&sKey,fn);
	}

	tList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	PRINT("Testing find/findNext\n");

	PRINT("scanning fowards\n");
	sKey.value.intgr = 3;
	fn = (testNode_c *)tList->findFirst(&sKey,1/*towardsNext*/);
	while (fn)
	{
		fn->print();
		fn = (testNode_c *)tList->findNext(&sKey,fn);
	}
	PRINT("scanning backwards\n");
	fn = (testNode_c *)tList->findFirst(&sKey,0/*towardsNext*/);
	while (fn)
	{
		fn->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);
		fn = (testNode_c *)tList->findNext(&sKey,fn,0);
	}

	PRINT("BEFORE EMPTY\n");
	tList->empty();
	PRINT("AFTER EMPTY\n");

	int valueMap[25]={1,3,5,2,4,6,9,8,7,9,24,23,22,9,20,10,11,12,13,15,14,16,17,19,18};
	PRINT("TESTING Sorted add\n");
	testNode_c *tn;
	for (i=0;i<25;i++)
	{
		tn = new testNode_c();
		j = valueMap[i];
		tn->setValue(i);
		tn->setKeyInt(j);

		tList->addSorted(tn,TRUE);
	}
	tList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	
	PRINT("DONE TESTING LISTS\n");
	return ABC_PASS;
}


//
// here we test node basics including clone for abcNode_c, abcListNode_c and testNode_c
//
abcResult_e test2_node()
{
	int checkTestFailures = 0;//	count up failuers seen inside checkTest Macro

	fprintf(stderr,"Test 2 - abcNode_c basic testing\n");
	globalConfig->printErrors = FALSE;
	globalConfig->printWarnings = FALSE;
	globalConfig->printDebug = FALSE;
	globalConfig->printPrint = FALSE;

	// test 2.1  >  null pointers in abcNode_c
	abcNode_c *node = new abcNode_c();
	char *type = node->getObjType();
	CHECK_TEST("2.1", (strcmp(type,"abcNode_c") == 0),checkTestFailures);

	char *name = node->getObjName();
	CHECK_TEST("2.1.2", ( name == NULL),checkTestFailures);

	abcNode_c *ptr = node->next();
	CHECK_TEST("2.1.3", ( ptr == NULL),checkTestFailures);

	ptr = node->prev();
	CHECK_TEST("2.1.4", ( ptr == NULL),checkTestFailures);
	delete node;

	// test 2.2  >  abcListNode_c basics
	abcListNode_c *ln = new abcListNode_c();
	type = ln->getObjType();
	CHECK_TEST("2.2.1", (strcmp(type,"abcListNode_c") == 0),checkTestFailures);

	name = ln->getObjName();
	CHECK_TEST("2.2.2", ( name == NULL),checkTestFailures);

	// test 2.3  >  abcListNode_c nodeKey
	nodeKey_s testKey;
	nodeKey_s *tkp = &testKey;
	nodeKey_init(tkp);
	nodeKey_setInt(tkp,40);
	ln->setKeyInt(32);
	CHECK_TEST("2.3.1.1", ln->diffKey(tkp) == ABC_GREATER_THAN,checkTestFailures);
	nodeKey_setInt(tkp,30);
	CHECK_TEST("2.3.1.2", ln->diffKey(tkp) == ABC_LESS_THAN,checkTestFailures);
	nodeKey_setInt(tkp,32);
	CHECK_TEST("2.3.1.3", ln->diffKey(tkp) == ABC_EQUAL,checkTestFailures);
	ln->setKeyDbl(32.0);	// test mismatch ... should cause err... don't let it abort

	int tmpPrint = globalConfig->printErrors;
	//globalConfig->printErrors = TRUE;
	CHECK_TEST("2.3.1.4", ln->diffKey(tkp) == ABC_ERROR,checkTestFailures);
	globalConfig->printErrors = tmpPrint;

	// moving to strings.. only going to test simple strings (strdup'd') 
	// cloning with memory tracking will make it more testable, but we don't have memory tracking here
	//
	//  Note:  changing keyTypes is dangerous !!! don't do it without a destroy
	nodeKey_destroy(tkp);	// really just a clearing out
	char *extString1 = (char *)"ab5";
	char *extString2 = (char *)"ab6";
	ln->setKeyString(extString1);
	nodeKey_setString(tkp,extString2);
	//globalConfig->printPrint = TRUE;
	//PRINT("strcmp(keyonly:%s,node:%s) = %s\n",tkp->value.string,ln->getKeyString(),abcResultAsStr(ln->diffKey(tkp)));
	CHECK_TEST("2.3.2.1", ln->diffKey(tkp) == ABC_GREATER_THAN,checkTestFailures);
	nodeKey_setString(tkp,"ab4");
	//PRINT("strcmp(keyonly:%s,node:%s) = %s\n",tkp->value.string,ln->getKeyString(),abcResultAsStr(ln->diffKey(tkp)));
	CHECK_TEST("2.3.2.2", ln->diffKey(tkp) == ABC_LESS_THAN,checkTestFailures);
	nodeKey_setString(tkp,"ab5");
	CHECK_TEST("2.3.3.3", ln->diffKey(tkp) == ABC_EQUAL,checkTestFailures);

	// test 2.3  listNode cloning
	abcListNode_c *lnClone = ln->clone();		
	CHECK_TEST("2.4.1", (lnClone != NULL), checkTestFailures);
	if (lnClone != NULL)
	{
		// test 2.4.1.1  >  null pointers in abcNode_c
		char *type = lnClone->getObjType();
		CHECK_TEST("2.4.2", (strcmp(type,"abcListNode_c") == 0),checkTestFailures);

		char *name = lnClone->getObjName();
		CHECK_TEST("2.4.2", ( name == NULL),checkTestFailures);

		abcNode_c *ptr = lnClone->next();
		CHECK_TEST("2.4.3", ( ptr == NULL),checkTestFailures);

		ptr = lnClone->prev();
		CHECK_TEST("2.4.4", ( ptr == NULL),checkTestFailures);

		// check the clone against the original
        //globalConfig->printPrint = TRUE;
		//abcResult_e diffRes = lnClone->diffKey(ln);
        //PRINT("strcmp(clone:%s,orig:%s) = %s\n",lnClone->getKeyString(),ln->getKeyString(),abcResultAsStr(diffRes));
		CHECK_TEST("2.4.5.1", lnClone->diffKey(ln) == ABC_EQUAL,checkTestFailures);

		// moving to strings.. only going to test simple strings (strdup'd') 
		// cloning with memory tracking will make it more testable, but we don't have memory tracking here
		//
		//  Note:  changing keyTypes is dangerous !!! don't do it without a destroy
		nodeKey_destroy(tkp);
		char *extString2 = (char *)"ab6";
		nodeKey_setString(tkp,extString2);
		//globalConfig->printPrint = TRUE;
		//PRINT("strcmp(keyonly:%s,node:%s) = %s\n",tkp->value.string,ln->getKeyString(),abcResultAsStr(ln->diffKey(tkp)));
		CHECK_TEST("2.4.5.2", lnClone->diffKey(tkp) == ABC_GREATER_THAN,checkTestFailures);
		nodeKey_setString(tkp,"ab4");
		//PRINT("strcmp(keyonly:%s,node:%s) = %s\n",tkp->value.string,ln->getKeyString(),abcResultAsStr(ln->diffKey(tkp)));
		CHECK_TEST("2.4.5.3", lnClone->diffKey(tkp) == ABC_LESS_THAN,checkTestFailures);
		nodeKey_setString(tkp,"ab5");
		CHECK_TEST("2.4.5.4", lnClone->diffKey(tkp) == ABC_EQUAL,checkTestFailures);
	}
	delete ln;
	delete lnClone;
	nodeKey_destroy(tkp);

	// test 2.5  >  testng a testNode_c [ a subclass of abcListNode_c 
	char *tnName=(char *)"unitTest1_testNode";
	testNode_c *tn = new testNode_c(tnName);
	type = tn->getObjType();
	CHECK_TEST("2.5.1", (strcmp(type,"testNode_c") == 0),checkTestFailures);

	name = tn->getObjName();	// make the standard... take is just the point,  storing does the strdup
	CHECK_TEST("2.5.2", ( strcmp(tnName,name) == 0),checkTestFailures);
	
	tn->setValue(123);
	tn->setKeyInt(456);
	nodeKey_setInt(tkp,456);
	CHECK_TEST("2.5.3", tn->diffKey(tkp) == ABC_EQUAL,checkTestFailures);
	abcListNode_c *tnClone = tn->clone();
	CHECK_TEST("2.6.1", (tnClone != NULL) ,checkTestFailures);
	CHECK_TEST("2.6.2", tnClone->diffKey(tkp) == ABC_EQUAL,checkTestFailures);

	// clean up
	nodeKey_destroy(tkp);
	delete tn;
	delete tnClone;


	/////////////////// done with printing testing !!!
	fprintf(stderr,"%s:  %d failures\n",__FUNCTION__,checkTestFailures);
	return (checkTestFailures ? ABC_FAIL:ABC_PASS);
} // end test2_node



// a test for the debug printing capabilities.
// These require user attention.... sorry !!
abcResult_e test1_printing()
{
	fprintf(stderr,"Test 1 - Debug printing Test.  Manually confirm questions\n");
	int checkTestFailures = 0;//	count up failuers seen inside checkTest Macro

	// test 1.1 - test PRINT macro and globalConfig control over it
	globalConfig->printPrint = TRUE;
	fprintf(stderr,"Do you see \"PrintingEnabled\" inside the brackets.... [");
	PRINT(" PrintingEnabled ");
	fprintf(stderr,"] (Y/n)");
	char inStr[128]; fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.1",inStr,'Y',checkTestFailures);

	// test 1.2
	globalConfig->printPrint = FALSE;
	fprintf(stderr,"Do you see \"PrintingEnabled\" inside the brackets.... [");
	PRINT(" PrintingEnabled ");
	fprintf(stderr,"] (y/N)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.2",inStr,'N',checkTestFailures);

	// test 1.3
	globalConfig->printPrint = TRUE;
	int i;
	fprintf(stderr,"Do you see \"1 2 3 5 10 15\" inside the brackets.... [");
	for (i=1;i<=15;i++)
	{
		PRINT_LTD(3,5,"%d ",i);
	}
	fprintf(stderr,"] (Y/n)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.3",inStr,'Y',checkTestFailures);

	// test 1.4 test PRINT_LTD
	globalConfig->printPrint = FALSE;
	fprintf(stderr,"Do you see \"1 2 3 5 10 15\" inside the brackets.... [");
	for (i=1;i<=15;i++)
	{
		PRINT_LTD(3,5,"%d ",i);
	}
	fprintf(stderr,"] (y/N)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.4",inStr,'Y',checkTestFailures);
	globalConfig->printPrint = TRUE;

	// test 1.5 test WARNING
	
	globalConfig->printWarnings = TRUE;
	fprintf(stderr,"Below, You should see JUST ONE line with the text \"WARNING! Reason=ABC_REASON_TEST_FAILED in method test1_printing at unitTest.cpp:xxx\"\n");
	globalResetReason();
	WARNING(ABC_REASON_TEST_FAILED); // setReason mapped to globalSetReason for in this test
	CHECK_TEST("1.5.1",(globalCore->getReason() == ABC_REASON_TEST_FAILED),checkTestFailures);

	globalConfig->printWarnings = FALSE;
	globalResetReason();
	WARNING(ABC_REASON_TEST_FAILED);
	CHECK_TEST("1.5.2",(globalCore->getReason() == ABC_REASON_TEST_FAILED),checkTestFailures);	// reason didn't get set properly
	fprintf(stderr,"Confirm (Y/n)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.5",inStr,'Y',checkTestFailures);

	// test 1.6 test WARNING_LTD
	globalResetReason();
	globalConfig->printWarnings = TRUE;
	fprintf(stderr,"Below, You should see several lines with the text \"WARNING(#)! Reason=ABC_REASON_TEST_FAILED in method test1_printing at unitTest.cpp:xxx\" N={4,5,10,15}\n");
	for (i=1;i<=15;i++)
	{
		globalConfig->printWarnings = (i<4) ? FALSE:TRUE ;
		WARNING_LTD(4,5,ABC_REASON_TEST_FAILED);	// printEnable tested after the filter is run
	}
	fprintf(stderr,"Confirm (Y/n)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.6",inStr,'Y',checkTestFailures);
	CHECK_TEST("1.6.1",(globalCore->getReason() == ABC_REASON_TEST_FAILED),checkTestFailures);	// reason didn't get set properly
	globalConfig->printWarnings = TRUE;

	// test 1.7 test DEBUG
	globalConfig->printDebug = TRUE;
	fprintf(stderr,"Do you see \" [DEBUG_MESSAGE] in method test1_printing at unitTest1.cpp:xxx\"\n");
	DEBUG(" [DEBUG_MESSAGE] ");
	globalConfig->printDebug = FALSE;
	DEBUG(" [DEBUG_MESSAGE] ");
	fprintf(stderr,"Confirm you see the message JUST ONCE (Y/n)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.7",inStr,'Y',checkTestFailures);
	globalConfig->printDebug = TRUE;

	// test 1.8 test ERROR
	globalConfig->printErrors = TRUE;
	globalConfig->abortErrors = FALSE;
	fprintf(stderr,"Below, You should see TWO line with the text \"ERROR! Reason=ABC_REASON_TEST_FAILED in method test1_printing at unitTest.cpp:xxx\"\n");

	globalResetReason();
	ERROR(ABC_REASON_TEST_FAILED);
	int test181 = (globalCore->getReason() == ABC_REASON_TEST_FAILED);
	globalResetReason();

	globalResetReason();
	globalConfig->printErrors = FALSE;
	ERROR(ABC_REASON_TEST_FAILED);
	int test182 = (globalCore->getReason() == ABC_REASON_TEST_FAILED);

	globalResetReason();
	globalConfig->printErrors = TRUE;
	ERROR(ABC_REASON_TEST_FAILED);
	int test183 = (globalCore->getReason() == ABC_REASON_TEST_FAILED);

	fprintf(stderr,"Confirm (Y/n)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.8",inStr,'Y',checkTestFailures);

	CHECK_TEST("1.8.1",test181,checkTestFailures);	// check if reason didn't get set properly
	CHECK_TEST("1.8.2",test182,checkTestFailures);	// check if reason didn't get set properly
	CHECK_TEST("1.8.3",test183,checkTestFailures);	// check if reason didn't get set properly


	// test 1.9 test ERROR_LTD
	//globalConfig->abortErrors = TRUE; can't use this because it aborts the test
	fprintf(stderr,"Below, you should see 4 lines with the text \"Error(#): Reason=ABC_REASON_TEST_FAILED in method test1_printing at unitTest.cpp:xxx\" #={1,2,3,4}\n");
	for (i=1;i<=15;i++)
	{
		ERROR_LTD(4,ABC_REASON_TEST_FAILED);	// printEnable tested after the filter is run
	}
	fprintf(stderr,"Confirm (Y/n)"); fgets(inStr,128,stdin);
	TTY_CHK_TEST("1.6",inStr,'Y',checkTestFailures);


	/////////////////// done with printing testing !!!
	fprintf(stderr,"%s:  %d failures\n",__FUNCTION__,checkTestFailures);
	return (checkTestFailures ? ABC_FAIL:ABC_PASS);
}
/*
abcResult_e nodeTests()
{
	PRINT("TESTING NODES\n");
	PRINT("building a ListNode\n");
	abcListNode_c *le = new abcListNode_c();
	PRINT("destroying a ListNode\n");
	delete le;
	PRINT("FINISHED TESTING NODES\n");
	return ABC_PASS;
}
*/


#if 0

	abcListNode_c *le = new abcListNode_c();
	fprintf(stderr," === \n");
	abcNode_c *otherNode = le;
	abcNode_c *base = new abcNode_c();
	le->setKeyInt(123);
	le->setKeyString("Hello Fred");


	abcListNode_c *le_clone = le->clone();


	abcList_c *myList = new abcList_c("TestList");
	abcList_c *cloneList = myList->clone();

	fprintf(stderr," === deleting le\n");
	delete otherNode;
	fprintf(stderr," === deleting base\n");
	delete base;
	fprintf(stderr," === done \n");

	
	btest_c *bto = new btest_c("Joe");
	bto->puta(123);
	bto->putb(456000);
	bto->display();
	printf("bto= %llx\n",(int64_t)bto);

	delete bto;
#endif

/*
class btest_c
{
  private:
  	int64_t a;
	int64_t b;
	char	*name;
  protected:

  public:
  	btest_c(const char *str); // {  name=strdup(str); }
	~btest_c(); //{ free (name); }

  	void puta(int64_t setval) {  a = setval; }
  	void putb(int64_t setval) {  b = setval; }
	void display() { printf("%s a=%lld b=%lld\n",name,a,b); }
	int64_t getaplusb() { return a+b; }
};

btest_c::btest_c(const char *str)
{
	name=strdup(str); 
}
btest_c::~btest_c()
{
	free (name); 
}
*/

////////////////////////////////////////////////////////////////////////////
//
//  THE TEST MAIN PROGRAM and its infrastructure
//
////////////////////////////////////////////////////////////////////////////
static struct option testOpts[] =
{
	{ "quiet"/*longCmd*/,0 /* has_arg*/,NULL,'q'/*shortCmd*/ },
	{ "logfile"/*longCmd*/,1 /* has_arg*/,NULL,'l'/*shortCmd*/ },
	{ NULL, 0, NULL, 0 }
};

// parsed command line argument storage
static struct cmdArgs
{
	uint8_t	errorsFatal;
	uint8_t	quietMode;
	char	*logFile;
	FILE	*logFp;
} globalArgs;

void setDefaults()
{
	memset(&globalArgs,0,sizeof(cmdArgs));
	globalArgs.quietMode = FALSE;
	globalArgs.logFile   = NULL;
	globalArgs.logFp = NULL;

}

void usage(int argc, char **argv)
{
	fprintf(stderr,"Usage  %s [-opt <val> || --longOpt <val> ]...\n",argv[0]);
	int i=0;
	char *descr;
	while (testOpts[i].name != NULL)
	{
		switch (i)
		{
			case 'q':
				descr=(char *)"Run Quietly      ";
				break;
			case 'l':
				descr=(char *)"log file         ";
				break;
			default:
				descr=(char *)"No description   ";
		}
		if (testOpts[i].has_arg)
		{
			fprintf(stderr,"-%c --%-10s Value   =>  %s\n",testOpts[i].val,testOpts[i].name,descr);
		}
		else
		{
			fprintf(stderr,"-%c --%-10s        =>  %s\n",testOpts[i].val,testOpts[i].name,descr);
		}
		i++;
	}
}


int main(int argc, char **argv)
{
	// This test doesn't use the memory tracker.  Instead it tests the 
	// components that need to work for the memory tracker to work, and core things that don't use it
	// Prime number generation and key crc generatotrs are examples.
	//
	// Those are basically hash list and node..  & keyHash, prime, 
	// 
	globalCore = new abcCore_c();
	abcResult_e initStatus = globalCore->init(1);
	globalConfig = globalCore->getConfig();

	// manual defualt configuration after globalInit
	globalConfig->abortErrors = FALSE;
	
	if (initStatus != ABC_PASS)
	{
		ERROR_G(ABC_REASON_GLOBAL_INIT_FAILED);
		exit(1);
	}
	srand(1);		// will be a fixed random sequence

	// parse command line.  arg0 = programName
	char cmdChar;
	while ((cmdChar = getopt_long(argc, argv, "l:qFv", testOpts, NULL)) != -1)
	{
		switch (cmdChar)
		{
			case 'q':
				globalConfig->printWarnings = FALSE;
				globalConfig->printErrors = FALSE;
				globalConfig->printPrint = FALSE;
				globalConfig->printDebug = FALSE;
				break;

			case 'v':
				globalConfig->printWarnings = TRUE;
				globalConfig->printErrors = TRUE;
				globalConfig->printPrint = FALSE;
				globalConfig->printDebug = FALSE;
				break;

			case 'F':
				// make errors fatal (and hence abort)
				globalConfig->abortErrors = TRUE;
				break;
				
			case 'l':
				globalArgs.logFile = strdup(optarg);
				globalArgs.logFp = fopen(globalArgs.logFile,"wo");
				if (globalArgs.logFp == NULL)
				{
					perror("Can't open log file");
					exit(1);
				}
				break;
			default:
				usage(argc,argv);
				exit(1);
		}
	}

	// go through the tests
	abcResult_e result;

	// build a node then a listNode
//	result = test1_printing();
	result = test2_node();
	//result = listTests();
	//result = timeSortedListTest(10000, 1000);
	//result = testCrc();
	//result = testPrimes();
	//result=testSlicedList();
	//result=testHashList();
	//result=testMemMon();
//
	//abcResult_e shutdownResult = abcShutdown(1);
	//int retVal = (shutdownResult == ABC_PASS ? 0:1);
	int retVal = 1;
	return retVal;

}


