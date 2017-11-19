/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com
// File:  abcThreads.cpp
// 
//

#include "abcStrings.h"	

#define MAX_UNIT_RECS 9
static unitsRec_s unitsTab[MAX_UNIT_RECS] =
{
	{ 1.0e-12, (char *)"p",NULL },
	{ 1.0e-9, (char *)"n",NULL },
	{ 1.0e-6, (char *)"u",NULL },
	{ 1.0e-3, (char *)"m",NULL },
	{ 1.0e-0, (char *)" ",(char *)"%3.0f" },
	{ 1.0e-3, (char *)"K",NULL },
	{ 1.0e-6, (char *)"M",NULL },
	{ 1.0e-9, (char *)"G ",NULL },
	{ 1.0e-12, (char *)"T",NULL }
};


//
// base constructor for abcStrings_c
// call here for all thread subclasses (first)
//
abcStrings_c::abcStrings_c(const char *setName )
{
	if (setName)
	{
		name = strdup(setName);
	}
} // end abcStrings_c::abcStrings_c

//
// base class destructor
//
abcStrings_c::~abcStrings_c()
{
} // end abcStrings_c::~abcStrings_c()

char *abcStrings_c::getObjType()
{
	return (char *)"abcStrings_c";
} // end abcStrings_c::getObjType()

char *abcStrings_c::getObjName()
{
	return name;
} // end abcStrings_c::getObjName()

// Error handling stuff & print stuff
void abcStrings_c::setErrorReason(abcReason_e reason)
{
	errorReason = reason;
}
abcReason_e abcStrings_c::getErrorReason()
{
	return errorReason;
}

abcResult_e abcStrings_c::print(abcPrintStyle_e printStyle)
{
	TRACE_NONIMPL("abcStrings_c");
	return ABC_FAIL;
} // end abcStrings_c::print()

abcResult_e abcStrings_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	TRACE_NONIMPL("abcStrings_c");
	return ABC_FAIL;
} // end abcStrings_c::printBuff()


//// our most important units printer
char *abcStrings_c::snprintf_eng(char*buff,int buffSize,char *format, double value, char *metric, uint8_t computer)
{
	uint8_t go = 0; 				// >0 means increase units, <0 means decrease.  0 means stop
	int idx = 4;					// start at multiplier of zero
	double mantissa = value;
	double neg = 1.0;
	if (value < 0.0)
	{
		mantissa = -value;
		neg = -1.0;
	}

	double unitsShift = computer ? 1024.0 : 1000.0;
	int overflow = FALSE;

	do
	{
		if (mantissa > 999.9999999)
		{
			if (idx >= (MAX_UNIT_RECS-1) )
			{
				// overflow... print in true scientific notation
				idx = (MAX_UNIT_RECS)-1;
				overflow=TRUE;
				break;
			}
			go = +1;
			idx++;
			mantissa /= unitsShift;
		}
		else if (mantissa < 1.000)
		{
			if (idx < 0)
			{
				// value too small... use index 4 (no units)
				idx = 4;
				mantissa = 0.0;
				break;
			}
			go = -1;
			idx--;
			mantissa *= unitsShift;
		}
		else
		{
			go = 0;
		}
	} while (go != 0);

	// now print the mantissa 
	double printVal = mantissa * neg;
	char nBuff[64];
	char *prefix = unitsTab[idx].prefixStr;
	char *altFmt =  unitsTab[idx].zeroFmtStr;
	if (!overflow)
	{
		char *fmt = (idx == 4) ?  altFmt : format;
		snprintf(nBuff,64,fmt,printVal);
		snprintf(buff,buffSize,"%s %s%s",nBuff,prefix,metric);
	}
	else
	{
		snprintf(buff,buffSize,"%10E %s",value,metric);
	}
	return buff;
} // end abcStrings_c::snprintf_eng(char*buff,int buffSize,char *format, double value, char *metric, uint8_t computer)




