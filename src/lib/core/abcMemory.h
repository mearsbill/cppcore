/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
// Author: Bill Mears   bill@imears.com
// File:  abcMemory.h
// 
//

#ifndef __ABC_MEMORY_H__
#define __ABC_MEMORY_H__

#include "abcList.h"		// we use the various lists available to us.... and make our own abcListNode_c

typedef enum abcMmapType_e
{
	ABC_MMAP_TYPE_UNKNOWN = 0,
	ABC_MMAP_TYPE_RAW,
	ABC_MMAP_TYPE_STRUCT,
	ABC_MMAP_TYPE_CLASS,
	ABC_MMAP_TYPE_INVALID
} abcMmapType_e;


//
// this object holds names.  they are for both instance name and for object name
// they are just names for identification
//

class abcMemNameNode_c : public abcListNode_c	// a node just for names and indexed by nameHash
{
	private:

		// uint64_t			nameHash; 	// stored in the nodeKey !!
		char				*name;
		int64_t				useCount;
		CONFIG_t			configBits;

	public:
		friend class abcMemMon_c;

		abcMemNameNode_c(uint64_t hash, const char *name, int setUseCount = 0);
		~abcMemNameNode_c();

		// things we don't do we must cause to fail
		virtual char       		 *getObjType();  // the class name
		virtual char       		 *getObjName();  // the instance name when available
		virtual abcResult_e		 print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_MEM_NAME_LIST);
		abcResult_e        		 printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); //nonVirtual prinfbuff for this level of the object hiearchy
		virtual abcListNode_c	 *clone();
		abcResult_e				 copyOut(abcMemNameNode_c *targetOfCopy);
		virtual abcResult_e		diffNode(class abcListNode_c *other);   // compare entire node



		// static helper methods for building sting 
		//
		// Instance Name
		//    fileName:fileLine:function"
		//
		// Object Name
		// for raw	  = "R:storageSize"
		// for struct = "S:structName"
		// for classo = "C:className"
		//
		static char *buildLocationString(const char *fileName, const int fileLine, const char *function);
		static char	*buildObjectClassString(const char *className);  // for raw, build a string and a
		static char	*buildObjectStructString(const char *structName);  // for raw, build a string and a
		static char	*buildObjectRawString(const int size);  // helper for type raw
		//
		static uint64_t	calcNameHash(const char *string);  // consistent place to get your hash
		//
		int64_t		incrUseCount();
		int64_t		getUseCount();
}; // end class abcMemNameNode_c

//
// This object is used to store stats data on particular object types.
// The list we use is a hashedList hashed by object name for quck lookup
class abcMemStatsNode_c : public abcListNode_c
{
	// the key on these records will be the same as mmpaObjName as stored in the abcMmapNode_c object
	private:
		// uint64_t	objTypeNameHash	 			// stored in the nodeKey 
		char				*objTypeName;	 // ptr to the objet name
		int64_t				createCount;
		int64_t				destroyCount;
		int32_t				size;

	public:
		friend class abcMemMon_c;

		abcMemStatsNode_c(uint64_t objTypeHash, const char *objTypeName, int size, int setCreateCount=1);
		~abcMemStatsNode_c();


		// things we don't do we must cause to fail
		virtual char       		 *getObjType();  // the class name
		virtual char       		 *getObjName();  // the instance name when available
		virtual abcResult_e		 print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_MEM_STATS_LIST);
		abcResult_e        		 printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); //nonVirtual prinfbuff for this level of the object hiearchy
		virtual abcListNode_c	 *clone();
		abcResult_e				 copyOut(abcMemStatsNode_c *targetOfCopy);
		virtual abcResult_e		diffNode(class abcListNode_c *other);   // compare entire node

		// report out
		int		getCreateCount();
		int		getDestroyCount();
		int		getSize();

		abcResult_e		incrCreates(int count =1);			// default increment is 1
		abcResult_e		incrDestroys(int count =1);			// defualt decrement is 1
}; // end class abcMemStatsNode_c


class abcMmapNode_c : public abcListNode_c			// hashlist node keyed by memory address
{
	private:
		//void				*mmapPtr;			// stored in the nodeKey
		abcMemStatsNode_c	*statsPtr;			// stats for all of the same object type
		abcMemNameNode_c	*locNewPtr;		// node with full data on where it was creasted [ expecing many repeats ! ]
		abcMemNameNode_c	*locDelPtr;		// node with full data on where it was allocated [ expecing many repeats ! ]

	public:
		friend class abcMemMon_c;

		abcMmapNode_c(void *locPtr);			// inited with the address of the allocated memory
		~abcMmapNode_c();


		// things we don't do we must cause to fail
		virtual char       		 *getObjType();  // the class name
		virtual char       		 *getObjName();  // the instance name when available
		virtual abcResult_e		 print(abcPrintStyle_e printStyle=PRINT_STYLE_LIST_MMAP_LIST);
		abcResult_e        		 printBuff(char *pBuff, int pbuffSize, abcPrintStyle_e printStyle); //nonVirtual prinfbuff for this level of the object hiearchy
		virtual abcListNode_c	 *clone();
		abcResult_e				 copyOut(abcMmapNode_c *targetOfCopy);
		virtual abcResult_e		diffNode(class abcListNode_c *other);   // compare entire node

		abcResult_e			setStatsNode(abcMemStatsNode_c *stats); 
		abcMemStatsNode_c	*getStatsNode();				             
		abcResult_e			setLocNewPtr(abcMemNameNode_c *newLoc);         
		abcMemNameNode_c	*getLocNewPtr();  				
		abcResult_e			setLocDelPtr(abcMemNameNode_c *delLoc);       
		abcMemNameNode_c	*getLocDelPtr();
};


////////////////  ************************  //////////////////
////////////////     abcMemMon_c            //////////////////
////////////////  ************************  //////////////////
// this object + macros handles memory and leak tracking
//
class abcMemMon_c // : public abcListNode_c
{
  private:
  	char			*name;
	CONFIG_t		configBits;
  	abcHashList_c	*mmapList;			// hashed by address using abcMmapNode_c nodes
  	abcHashList_c	*delmapList;		// hashed by address using abcMmapNode_c nodes... all the deleted memory segments
	abcHashList_c	*nameList;			// hashed by name. Hold objectTypeNames and locationStrings using  abcMemNameNode_c
	abcHashList_c	*statsList;			// hashed by objectName using abcMemStatsNode_c;

	int64_t			netBytesInUse;
	int64_t			maxMemoryUsed;
	int64_t			totalBytesMalloced;
	int64_t			totalBytesFreed;
	int64_t			netMallocsInUse;
	int64_t			maxMallocsUsed;
	int64_t			totalMallocCount;
	int64_t			totalFreeCount;

  protected:
	abcReason_e	reason;

  public:
	abcMemMon_c(const char *setName = NULL);
	~abcMemMon_c();

	// errror handling
	void resetReason();
	void setReason(abcReason_e reason);
	abcReason_e getReason();


	// public methods
	void *interceptCommonNew(uint8_t useMt,void *objAddr, char *objHashName, int objectSize, char *fileName, int fileLine, char *fileFunction);
	void *interceptClassNew(uint8_t useMt,void *objAddr, char *className, int objectSize, char *fileName, int fileLine, char *fileFunction);
	void *interceptStructNew(uint8_t useMt,void *structAddr, char *structName, int objectSize, char *fileName, int fileLine, char *fileFunction);
	void *interceptRawNew(uint8_t useMt,void *rawAddr, int objectSize, char *fileName, int fileLine, char *fileFunction);
	char *interceptStrdup(uint8_t useMt,char *string);

	// same routine works for all delete of all oject types..
	void interceptDelete(uint8_t useMt,void *objAddr, char *fileName, int fileLine, char *fileFunction);

	void printMemoryMap();
	void printMemoryStats();
	void shutdown(int shutdownVal);	// shutdownVal control reporting

};// end class abcMemMon_c 

// the macros !!

#define ABC_NEW_CLASS(useMt,a,b...)  (a *)globalMem->interceptClassNew(useMt,(void *)(new a(b)),(char *)STRINGIFY(a),sizeof(a),(char *)__FILE__,__LINE__,(char *)__FUNCTION__)
#define ABC_NEW_STRUCT(useMt,a)  (a *)globalMem->interceptStructNew(useMt,(void *)(calloc(1,sizeof(a)),(char *)STRINGIFY(a),sizeof(a),(char *)__FILE__,__LINE__,(char *)__FUNCTION__)
#define ABC_MALLOC(useMt,mSize)  globalMem->interceptRawNew(useMt,malloc(mSize),mSize,(char *)__FILE__,__LINE__,(char *)__FUNCTION__)
#define ABC_CALLOC(useMt,cCount, cSize)  globalMem->interceptRawNew(useMt,calloc(cCount,cSize),(cCount * cSize),(char *)__FILE__,__LINE__,(char *)__FUNCTION__)
#define ABC_STRDUP(useMt,string)  globalMem->interceptStrdup(useMt,string )

#define ABC_DEL_CLASS(useMt,objPtr)     globalMem->interceptDelete(useMt,(void *)objPtr,(char *)__FILE__,__LINE__,(char *)__FUNCTION__); delete objPtr
#define ABC_DEL_STRUCT(useMt,structPtr) globalMem->interceptDelete(useMt,(void *)structPtr,(char *)__FILE__,__LINE__,(char *)__FUNCTION__);free (structPtr)
#define ABC_FREE(useMt,freePtr)         globalMem->interceptDelete(useMt,(void *)freePtr,(char *)__FILE__,__LINE__,(char *)__FUNCTION__);free(freePtr)



#endif //__ABC_MEMORY_H__

////////////////////////////
// EOF abcMemory.h

