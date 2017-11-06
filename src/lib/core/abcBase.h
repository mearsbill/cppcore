/*******************************************************
 ****   abcCorp Copyright 2017 All Rights reserved *****
 *******************************************************/
 // Author Bill Mears   bill@imears.com
 // abcCore.h   This is the core .h file.   Everyone should include this.
 //
 #ifndef __ABC_BASE_H__
 #define __ABC_BASE_H__

// Common system includes here.
extern "C" 
{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>
}

// go here for our enum values.
#include "abcTypes.h"
#include "abcEnums.h"
#include "abcConfig.h"
#include "abcDebugMacros.h"


// wierd stuff
#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)


#endif //__ABC_BASE_H__
// EOF abcBase.h
