/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author Bill Mears   bill@imears.com  2017-1201
 // abcEnums.h   
 //
 #ifndef __ABC_ENUMS_H__
 #define __ABC_ENUMS_H__
 #include "abcBase.h"

//////////// This is where all the enums are defined.  There is a string maker for each type

//
// the most basic enum is abcResult_e which is the ubiquetous return value for function calls.
// 
typedef enum abcResult_e
{
	ABC_RETRY = -3, // retry us basically a fai;ure
	ABC_LESS_THAN = -2,
	ABC_FAIL = -1,	// fail codes are negative
	ABC_PASS = 0,	// zero is simply pass !
	ABC_EQUAL = 0,
	ABC_SAME = 0,
	ABC_NOT_EQUAL = 1,
	ABC_GREATER_THAN = 2,
	ABC_DIFFERENT = 3,
	ABC_ERROR = 4,
} abcResult_e;
char *abcResultAsStr(abcResult_e result);

typedef enum abcReason_e
{
	ABC_REASON_NONE=0,
	ABC_REASON_NODE_CLONE_FAILED,
	ABC_REASON_NODE_NOT_OWNED,
	ABC_REASON_NODE_OWNER_NOT_NULL,
	ABC_REASON_NODE_NOT_ON_LIST,
	ABC_REASON_NODE_PARAM_IS_NULL,
	ABC_REASON_NODE_PARAM_SHOULD_BE_NULL,
	ABC_REASON_NODE_BAD_LINK_STRUCTURE,
	ABC_REASON_NODE_KEY_TYPE_MISMATCH,
	ABC_REASON_NODE_KEY_TYPE_INVALID,
	ABC_REASON_NODE_KEY_TYPE_NOT_INITIALIZED,
	ABC_REASON_NODE_KEY_INVALID_HASH,
	ABC_REASON_OBJECT_SIZE_WRONG,
	ABC_REASON_ATTEMP_TO_RECREATE_SINGLETON,
	ABC_REASON_GLOBAL_INIT_FAILED,
	ABC_REASON_UNRECOVERABLE_FAILURE,
	ABC_REASON_LIST_CLONE_FAILED,
	ABC_REASON_LIST_NOT_SORTED,
	ABC_REASON_LIST_IS_SORTED,
	ABC_REASON_LIST_BAD_SLICE_COUNT,
	ABC_REASON_LIST_ALREADY_INITIALIZED,
	ABC_REASON_LIST_BAD_NODE_COUNT,
	ABC_REASON_LIST_INVALID_SLICE_NUMBER,
	ABC_REASON_LIST_BAD_SLICE_ACCOUNTING,
	ABC_REASON_LIST_NODE_SLICE_MISMATCH,
	ABC_REASON_LIST_DISABLED_OPERATION,
	ABC_REASON_LIST_HASH_SLICE_OVERFULL,
	ABC_REASON_LIST_HASH_ADD_FAILED,
	ABC_REASON_LIST_HASH_RESIZE_FAILED,
	ABC_REASON_MMAP_DUPLICATE_ADDR_INVALID,
	ABC_REASON_MMAP_ADD_FAILED,
	ABC_REASON_MMAP_ALREADY_DELETED,
	ABC_REASON_MMAP_DEL_FAILED,
	ABC_REASON_MMAP_STATS_NODE_MISSING,
	ABC_REASON_PARAM_TOO_SMALL,
	ABC_REASON_TEST_FAILED,
	ABC_REASON_LAST
} abcReason_e;
char *abcReasonAsStr(abcReason_e reason);

typedef enum abcPrintStyle_e
{
	PRINT_STYLE_UNSET = 0,
	PRINT_STYLE_LIST_WITH_NODE_DETAILS,
	PRINT_STYLE_LIST_HEADER_SUMMARY,
	PRINT_STYLE_LIST_MEM_NAME_LIST,
	PRINT_STYLE_LIST_MEM_STATS_LIST,
	PRINT_STYLE_LIST_MMAP_LIST,
	PRINT_STYLE_MAX,
} abcPrintStyle_e;

// not really enums, but we print them
char *trueFalseAsStr(uint8_t trueFalse);
char *yesNoAsStr(uint8_t yesNo);


#endif //__ABC_TYPES_H__
// EOF abcTypes.h