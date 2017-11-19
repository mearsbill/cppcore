/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author Bill Mears   bill@imears.com  2017-1201
 // abcEnums.cpp   
 //
 #include "abcEnums.h"

char *abcResultAsStr(abcResult_e result)
{
	switch (result)
	{
	  case ABC_RETRY: // = -3 retry is basically a fai;ure
		return (char *)"ABC_RETRY";
	  case ABC_LESS_THAN: //  = -2,
		return (char *)"ABC_LESS_THAN";
	  case ABC_FAIL: //  = -1,	// fail codes are negative
		return (char *)"ABC_FAIL";
	  case ABC_PASS: //   = 0,	// zero is simply pass !
	//case ABC_EQUAL = 0,
	//case ABC_SAME = 0,
		return (char *)"ABC_PASS:EQUAL:SAME";
	  case ABC_NOT_EQUAL: //  = 1,
		return (char *)"ABC_NOT_EQUAL";
	  case ABC_GREATER_THAN: //  = 2,
		return (char *)"ABC_GREATER_THAN";
	  case ABC_DIFFERENT: //  = 3,
		return (char *)"ABC_DIFFERENT";
	  case ABC_ERROR: //  = 4,
		return (char *)"ABC_ERROR";
	default:
		return (char *)"abcResult_e case not handled";
	}
	return NULL;
} // abcResultAsStr()

/* Handled now with STRINGIFY() 
char *abcReasonAsStr(abcReason_e reason)
{
	switch (reason)
	{
	  case ABC_REASON_NONE: // =0;
		return (char *)"ABC_REASON_NONE";
	  case  ABC_REASON_NODE_CLONE_FAILED:
		return (char *)"ABC_REASON_NODE_CLONE_FAILED";
	  case  ABC_REASON_NODE_NOT_OWNED:
		return (char *)"ABC_REASON_NODE_NOT_OWNED";
	  case  ABC_REASON_NODE_OWNER_NOT_NULL:
		return (char *)"ABC_REASON_NODE_OWNER_NOT_NULL";
	  case  ABC_REASON_NODE_NOT_ON_LIST:
		return (char *)"ABC_REASON_NODE_NOT_ON_LIST";
	  case  ABC_REASON_NODE_PARAM_IS_NULL:
		return (char *)"ABC_REASON_NODE_PARAM_IS_NULL";
	  case  ABC_REASON_NODE_PARAM_SHOULD_BE_NULL:
		return (char *)"ABC_REASON_NODE_PARAM_SHOULD_BE_NULL";
	  case  ABC_REASON_NODE_BAD_LINK_STRUCTURE:
		return (char *)"ABC_REASON_NODE_BAD_LINK_STRUCTURE";
	  case  ABC_REASON_NODE_KEY_TYPE_MISMATCH:
		return (char *)"ABC_REASON_NODE_KEY_TYPE_MISMATCH";
	  case  ABC_REASON_NODE_KEY_TYPE_INVALID:
		return (char *)"ABC_REASON_NODE_KEY_TYPE_INVALID";
	  case  ABC_REASON_OBJECT_SIZE_WRONG:
		return (char *)"ABC_REASON_OBJECT_SIZE_WRONG";
	  case  ABC_REASON_ATTEMP_TO_RECREATE_SINGLETON:
		return (char *)"ABC_REASON_ATTEMP_TO_RECREATE_SINGLETON";
	  case  ABC_REASON_GLOBAL_INIT_FAILED:
		return (char *)"ABC_REASON_GLOBAL_INIT_FAILED";
	  case  ABC_REASON_UNRECOVERABLE_FAILURE:
		return (char *)"ABC_REASON_UNRECOVERABLE_FAILURE";
	  default:
		return (char *)"abcReasonAsStr FAILED...CASE NOT HANDLED";
	}
	return NULL;

} // abcReasonAsStr()
*/

char *trueFalseAsStr(uint8_t trueFalse)
{
	return (char *)(trueFalse ? "True" : "False");
}

char *passFailAsStr(uint8_t passFail)
{
	return (char *)(passFail ? "Pass" : "Fail");
}


char *yesNoAsStr(uint8_t yesNo)
{
	return (char *)(yesNo ? "Yes" : "No");
}

// EOF abcEnums.cpp
