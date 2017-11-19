/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com
// File:  abcNodes.h
// 
// abcNode_c is the most basic node of the list construct
//

#ifndef __ABC_NODES_H__
#define __ABC_NODES_H__

#include "abcBase.h"

//////////////////////////////////////////////////////////////////////////////////
// This is the most basic list construct.  No state and no owner.
// Intended for use as a base class
//
class abcNode_c
{
  protected:
	class  abcNode_c	*nextNode;
	class  abcNode_c 	*prevNode;

  public:
	friend class abcList_c;

	// public lifecycle
	abcNode_c();
	virtual ~abcNode_c();

	virtual char	*getObjType();	// the class name
	virtual char	*getObjName();	// the instance name when available
	virtual abcResult_e print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_WITH_NODE_DETAILS); 
	abcResult_e		printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); //nonVirtual prinfbuff for this level of the object hiearchy
	
	// public methods
	class abcNode_c *next();
	class abcNode_c *prev();

};

//////////////////////////////////////////////////////////////////////////////////
//
// Note on different keyTypes.
//   STRING_COPYIN is the safest type of string.  When cloned, the string is dupped so the lifecycles are separate
//   STRING_MEMBER means that there is storage withing the actual node subclass that gets pointed at.  When cloned this pointer is updated to point at the new objects guts and is not deleted
//   STRING_EXTERNAL means the listNode is not responsible for the lifecycle of the string... when the listnode is clones, this pointer is copied.  If the the original deletes this string, then the clone will be in trouble.
///	 BIN.... all modes are the same as with string
//
//   Net net.... if you have concens, use STRING_COPYIN.... its clean and the safest unless your data is massive, safe is best !!!
//
typedef enum keyType_e
{
	KEYTYPE_UNKNOWN = 0,
	KEYTYPE_INT,
	KEYTYPE_DOUBLE,				// not a great key for equals compares,
	KEYTYPE_PTR,
	KEYTYPE_STRING_COPYIN,		// null terminated string Copied in
	KEYTYPE_STRING_MEMBER,		// null terminated string is a member of the object
	KEYTYPE_STRING_EXTERNAL,	// null terminated string via extrenal reference or refence within the listNode object [ but not copied in ]
	KEYTYPE_BIN_COPYIN,			// arbitraty memory block with size copied in
	KEYTYPE_BIN_MEMBER,			// member variable  memory block with fixed size
	KEYTYPE_BIN_EXTERNAL,		// arbitraty external memory block with fixed size
} keyType_e;
char *keyTypeAsStr(keyType_e type);


typedef struct nodeKey_s
{
	keyType_e	type;
	int32_t		size;		// size for Binary keyTypes (by convention use the string union member as the memory block pointer)
	union nodeKeyVal_u
	{
		int64_t		intgr;
		double		dbl;
		void		*ptr;
		char		*string;
	} value;
} nodeKey_s;

// methods to init the nodeKey_s structure
// there can't be a _setXXXMember for thise style init.L
void nodeKey_init(nodeKey_s *This);
void nodeKey_destroy(nodeKey_s *This);
void nodeKey_setInt(nodeKey_s *This, const int64_t intKey);
void nodeKey_setDbl(nodeKey_s *This, const double dbltKey);
void nodeKey_setPtr(nodeKey_s *This, const void *ptrKey);
void nodeKey_setString(nodeKey_s *This, const char *string);
void nodeKey_setStringExternal(nodeKey_s *This, const char *string);
void nodeKey_setBinary(nodeKey_s *This, const uint8_t *string, int sLen);
void nodeKey_setBinaryExternal(nodeKey_s *This, const uint8_t *string, int sLen);


class  abcListNode_c : public abcNode_c
{
  protected: 
	friend class abcList_c;
	friend class abcSlicedList_c;
	friend class abcHashList_c;

	nodeKey_s			key;		// the search key
	class abcList_c		*owner;
	int32_t				sliceIndex;		// used for slice, hash lists
	abcReason_e			reason;

  public:
	abcListNode_c();	// method in cpp file
	virtual ~abcListNode_c();	// method in cpp file

	void				resetReason();
	void				setReason(abcReason_e setReason);
	abcReason_e			getReason();

	virtual char		*getObjType();	// the class name
	virtual char		*getObjName();	// the instance name when available
	char *getKeyString() {return key.value.string; }
	virtual abcResult_e print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_WITH_NODE_DETAILS); 
	abcResult_e			printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); //nonVirtual prinfbuff for this level of the object hiearchy
	abcResult_e			keyPrintBuff(char *pBuff,int pbuffSize);
	uint64_t 			calcKeyHash(abcResult_e *resultOut=NULL);								// for this node
	uint64_t 			calcKeyHash(struct nodeKey_s *keyOnly,abcResult_e *resultOut=NULL);		// actually a static method but not coded so

	// object general behavior.   Note cloning will use the abcMemMon macros.
	virtual abcListNode_c *clone();
	abcResult_e copyOut(abcListNode_c *targetOfCopy);

	virtual abcResult_e diffKey(class abcListNode_c *other);	// compare just the key	( other on left.)
	virtual abcResult_e diffKey(struct nodeKey_s *key);			// compare to a  key
	virtual abcResult_e diffNode(class abcListNode_c *other);	// compare entire node


	// specific behavior
	void setKeyInt(const int64_t intKey) { key.type = KEYTYPE_INT; key.value.intgr = intKey; };
	void setKeyDbl(const double dblKey)  { key.type = KEYTYPE_DOUBLE; key.value.dbl = dblKey; };
	void setKeyPtr(void *ptr) { key.type = KEYTYPE_PTR; key.value.ptr = ptr; };
	void setKeyString(const char *ptr);
	void setKeyStringMember(const char *ptr);
	void setKeyStringExternal(const char *ptr);
	void setKeyBinary(const uint8_t *ptr,int size);
	void setKeyBinaryMember(const uint8_t *stringKey, int size);
	void setKeyBinaryExternal(const uint8_t *stringKey, int size);
};

//////////////////////////////////////////////////////////////////////////////////

class  testNode_c : public abcListNode_c
{
  private:
	char		*name;		// some private data
	int64_t		value;

  protected: 

  public:
	testNode_c(const char *name = NULL);	// method in cpp file
	virtual ~testNode_c();	// method in cpp file

	virtual char		*getObjType();	// the class name
	virtual char		*getObjName();	// the instance name when available
	virtual abcResult_e print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_WITH_NODE_DETAILS);
	abcResult_e			printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); //nonVirtual prinfbuff for this level of the object hiearchy

	virtual abcListNode_c *clone();
	abcResult_e copyOut(testNode_c *targetOfCopy);

	virtual abcResult_e diffNode(class abcListNode_c *other);	// compare entire node

	// specific behavior
	void setName(const char *setName);	// not key
	void setValue(const int setValue);
};



#endif //__ABC_NODES_H__

// EOF abcNodes.h

