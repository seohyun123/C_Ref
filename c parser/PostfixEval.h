// $Id: PostfixEval.h 1919 2009-02-13 10:40:19Z /C=FR/O=CNRS/OU=UMR5821/CN=Guillaume Dargaud/emailAddress=dargaud@lpsc.in2p3.fr $

#ifndef __POSTFIX_EVAL_H
#define __POSTFIX_EVAL_H

#include "Def.h"	// For BOOL, TRUE, FALSE

// Available types. This is more or less arbitrary
#ifndef VAL_DOUBLE
#define VAL_DOUBLE 4	// CVI compatibility
#endif
#define VAL_BOOL 12
#define VAL_ERROR (-1)

#define STACK_SIZE 16

// Error codes
enum {
	PE_NOERROR,
	PE_MEMOUT,
	PE_PARSING,
	PE_MISSINGVAR,
	PE_WRONGTYPE,
	PE_MATH,
	PE_INF,
	PE_NAN,
	PE_EDOM,
	PE_ERANGE,
	PE_STACKEMPTY,
	PE_STACKUNDERFLOW,
	PE_STACKOVERFLOW,
	PE_REMAIN,
	PE_INVALID,
	PE_UNKNOWN,
	PE_INVALIDTYPE,
};


// Points to a variable in memory
typedef struct sPE_Variable {
	void* pVal;
	char *Name;
	int Type;	// Indicates what the pointer points to: VAL_DOUBLE, VAL_BOOL
} tPE_Variable;

typedef struct sPE_Val {
	union {
		double D;	// For doubles
		BOOL B;		// For booleans
		int Err;	// For error codes
	};
	int Type;		// Indicate what's in the union: VAL_DOUBLE, VAL_BOOL, VAL_ERROR
} tPE_Val;

extern tPE_Val PostfixEval_Compute(const char* const Formula, 
	const tPE_Variable VarList[], const int NbVar,
	char* Evaluation);
extern char* PostfixEval_ErrMsg(const int Err);
extern char *PostfixEval_Help(const tPE_Variable VarList[], const int NbVar, const BOOL DispVal, const BOOL UseColors);



#endif
