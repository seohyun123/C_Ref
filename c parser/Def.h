/**************************************************************************************************
	FILE:       def.h

	PURPOSE:    General declarations and macros. Used by both MSVC and LabWindows/CVI
	NOTE:       The macros _NI_mswin_ and _MSC_VER controls whether to compile for CVI or MSVC
**************************************************************************************************/
#ifndef _DEF
#define _DEF

#include <iso646.h>	// for 'and', 'or'...

#ifndef NULL
	#define NULL 0
#endif

#define NAN ((float)1e-30) /* Not A Number - Indicate a wrong or non significant value - 
					 Must always be tested, i.e. never assume that it's "close to zero, anyway" !
					 Could be any value, esp IEEE error or infinity formats. */

#ifdef _CVI_
	#include <toolbox.h>	// you need to include toolbox.fp in your project
	#define IS_NAN(x) (FP_Compare((x),NAN)==0)
#endif

#define Pi              3.1415926535897932384626433832795028841971
#define RAD(x)          ((x)*PI/180.0)
#define SINC(x)         ((x) != 0.0  ? sin(PI*(x))/(PI*(x)): 1.0)
#define Ln10     2.30258509298    // For inverse of log10. Use 10^x==exp(x*Ln10)
#define DegToRad 0.017453292519943295769237 
#define RadToDeg 57.29577951     // Use * for conversions between degrees and radians

#define MS2CVI_TIME 2208902400   // Because MS time starts at 1/1/1970 and ANSI C at 1/1/1900
//#define MS2CVI_TIME 2208978000   // Because MS time starts at 1/1/1970 and ANSI C at 1/1/1900
#define IsCviTime(t) ((t)>=MS2CVI_TIME)	// Check if a date after 1970 is in CVI format

#define Malloc(type)   (type *)malloc(sizeof(type))
#define Calloc(n,type) (type *)calloc(n, sizeof(type))
// Syntax:   float *Array=Calloc(1000, float);

#define MIN(a,b) ((a)<=(b)?(a):(b))
#define MAX(a,b) ((a)>=(b)?(a):(b))
#define MIN3(a,b,c) ((a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c) )
#define MAX3(a,b,c) ((a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c) )
#define BETWEEN(a,b,c) ((a)<=(b) and (b)<=(c))

typedef struct {
	float Re, Im;
} complex;

//typedef enum {false, true} bool;

// Basic types (from Windows)
#pragma warning(disable:4209 4142)

typedef unsigned char  BYTE;        // 8-bit unsigned entity
typedef unsigned short WORD;        // 16-bit unsigned number
typedef unsigned int   UINT;        // machine sized unsigned number (preferred)
//typedef long           LONG;      // 32-bit signed number
typedef unsigned long  DWORD;       // 32-bit unsigned number
//typedef short          BOOL;      // BOOLean (0 or !=0)
typedef void*          POSITION;    // abstract iteration position

#define TRUE 1
#define FALSE 0
#define YES 1
#define NO 0
#define OPEN 1
#define CLOSE 0
#define MODIF 1
#define NOMODIF 0

#pragma warning(default:4209 4142)


#ifdef _DEBUG
	// NOTE: you should #include <stdlib.h> if you use those macros
	#define TRACE     printf
	#define ASSERT(f) ((f) ? (void)0 : printf("Assertion " #f " failed file %s line %d",__FILE__, __LINE__))
#else
	#define ASSERT(f) ((void)0)
	#define TRACE     ((void)0)
#endif // _DEBUG


#endif // _DEF
