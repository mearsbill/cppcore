
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
}

#define _PRINT_DEBUG_A_
#include "abcCore.h"

abcResult_e testMemMon()
{
	int i;

	// test the creation  and printing of the name node
	DEBUG_A("TESTING abcMemNameNode_c\n");

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
	DEBUG_A("TESTING abcMemStatsNode_c\n");

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
	DEBUG_A("TESTING abcMmapNode_c\n");

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
	DEBUG_A("Done with memMon node tests\n");
	DEBUG_A("Deleting all objects\n");
	delete  tmmn;
	delete tsn;
	delete tmn;
	delete hList1;
	delete hList2;
	delete hList3;


	DEBUG_A("TESTING abcMemMon_c\n");
	abcMemMon_c *mMon = new abcMemMon_c("testMemMon");
	char  *j1= (char *)mMon + 0x123450;
	void *returnPtr = mMon->interceptClassNew(j1, (char *)"abcJunkClass_c",sizeof(abcMemMon_c),(char *)"unitTest.cpp",89,(char *)"testFn_name");
	if (returnPtr != j1)
	{
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return ABC_FAIL;
	}
	// should fail with same address
	returnPtr = mMon->interceptClassNew(j1, (char *)"abcJunkClass_c",sizeof(abcMemMon_c),(char *)"unitTest.cpp",89,(char *)"testFn_name");
	if (returnPtr)
	{
		DEBUG_A("Should have failed but didn't\n");
		return ABC_FAIL;
	}
	// check for error
	abcReason_e reason = mMon->getErrorReason();
	if (reason != ABC_REASON_NONE && reason != ABC_REASON_MMAP_DUPLICATE_ADDR_INVALID)
	{
		DEBUG_A("interceptClassNew failed reason=%d\n",reason);
		return ABC_FAIL;
	}
	DEBUG_A("Resetting reason and continuing\n"); abcGlobalResetErrorReason(); mMon->setErrorReason(ABC_REASON_NONE);

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
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}

	// initialize with 6(=>7) slices, rebuild threshold 10, 400% growth on rebuild
	DEBUG_A("Adding Single Node to list\n");
	result = tList->addSliceTail(6,t1);		// should fail... disabled method
	if (result != ABC_FAIL)
	{
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->setErrorReason(ABC_REASON_NONE); DEBUG_A("resetting expected error condition at %s:%d\n",__FILE__,__LINE__);
	tList->addTail(t1);		// should fail...method defeated
	if (result != ABC_FAIL)
	{
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->setErrorReason(ABC_REASON_NONE); DEBUG_A("resetting expected error condition at %s:%d\n",__FILE__,__LINE__);
	result = tList->add(t1);		
	if (result != ABC_FAIL)
	{
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->setErrorReason(ABC_REASON_NONE); DEBUG_A("resetting expected error condition at %s:%d\n",__FILE__,__LINE__);
	tList->abcSlicedList_c::print();

	int i;;
	for (i=0;i<100;i++)
	{
		testNode_c *t2= new testNode_c  ();
		testNode_c *fn = NULL;
		char name[32];
		snprintf(name,32,"v%d",i);
		t2->setValue(i);
		t2->setKeyInt(i);
		t2->setName(name);
		result = tList->add(t2);
		if (result != ABC_PASS)
		{
			FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
			return(ABC_FAIL);
		}
	} // finished building list

	tList->abcSlicedList_c::print();


	DEBUG_A("testing find\n");
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
	DEBUG_A("printing Test List\n");
	tList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);
#endif

	DEBUG_A("Cloning hashedList\n");
	abcHashList_c *cList = (abcHashList_c *)tList->clone();
	if (!cList)
	{
		WARNING_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}

	DEBUG_A("Deleting original\n");
	delete tList;

	DEBUG_A("testing find on cloned list\n");
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
	DEBUG_A("Test Pull Head\n");



	abcListNode_c *dn;
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
	DEBUG_A("printing cloned Hash list\n");
	cList->abcSlicedList_c::print();
#endif

	DEBUG_A("Deleting the clone\n");
	delete cList;

	DEBUG_A("Done with HashList test\n");
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
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->setErrorReason(ABC_REASON_NONE); DEBUG_A("resetting error condition\n");
	tList->init(6);
	tList->addSliceTail(6,t1);		// should fail... slice# out of range
	if (result != ABC_FAIL)
	{
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->setErrorReason(ABC_REASON_NONE); DEBUG_A("resetting error condition\n");
	tList->addTail(t1);		// should fail...method defeated
	if (result != ABC_FAIL)
	{
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	tList->setErrorReason(ABC_REASON_NONE); DEBUG_A("resetting error condition\n");
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

	DEBUG_A("printing Test List\n");
	tList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	abcSlicedList_c *t2List = (abcSlicedList_c *)tList->clone();
	if (t2List == NULL)
	{
		FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
		return(ABC_FAIL);
	}
	t2List->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	DEBUG_A("Deleting Test SlicedList\n");
	delete tList;

	// test remove some
	// int sliceMap[30] = {0,1,1,1,3,3,3,3,5,5,5,4,3,2,1,0,0,2,4,5,5,2,3,1,4,2,4,4,0,2};
	// slice counts =>  0:4  1:5  2:6  3:7  4:4 5:5

	abcListNode_c *dn;
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
		FATAL_ERROR_G(ABC_REASON_NODE_PARAM_IS_NULL);
		exit(1);
	}
	DEBUG_A("Deleting some nodes..... slice 3 nodes 2 & 3\n");
	t1 = (testNode_c *)t0->next();	// t1 sb 2 of 5 
	testNode_c *t2 = (testNode_c *)t1->next();	// t2 sb 3 of 5
	DEBUG_A("read... go!\n");
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
	DEBUG_A("looking for 13 and expecting to fail\n");
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey);
	if (t1)
	{
		DEBUG_A("Wasn't upposed to find anything\n");
		exit(1);
	}
	DEBUG_A("Failed as expected\n");
	DEBUG_A("looking for 24 and expecting to find\n");
	nodeKey_setInt(&testKey,24); // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey);
	t1->print();
	if (!t1)
	{
		DEBUG_A("Wasn't supposed to fail\n");
		exit(1);
	}
	DEBUG_A("looking for 26 and expecting to succeed\n");
	nodeKey_setInt(&testKey,26); // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceNext(4,&testKey,t1);
	if (!t1)
	{
		DEBUG_A("Wasn't supposed to fail\n");
		exit(1);
	}
	DEBUG_A("passed as expected\n");
	t1->print();

	DEBUG_A("Now testing searching backwards\n");
	DEBUG_A("looking for 19 and expecting to fail\n");
	nodeKey_setInt(&testKey,19); 
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey,FALSE/*backwards*/);
	if (t1)
	{
		DEBUG_A("Wasn't upposed to find anything\n");
		exit(1);
	}
	DEBUG_A("Failed as expected\n");
	DEBUG_A("looking for 27 and expecting to find\n");
	testKey.value.intgr = 27; // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceFirst(4,&testKey,FALSE);
	if (!t1)
	{
		DEBUG_A("Wasn't supposed to fail\n");
		exit(1);
	}
	t1->print();
	DEBUG_A("looking for 27 again and  expecting to fail\n");
	nodeKey_setInt(&testKey,27); 
	testNode_c *t1a = (testNode_c *)t2List->findSliceNext(4,&testKey,t1,FALSE);
	if (t1a)
	{
		DEBUG_A("Didn't expect to find\n");
		exit(1);
	}
	DEBUG_A("Failed as expected\n");
	DEBUG_A("looking for 18 and expecting to succeed\n");
	nodeKey_setInt(&testKey,18); 
	t1 = (testNode_c *)t2List->findSliceNext(4,&testKey,t1,FALSE);
	if (!t1)
	{
		DEBUG_A("Wasn't supposed to fail\n");
		exit(1);
	}
	DEBUG_A("passed as expected\n");
	t1->print();

	DEBUG_A("looking for 24 and expecting to succeed\n");
	testKey.value.intgr = 24; // fail on first node of slice 2
	t1 = (testNode_c *)t2List->findSliceNext(4,&testKey,t1,FALSE);
	if (!t1)
	{
		DEBUG_A("Wasn't supposed to fail\n");
		exit(1);
	}
	DEBUG_A("passed as expected\n");
	t1->print();




	DEBUG_A("\n\nDone testing slice find\n");









	t2List->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);
	DEBUG_A("Deleting Clone SlicedList\n");
	delete t2List;



	#if 0
	if (tList->getSliceNodeCount(slice+1))
	{
		tList->addSliceTail(slice+1,t2);		// should fail...method defeated
		if (result != ABC_FAIL)
		{
			FATAL_ERROR_G(ABC_REASON_TEST_FAILED);
			return(ABC_FAIL);
		}
		tList->setErrorReason(ABC_REASON_NONE); DEBUG_A("resetting error condition\n");
	}
	#endif

	return ABC_PASS;
}

abcResult_e testPrimes()
{
	//abcGlobalCore->initPrimes(3600);
	abcGlobalCore->printPrimes(1,20);
	abcGlobalCore->updatePrimes(8000);

	int primeTry=200;
	int  myPrime = abcGlobalCore->findPrime(primeTry);
	DEBUG_A("my prime try:%d  number is %d\n",primeTry, myPrime);
	primeTry=104729;
	myPrime = abcGlobalCore->findPrime(primeTry);
	DEBUG_A("my prime try:%d  number is %d\n",primeTry, myPrime);
	abcGlobalCore->printPrimes(104700,104750);
	abcGlobalCore->printPrimes(1,20);
	//abcGlobalCore->initPrimes(10000);
	//abcGlobalCore->initPrimes(1000000);
	return ABC_PASS;
}
abcResult_e testCrc()
{
    abcGlobalCore->generateCrc32Table();
    abcGlobalCore->generateCrc64Table();

	DEBUG_A("Testing CRC32\n");
	uint8_t  *t9=(uint8_t *)"The quick brown fox jumps over the lazy dog";
	uint32_t ref1=0x414FA339;
	int t9l= strlen((char *)t9);
	uint32_t r1 = abcGlobalCore->computeCrc32(0,t9,t9l);
	DEBUG_A("CRC32..... seed 0, string=\"%s\" crc32=%08x ref=%08x diff=%d\n",t9,r1,ref1,r1-ref1);

	t9=(uint8_t *)"123456789";
	t9l= strlen((char *)t9);
	r1 = abcGlobalCore->computeCrc32(0,t9,t9l);
	ref1=0xCBF43926;
	DEBUG_A("CRC32..... seed 0, string=\"%s\" crc32=%08x ref=%08x diff=%d\n",t9,r1,ref1,r1-ref1);


    uint64_t  result;
	typedef unsigned char* PBYTE ;

	unsigned char t1[8]; t1[0]=0x80;
    printf("taking CRC64 of \"\x80\" (should be 0xC96C5795D7870F42)\n");
    result = abcGlobalCore->computeCrc64(t1, 1);
	printf("result0: %016llx\n", result);

    printf("taking CRC64 of \"\\xDE\\xAD\\xBE\\xEF\" (should be FC232C18806871AF)\n");
    result = abcGlobalCore->computeCrc64((PBYTE)"\xDE\xAD\xBE\xEF", 4);
    printf("result: %016llxX\n", result);

    printf("taking CRC64 of \"99eb96dd94c88e975b585d2f28785e36\" (should be DB7AC38F63413C4E)\n");
    result = abcGlobalCore->computeCrc64((PBYTE)"\x99\xEB\x96\xDD\x94\xC8\x8E\x97\x5B\x58\x5D\x2F\x28\x78\x5E\x36", 16);
    printf("result: %016llx\n", result);

    printf("taking CRC64 of \"\\DE\\xAD\" (should be 44277F18417C45A5\n");
    result = abcGlobalCore->computeCrc64((PBYTE)"\xDE\xAD", 2);
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
	DEBUG_A("setup time: %6.2f milliseconds for %d nodes or (%7.6f uSec/node) \n", (double)setupTime1m/1000.0,nodeCount, ((double)setupTime1m/(double)nodeCount));


	int64_t runningTail = nodeCount;


	abcTime1m_t testStartTime1m = getTime1m();
	for (i=0;i<loopCount;i++)
	{
		timeNode = (testNode_c *)testList->pullHead();
		if (!timeNode)
		{
			FATAL_ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
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
	DEBUG_A("Run time: %6.2f milliseconds for %d loops or (%7.6f uSec/sortedAdd) \n", (double)runTime1m/1000.0,loopCount,((double)runTime1m/(double)loopCount));

	delete testList;	// also deletes all listNodes

	return ABC_PASS;
} // end abcResult_e timeSortedListTest(int nodeCount, int loopCount)

abcResult_e listTests()
{
	DEBUG_A("TESTING LISTS\n");

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
			FATAL_ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
			exit(1);
		}
		if (i == 0)
		{
			abcResult_e aResult = tList->addHead(tn);
			if ( aResult != ABC_FAIL )	// had better fail !
			{
				fprintf(stderr,"expected result ABC_FAIL but got %s\n",abcResultAsStr(aResult));
				FATAL_ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
				exit(1);
			}
			DEBUG_A("No problem... previous error  expected... clearing error condition at %s:%d\n",__FILE__,__LINE__);
			tList->setErrorReason(ABC_REASON_NONE);	// reset the error we intentionally caused
		}
	}
	DEBUG_A("generating list print\n");
	//tList->print(1,1);

	DEBUG_A("Cloning the list\n");
	abcList_c *cloneList = tList->clone();

	DEBUG_A("Printing the Clone \n");
	cloneList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	// test the stateless calls
	if (tList->getHead() !=  tList->head())
	{
		DEBUG_A("FAIL getHead != head \n");
		exit(1);
	}

	if (tList->getTail() !=  tList->tail())
	{
		DEBUG_A("FAIL getTail != tail \n");
		exit(1);
	}

	DEBUG_A("deleting the clone and emptying the testList\n");
	delete cloneList;
	tList->empty();

	DEBUG_A("preparing list for find test\n");
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

	DEBUG_A("Building search key\n");
	nodeKey_s sKey;
	sKey.type = KEYTYPE_INT;
	sKey.value.intgr = 5;
	testNode_c *fn = (testNode_c *)tList->findFirst(&sKey,1/*towardsNext*/);
	while (fn)
	{
		DEBUG_A("ADDING %d in the middle\n",i);
		testNode_c *tt = new testNode_c();
		tt->setValue(i);
		tt->setKeyInt(j);
		tt->setName("addMiddle");
		tList->addMiddle(tt,fn);
/*
		abcResult_e  resultAm = tList->addMiddle(tt,fn);
		if (resultAm != ABC_FAIL)
		{
			FATAL_ERROR_G(ABC_REASON_UNRECOVERABLE_FAILURE);
			exit(1);
		}
*/
		i++;
		j = (j+1)%7;
		fn = (testNode_c *)tList->findNext(&sKey,fn);
	}

	tList->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);

	DEBUG_A("Testing find/findNext\n");

	DEBUG_A("scanning fowards\n");
	sKey.value.intgr = 3;
	fn = (testNode_c *)tList->findFirst(&sKey,1/*towardsNext*/);
	while (fn)
	{
		fn->print();
		fn = (testNode_c *)tList->findNext(&sKey,fn);
	}
	DEBUG_A("scanning backwards\n");
	fn = (testNode_c *)tList->findFirst(&sKey,0/*towardsNext*/);
	while (fn)
	{
		fn->print(PRINT_STYLE_LIST_WITH_NODE_DETAILS);
		fn = (testNode_c *)tList->findNext(&sKey,fn,0);
	}

	DEBUG_A("BEFORE EMPTY\n");
	tList->empty();
	DEBUG_A("AFTER EMPTY\n");

	int valueMap[25]={1,3,5,2,4,6,9,8,7,9,24,23,22,9,20,10,11,12,13,15,14,16,17,19,18};
	DEBUG_A("TESTING Sorted add\n");
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

	
	DEBUG_A("DONE TESTING LISTS\n");
	return ABC_PASS;
}


// abcNode tests
abcResult_e nodeTests()
{
	DEBUG_A("TESTING NODES\n");
	DEBUG_A("building a ListNode\n");
	abcListNode_c *le = new abcListNode_c();
	DEBUG_A("Cloning  a ListNode\n");
	abcListNode_c *cle = le->clone();
	DEBUG_A("destroying a ListNode\n");
	delete le;
	//delete cle;

	DEBUG_A("building a testNode\n");
	testNode_c *tn = new testNode_c();
	tn->setName("testNodeName");
	tn->setValue(123);
	DEBUG_A("Cloning  a testNode\n");
	testNode_c *tnc = (testNode_c *)tn->clone();
	DEBUG_A("Cloning  a testNode\n");
	DEBUG_A("Comparing original with clone\n");
	DEBUG_A("destroying a ListNode\n");
	delete tn;
	delete tnc;
	DEBUG_A("FINISHED TESTING NODES\n");
	return ABC_PASS;
}

/*
abcResult_e nodeTests()
{
	DEBUG_A("TESTING NODES\n");
	DEBUG_A("building a ListNode\n");
	abcListNode_c *le = new abcListNode_c();
	DEBUG_A("destroying a ListNode\n");
	delete le;
	DEBUG_A("FINISHED TESTING NODES\n");
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
	abcGlobalCore = new abcCore_c();
	abcResult_e initStatus = abcGlobalCore->init(1);
	if (initStatus != ABC_PASS)
	{
		FATAL_ERROR_G(ABC_REASON_GLOBAL_INIT_FAILED);
		exit(1);
	}
	srand(1);		// will be a fixed random sequence

	// parse command line.  arg0 = programName
	char cmdChar;
	while ((cmdChar = getopt_long(argc, argv, "l:q", testOpts, NULL)) != -1)
	{
		switch (cmdChar)
		{
			case 'q':
				globalArgs.quietMode = TRUE;
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

/*
	char *aStr=(char*)"abced";
	char *bStr=(char*)"abcd";
	printf("%s cmp %s = %d\n",aStr,bStr,strcmp(aStr,bStr));
	exit(1);
*/
	abcResult_e result;

	// build a node then a listNode
	result = nodeTests();
	result = listTests();
	result = timeSortedListTest(10000, 1000);
	//result = testCrc();
	//result = testPrimes();
	result=testSlicedList();
	result=testHashList();
	result=testMemMon();
}


