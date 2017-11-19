/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com	 2017-1121
// File:  abcNodes.cpp
//

// There are global defines, but placed here to 
// allow quick checking of stuff without a massive recompile
#define _TRACE_NONIMPL_


#include "abcNodes.h"
#include "abcDebugMacros.h"
#include "abcCore.h"

extern "C"
{
#include <string.h>
}


/////////////////////////////
// static helper functions
/////////////////////////////
char *keyTypeAsStr(keyType_e type)
{
	switch(type)
	{
	    case KEYTYPE_UNKNOWN:
			return (char *)"keyUnknwn";
	    case KEYTYPE_INT:
			return (char *)"keyIntgr";
	    case KEYTYPE_DOUBLE:
			return (char *)"keyDbl";
	    case KEYTYPE_PTR:
			return (char *)"keyPtr   ";
	    case KEYTYPE_STRING_COPYIN:
			return (char *)"keyString";
	    case KEYTYPE_STRING_MEMBER:
			return (char *)"keyStringMmbr";
	    case KEYTYPE_STRING_EXTERNAL:
			return (char *)"keyStringExt";
	    case KEYTYPE_BIN_COPYIN:
			return (char *)"keyBin";
	    case KEYTYPE_BIN_MEMBER:
			return (char *)"keyBinMember";
	    case KEYTYPE_BIN_EXTERNAL:
			return (char *)"keyBinExt";
		default:
			return (char *)"Invalid  ";
	}
}

///////////////////////////////////////////////////////
// methods for  nodeKey_s 
//
//  This is the key used by the abcNode_c object for searching
//
// abcListNode_c could use these methods  but doesnt
// and create helper methods for standalone nodeKeys
void nodeKey_init(nodeKey_s *This)
{
	This->type = KEYTYPE_UNKNOWN;
	This->size = 0;
	This->value.string = NULL;
}
void nodeKey_destroy(nodeKey_s *This)
{
	switch(This->type)
	{
	    case KEYTYPE_STRING_COPYIN:
	    case KEYTYPE_BIN_COPYIN:
			if (This->value.string)
			{
				free(This->value.string); 
				nodeKey_init(This);
			}
			break;

	    case KEYTYPE_UNKNOWN:
	    case KEYTYPE_INT:
	    case KEYTYPE_DOUBLE:
	    case KEYTYPE_PTR:
	    case KEYTYPE_STRING_MEMBER:
	    case KEYTYPE_STRING_EXTERNAL:
	    case KEYTYPE_BIN_MEMBER:
	    case KEYTYPE_BIN_EXTERNAL:
		default:
			break;
	}
	nodeKey_init(This);
}
void nodeKey_setInt(nodeKey_s *This, const int64_t intKey)
{
	This->type = KEYTYPE_INT;
	This->size = 0;
	This->value.intgr = intKey;
}
void nodeKey_setDbl(nodeKey_s *This, const double dblKey)
{
	This->type = KEYTYPE_DOUBLE;
	This->size = 0;
	This->value.dbl = dblKey;
}
void nodeKey_setPtr(nodeKey_s *This, const void *ptrKey)
{
	This->type = KEYTYPE_PTR;
	This->size = 0;
	This->value.ptr = (void *)ptrKey;
}
void nodeKey_setString(nodeKey_s *This, const char *string)
{
	This->type = KEYTYPE_STRING_COPYIN;
	This->size = 0;
	// protect against re-key 
	if (This->value.string)
	{
		free(This->value.string);
		This->value.string = NULL;
	}
	// only copying when not null
	if ( string )
	{
		This->value.string = strdup(string);
	}
}
void nodeKey_setStringExternal(nodeKey_s *This, const char *string)
{
	This->type = KEYTYPE_STRING_EXTERNAL;
	This->size = 0;
	This->value.string = (char *)string;
}
void nodeKey_setBinary(nodeKey_s *This, const uint8_t *binary, int sLen)
{
	This->type = KEYTYPE_BIN_COPYIN;
	// protect against re-key 
	if (This->value.string)
	{
		free(This->value.string);
		This->value.string = NULL;
		This->size = 0;
	}
	if ( binary )
	{
		This->value.string = (char *)malloc(sLen);
		This->size = (int32_t)sLen;
		memcpy(This->value.string,binary,sLen);
	}
}
void nodeKey_setBinaryExternal(nodeKey_s *This, const uint8_t *binary, int sLen)
{
	This->type = KEYTYPE_BIN_EXTERNAL;
	This->size = (int32_t)sLen;
	This->value.string = (char *)binary;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//
// abcNode_c is the most basic node of the list construct
// abcNode_c Methods
//
abcNode_c::abcNode_c() 
{
	prevNode = NULL;
	nextNode = NULL;
}
abcNode_c::~abcNode_c() 
{
}

// public methods
abcNode_c *abcNode_c::next()
{
	return nextNode; 
}
abcNode_c *abcNode_c::prev()
{
	return prevNode;
}

// virtual printing and debug print helping
char *abcNode_c::getObjType()
{
	// NOTE: DON"T TRY TO FREE THESE STRINGS
	return (char *)"abcNode_c";
}
char *abcNode_c::getObjName()
{
	return (char *)NULL;		// no name for this object type
}

// printing and print helping
abcResult_e     abcNode_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char *name = getObjName();
	if (!name)
	{
		name = getObjType();
	}
	switch(printStyle)
	{
		case PRINT_STYLE_LIST_WITH_NODE_DETAILS:	// for list node detail
			snprintf(pBuff,pbuffSize,"%s: %p  p:%p n:%p",name,this,prevNode,nextNode);
			break;
		case PRINT_STYLE_LIST_HEADER_SUMMARY:	// print for list header
			snprintf(pBuff,pbuffSize,"%s: %p ",name,this);
			break;
		case PRINT_STYLE_LIST_MEM_NAME_LIST:
		case PRINT_STYLE_LIST_MEM_STATS_LIST:
		case PRINT_STYLE_LIST_MMAP_LIST:
			snprintf(pBuff,pbuffSize,"%s ",name);
			break;
		default: // print name:
			snprintf(pBuff,pbuffSize,"n%d:  %s ",printStyle,name);
	}
	return ABC_PASS;
}

abcResult_e abcNode_c::print(abcPrintStyle_e printStyle)
{
	char pBuff[128];

	abcResult_e result = abcNode_c::printBuff(pBuff,128,printStyle);
	if (result)
	{
		ERROR_G(ABC_REASON_PRINTING_FAILED);
		return result;
	}
	fprintf(stderr,"%s\n",pBuff);
	return ABC_PASS;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//
// abcListNode_c Methods
//
abcListNode_c::abcListNode_c()
{
	nodeKey_init(&key);	// init to KEYTYPE_UNKNOWN
	owner = NULL;
	sliceIndex = 0;
	resetReason();
}
abcListNode_c::~abcListNode_c() 
{ 
	nodeKey_destroy(&key);
}
void abcListNode_c::resetReason()
{
	reason = ABC_REASON_NONE;
}
void abcListNode_c::setReason(abcReason_e setReason)
{
	reason = setReason;
}
abcReason_e abcListNode_c::getReason()
{
	return reason;
}
// key cloning is tricky because of the string.... which might be strduped
// when we clone it, we might need to strdup another copy so it can be freed independently
// if the value type is a pointer, the pointer itself is the unit of compare and does not get copied.
//
abcResult_e abcListNode_c::copyOut(abcListNode_c *other)
{
	// we'll verify that the size of the object has not changed.
	// when it does, you should carefully inspect for missing elements in the cloning process
	OBJ_SIZE_CHECK(abcListNode_c,56);	// will return error Var;


	//
	// element by element copy.  To make sure we get'em all
	// take special care with the key
	//

	other->key.type = key.type;
	switch(key.type)
	{
	    case KEYTYPE_STRING_COPYIN:	// null terminated
			other->key.value.string = strdup(key.value.string);
			break;

	    case KEYTYPE_BIN_COPYIN:	// memory block with size
			other->key.value.string = (char *)malloc(key.size);
			memcpy(other->key.value.string,key.value.string,key.size);
			break;

	    case KEYTYPE_BIN_MEMBER:
	    case KEYTYPE_STRING_MEMBER:
			{
				CHECK_ERROR(!key.value.string,ABC_REASON_NODE_PARAM_IS_NULL,ABC_FAIL);
				// we're assuming  child classes have copied their personal data so we can clone it
				// calculate Parent Offset.
				int64_t offset = (key.value.string - (char *)this);
				char *newPtr = ((char *)other + offset);
				key.value.string = (char *)newPtr;
			}
			break;


	    case KEYTYPE_UNKNOWN:
	    case KEYTYPE_INT:
	    case KEYTYPE_DOUBLE:
	    case KEYTYPE_PTR:
	    case KEYTYPE_STRING_EXTERNAL:
	    case KEYTYPE_BIN_EXTERNAL:
		default:
			other->key.value = key.value;	// copies the pointer for REF type strings.
	}
	other->owner = NULL;
	other->sliceIndex = sliceIndex;
	other->reason = reason;
	
	// should be returning with the payload copied but the owner and the prev/next variables NULL
	return ABC_PASS;
} // end abcListNode_c::copyOut()
//
abcListNode_c *abcListNode_c::clone()
{
	abcListNode_c *copy = new abcListNode_c();	// make right storage
	abcListNode_c::copyOut(copy);				// then copy guts
	return copy;
}

// this method returns the key used for hashing... which in the case of a string
// is a Crc64 of the string clipped to be positive.  Modulus operation is not performed bere.
uint64_t abcListNode_c::calcKeyHash(abcResult_e *resultOut)
{
	return abcListNode_c::calcKeyHash(&key, resultOut);
}
uint64_t abcListNode_c::calcKeyHash(struct nodeKey_s *keyOnly,abcResult_e *resultOut)
{
	switch (keyOnly->type)
	{
		case KEYTYPE_INT:
		case KEYTYPE_PTR:
			if (resultOut) *resultOut = ABC_PASS;
			return (uint64_t)(keyOnly->value.intgr);
		case KEYTYPE_DOUBLE:
			WARNING(ABC_REASON_NODE_KEY_INVALID_HASH);
			if (resultOut) *resultOut = ABC_PASS;
			return (uint64_t)(keyOnly->value.dbl);
		case KEYTYPE_STRING_COPYIN:
		case KEYTYPE_STRING_MEMBER:
		case KEYTYPE_STRING_EXTERNAL:
			{
				int strSize = keyOnly->size;
				if (resultOut) *resultOut = ABC_PASS;
				return (globalCore->computeCrc64((uint8_t *)keyOnly->value.string,strSize));
			}
		case KEYTYPE_BIN_COPYIN:
		case KEYTYPE_BIN_MEMBER:
		case KEYTYPE_BIN_EXTERNAL:
			{
				int strSize = strlen(keyOnly->value.string);
				if (resultOut) *resultOut = ABC_PASS;
				return (globalCore->computeCrc64((uint8_t *)keyOnly->value.string,strSize));
			}
		case KEYTYPE_UNKNOWN:
		default:
			ERROR(ABC_REASON_NODE_KEY_TYPE_INVALID);
			if (resultOut) *resultOut = ABC_FAIL;
			return 0;
	}
}

// compare external key to nodeKey
abcResult_e abcListNode_c::diffKey(struct nodeKey_s *keyOnly)
{
	// types string and binary have multiple match possiblities
	switch (key.type)
	{
		case KEYTYPE_INT:
			if (keyOnly->type != KEYTYPE_INT) break;	// error
			if (keyOnly->value.intgr == key.value.intgr) return ABC_EQUAL;
			if (keyOnly->value.intgr > key.value.intgr) return ABC_GREATER_THAN;
			return ABC_LESS_THAN;

		case KEYTYPE_PTR:
			if (keyOnly->type != KEYTYPE_PTR) break;	// error
			if (keyOnly->value.intgr == key.value.intgr) return ABC_EQUAL;
			if (keyOnly->value.intgr > key.value.intgr) return ABC_GREATER_THAN;
			return ABC_LESS_THAN;

		case KEYTYPE_DOUBLE:
			if (keyOnly->type != KEYTYPE_DOUBLE) break;	// error
			if (keyOnly->value.dbl == key.value.dbl) return ABC_EQUAL;
			if (keyOnly->value.dbl > key.value.dbl) return ABC_GREATER_THAN;
			return ABC_LESS_THAN;

		case KEYTYPE_STRING_COPYIN:
		case KEYTYPE_STRING_MEMBER:
		case KEYTYPE_STRING_EXTERNAL:	// note: strcmp is a signed compare
			if ((keyOnly->type != KEYTYPE_STRING_COPYIN) && (keyOnly->type != KEYTYPE_STRING_MEMBER) && (keyOnly->type !=KEYTYPE_STRING_EXTERNAL)) break;	// error
			return (abcResult_e)(strcmp(keyOnly->value.string,key.value.string)<<1);	// strcmp produces -1,0,1 => -2,0,2  which is ABC_LESS_THAN, ABC_EQUAL, ABC_GREATER THAN

		case KEYTYPE_BIN_COPYIN:
		case KEYTYPE_BIN_MEMBER:
		case KEYTYPE_BIN_EXTERNAL:	// note: memcmp is an unsigned compare
			if ((keyOnly->type != KEYTYPE_BIN_COPYIN) && (keyOnly->type != KEYTYPE_BIN_MEMBER) && (keyOnly->type !=KEYTYPE_BIN_EXTERNAL)) break;	// error
			if (keyOnly->size != key.size) return ABC_DIFFERENT;
			return (abcResult_e)memcmp(keyOnly->value.string,key.value.string,keyOnly->size);	// memcmp produces -128=>127

		case KEYTYPE_UNKNOWN:
		default:
			ERROR(ABC_REASON_NODE_KEY_TYPE_INVALID);
			return ABC_ERROR;
	}
	ERROR(ABC_REASON_NODE_KEY_TYPE_MISMATCH);
	return ABC_ERROR;
}
	

// compare an other node's key to this node's key
abcResult_e abcListNode_c::diffKey(class abcListNode_c *other)
{
	CHECK_ERROR(!other,ABC_REASON_NODE_PARAM_IS_NULL,ABC_ERROR);
	return diffKey(&(other->key));
}

// compare entire node
abcResult_e abcListNode_c::diffNode(class abcListNode_c *other)
{
	TRACE_NONIMPL("abcListNode_c");
	return ABC_EQUAL;
}

// string print the key
abcResult_e abcListNode_c::keyPrintBuff(char *pBuff,int pbuffSize)
{
	// print the key into a string
	switch (key.type)
	{
		case KEYTYPE_INT:
			snprintf(pBuff,pbuffSize,"%-10s %-10lld",keyTypeAsStr(key.type),key.value.intgr);
			break;
		case KEYTYPE_DOUBLE:
			snprintf(pBuff,pbuffSize,"%-10s %-10.2f",keyTypeAsStr(key.type),key.value.dbl);
			break;
		case KEYTYPE_PTR:
			snprintf(pBuff,pbuffSize,"%-10s %-10llx",keyTypeAsStr(key.type),(uint64_t)key.value.ptr);
			break;
		case KEYTYPE_STRING_COPYIN:
		case KEYTYPE_STRING_MEMBER:
		case KEYTYPE_STRING_EXTERNAL:
			snprintf(pBuff,pbuffSize,"%-10s %-10s",keyTypeAsStr(key.type),key.value.string);
			break;
		case KEYTYPE_BIN_COPYIN:
		case KEYTYPE_BIN_MEMBER:
		case KEYTYPE_BIN_EXTERNAL:
			snprintf(pBuff,pbuffSize,"%-10s %-10s",keyTypeAsStr(key.type),key.value.string);
			break;
		case KEYTYPE_UNKNOWN:
			snprintf(pBuff,pbuffSize,"%-10s %-10s ",keyTypeAsStr(key.type),"----------");
			break;
		default:
			snprintf(pBuff,pbuffSize,"%-10s %-10s ",keyTypeAsStr(key.type),"----------");
			return ABC_FAIL;
	}
	return ABC_PASS;
}

// virtual
char *abcListNode_c::getObjType()
{
	return (char *)"abcListNode_c";
}
char *abcListNode_c::getObjName()
{
	return (char *)NULL;		// no name for this object type
}

// printing and print helping
// style 0: ptr name  prev next key
// style 1: name 
abcResult_e     abcListNode_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char abcNodeBuff[128];
	char keyBuff[128];
	abcResult_e result;

	result = abcNode_c::printBuff(abcNodeBuff,128,printStyle);	
	result = abcListNode_c::keyPrintBuff(keyBuff,128);

	switch(printStyle)
	{
		case PRINT_STYLE_LIST_WITH_NODE_DETAILS:	// node detail
			snprintf(pBuff,pbuffSize,"%s o:%p s:%d => %s",abcNodeBuff,owner,sliceIndex,keyBuff);	// style zero
			break; 

		case PRINT_STYLE_LIST_HEADER_SUMMARY: // list header
			{
				snprintf(pBuff,pbuffSize,"%s o:%p",abcNodeBuff,owner);
			}
			break; 

		case PRINT_STYLE_LIST_MEM_NAME_LIST:
		case PRINT_STYLE_LIST_MEM_STATS_LIST:
			snprintf(pBuff,pbuffSize,"%s s:%d o:%p",abcNodeBuff,sliceIndex,owner);
			break;

		case PRINT_STYLE_LIST_MMAP_LIST:
			snprintf(pBuff,pbuffSize,"%s k:%p: o:%p",abcNodeBuff,key.value.ptr,owner);
			break;


		default:
			snprintf(pBuff,pbuffSize,"lnps%d:%s ",printStyle,abcNodeBuff);	// style ?
			break;

	}
	return ABC_PASS;
}

abcResult_e abcListNode_c::print(abcPrintStyle_e printStyle)
{
	char pBuff[256];
	abcResult_e result = abcListNode_c::printBuff(pBuff,256,printStyle);
	CHECK_ERROR(result,ABC_REASON_PRINTING_FAILED,result);
	fprintf(stderr,"%s\n",pBuff);
	return ABC_PASS;
}


/*  methods defined in the .h file
	void setKeyPtr(void *ptr) { key.type = KEYTYPE_PTR; key.ptr = ptr; }
*/
void abcListNode_c::setKeyString(const char *stringKey)
{
	key.type = KEYTYPE_STRING_COPYIN; 
	key.size = 0;
	key.value.string = strdup(stringKey);
}

void abcListNode_c::setKeyStringMember(const char *stringKey)
{
	key.type = KEYTYPE_STRING_MEMBER; 
	key.size = 0;
	key.value.string = (char *)stringKey;
}

void abcListNode_c::setKeyStringExternal(const char *stringKey)
{
	key.type = KEYTYPE_STRING_EXTERNAL; 
	key.size = 0;
	key.value.string = (char *)stringKey;
}



void abcListNode_c::setKeyBinary(const uint8_t *mem,int size)
{
	key.type = KEYTYPE_BIN_COPYIN; 
	key.value.string = (char *)malloc(size);
	key.size  = size;
	memcpy(key.value.string,mem,size);
}

void abcListNode_c::setKeyBinaryMember(const uint8_t *mem,int size)
{
	key.type = KEYTYPE_BIN_MEMBER; 
	key.value.string = (char *)mem;
	key.size  = size;
}

void abcListNode_c::setKeyBinaryExternal(const uint8_t *mem,int size)
{
	key.type = KEYTYPE_BIN_EXTERNAL; 
	key.value.string = (char *)mem;
	key.size  = size;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

testNode_c::testNode_c(const char *setName)
{
	if (setName != NULL)
	{
		name = strdup(setName);
	}
	else
	{
		name=NULL;	 // this is a strdup'd string
	}
	value=0;
}
testNode_c::~testNode_c()
{
	if (name)
	{
		free(name);
		name = NULL;
	}
}
//
// Note:  As we move up the heirarchy from child to parent WE DO THINGS A LITTLE DIFFERENTLY THAN YOU WOULD EXPECT
// Specifically, we clone/copy/initialize the child data before the parents so that (in this case) we can 
// know how to clone the KEYTYPE_STRING_MEMBER and KEYTYPE_BIN_MEMBER keytypes.  Be warned, this is probably
// not a good general practice, but seems safe enough here, so I'd doing it this way for at least this family of acbNode_c decendants.
//
abcResult_e testNode_c::copyOut(testNode_c *targetOfCopy)
{

	// safety check for object size change (something was added... which would imply clone method needs update
	OBJ_SIZE_CHECK(testNode_c,72);

	// copy parent and then our sepcifics

	abcResult_e coResult = abcListNode_c::copyOut(targetOfCopy);
	CHECK_ERROR(coResult,ABC_REASON_NODE_CLONE_FAILED,coResult);
	if (name)
	{
		int sl = strlen(name);
		char newName[sl+5];
		snprintf(newName,sl+5,"%s[cl]",name);
		targetOfCopy->name = strdup(newName);
	}
	targetOfCopy->value = value;


	// should be returning with the payload copied but the owner and the prev/next variables NULL
	return ABC_PASS;
}

// virtual
char *testNode_c::getObjType()
{
	return (char *)"testNode_c";
}
char *testNode_c::getObjName()
{
	return (char *)name;		// no name for this object type
}


// printing and print helping
abcResult_e     testNode_c::printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle)
{
	char abcListNodeBuff[256];
	abcResult_e result;

	result = abcListNode_c::printBuff(abcListNodeBuff,256,printStyle);	// lower
	switch(printStyle)
	{
		case 0:
		default:
			snprintf(pBuff,pbuffSize,"%s %s %lld", abcListNodeBuff,name, value);	// style zero
	}
	return ABC_PASS;
}

abcResult_e testNode_c::print(abcPrintStyle_e printStyle)
{
	char pBuff[256];
	abcResult_e result = testNode_c::printBuff(pBuff,256,printStyle);
	CHECK_ERROR(result,ABC_REASON_PRINTING_FAILED,result);
	fprintf(stderr,"%s\n",pBuff);
	return ABC_PASS;
}

abcListNode_c *testNode_c::clone()
{
	testNode_c *copy = new testNode_c();
	testNode_c::copyOut(copy);
	return copy;
}


abcResult_e testNode_c::diffNode(class abcListNode_c *other)
{
	TRACE_NONIMPL("testNode_c");
	return ABC_DIFFERENT;
}

void testNode_c::setName(const char *setName)
{
	if (name)
	{
		free(name);
		name = NULL;
	}
	if (setName)
	{
		name = strdup(setName);
	}
}
void testNode_c::setValue(const int setValue)
{
	value = setValue;
}


//////////////////////////////////////////////////////////////////////////////////
// EOF abcNodes.cpp

