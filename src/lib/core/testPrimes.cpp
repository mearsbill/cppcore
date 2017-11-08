
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
}

#define _PRINT_DEBUG_A_
#include "abcCore.h"

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

//abcCore_c *abcGlobalCore = NULL; /// this is the variable for abcCore_c object

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

	//abcGlobalCore->initPrimes(3600);
	DEBUG_A(" -----\n");
	abcGlobalCore->updatePrimes(8000);
	int primeTry=200;
	int	 myPrime = abcGlobalCore->findPrime(primeTry);
	DEBUG_A("my prime try:%d  number is %d\n",primeTry, myPrime);
	primeTry=104729;
	myPrime = abcGlobalCore->findPrime(primeTry);
	DEBUG_A("my prime try:%d  number is %d\n",primeTry, myPrime);
	abcGlobalCore->printPrimes(104000,105000);
	//abcGlobalCore->initPrimes(10000);
	//abcGlobalCore->initPrimes(1000000);
	
	


	return 0;
}


