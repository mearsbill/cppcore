/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com
// File:  abcStrings.h
// 
//

#ifndef __ABC_STRINGS_H__
#define __ABC_STRINGS_H__

#include "abcBase.h"

typedef struct unitsRec_s
{
	double	multiplier;
	char 	*prefixStr;
	char	*zeroFmtStr;
} unitsRec_s;


class abcStrings_c 
{
	private:
		char 		*name;
		abcReason_e	errorReason;
	protected:
		abcStrings_c(const char *setName = NULL);
		~abcStrings_c();

		void setErrorReason(abcReason_e reason);
		abcReason_e getErrorReason();

		char       		 *getObjType();  // the class name
		char       		 *getObjName();  // the instance name when available
		abcResult_e		 print(abcPrintStyle_e printStyle);
		abcResult_e      printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); 

	public:
		// public static convertes.  Thats all we have for now
		static char *snprintf_eng(char*buff,int buffSize,char *format, double value, char *metric, uint8_t computer=0); // computer=TRUE means K=1024 not 1000


}; // end class abcStrings_c

#endif //__ABC_STRINGS_H__

////////////////////////////
// EOF abcStrings.h

