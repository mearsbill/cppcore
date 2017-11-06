
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
}

#define _PRINT_DEBUG_A_
#include "abcCore.h"

/*
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
*/

void *testFn(void *fPtr,char *objName, char *objArgs, int objSize, char *file, int line, char *function)
{
	DEBUG_A("Constructed a %s(%s) of size %d @ %p  src:%s:%d in %s\n",objName,objArgs,objSize,fPtr,file,line,function);
	return fPtr;
}

void *testFb1(void *fPtr,char *file, int line, char *function)
{
	DEBUG_A("Destroyed %p  src:%s:%d in %s\n",fPtr,file,line,function);
	return fPtr;
}

abcResult_e memTests()
{
#define ABC_NEW(a,b...)  (a *)testFn((void *)(new a(b)),(char *)STRINGIFY(a),(char *)STRINGIFY(b),sizeof(a),(char *)__FILE__,__LINE__,(char *)__FUNCTION__)
	testNode_c *newNode = ABC_NEW(testNode_c,"fred");
	printf("\n");
	newNode->print();
#define ABC_DELETE(a)  delete a; testFb1(a,(char *)__FILE__,__LINE__,(char *)__FUNCTION__)
	ABC_DELETE(newNode);
	return ABC_PASS;
}

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
	abcResult_e initStatus = abcInit(1);
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
	result = memTests();

	return 0;
}


