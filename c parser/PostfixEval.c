///////////////////////////////////////////////////////////////////////////////
// MODULE	PostfixEval
// PURPOSE	This is an extremely simple math parser
//			- postfix notation without optimization for multiple calls, 
//			- uses program variables,
//			- static stack size (easy to extend of make dynamic),
//			- only 2 types (double and boolean), allows for logical operations,
//			- proper error control
// NOTE		ANSI C, with a few additions easy to remove
// EXAMPLE	See the test cases at the end of this file
// SVN		$Id: PostfixEval.c 1969 2009-03-18 13:53:20Z /C=FR/O=CNRS/OU=UMR5821/CN=Guillaume Dargaud/emailAddress=dargaud@lpsc.in2p3.fr $
///////////////////////////////////////////////////////////////////////////////

#include <iso646.h>
#include <utility.h>	// Only for Library error runtime checks, can be removed

#include "PostfixEval.h"

///////////////////////////////////////////////////////////////////////////////
// FUNCTION	strcasecmp
/// HIFN	Case insensitive compare (note: some stdlib have his already)
/// HIPAR	s1/First string to compare
/// HIPAR	s2/Second string to compare
/// HIRET	0 if the strings are identical, otherwise <0 or >0
///////////////////////////////////////////////////////////////////////////////
#ifndef HAVE_STRCASECMP
#define HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2) {
    while (toupper(*s1) == toupper(*s2)) {
        if (*s1=='\0') return 0;
        s1++;
        s2++;
    }
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}
#endif


// Because of union complaints
#pragma DisableUninitLocalsChecking

static char* Pos=NULL;	// Also used by the error message

///////////////////////////////////////////////////////////////////////////////
// FUNCTION	PostfixEval_Compute
/// HIFN	Perform a math evaluation of a postfix formula
/// ARRAY	VarList
/// OUT		Evaluation
/// HIPAR	Formula/String with the formula to use, tokens are separated by spaces
/// HIPAR	VarList/Array of variables which can be used in the formula
/// HIPAR	NbVar/Number of variables
/// HIPAR	Evaluation/Debug string (pass NULL to skip). Must have a size at least 10 times the formula
/// HIRET	Return the result. Check its type before using the value
///////////////////////////////////////////////////////////////////////////////
tPE_Val PostfixEval_Compute(const char* const Formula, 
	const tPE_Variable VarList[], const int NbVar,
	char* Evaluation) {
	tPE_Val Ret;
	int Prev, i, StackTop=0;	// Top free element on the stack
	tPE_Val Stack[STACK_SIZE];
	static char *Copy=NULL;
	
	Pos=NULL;
	if (Copy==NULL or strlen(Copy)<strlen(Formula)) 
		Copy = realloc      (Copy, strlen(Formula)+1);
	if (Copy==NULL) { Ret.Err=PE_MEMOUT; goto Error; }
	strcpy(Copy, Formula);
	
	// Init, just to avoid uninit warnings
	Ret.Err=PE_STACKEMPTY;
	Ret.Type=VAL_ERROR;
	for (i=0; i<STACK_SIZE; i++) { 
		Stack[i].Type=VAL_ERROR; 
		Stack[i].Err=PE_INVALID; 
	}

	if (Evaluation!=NULL) Evaluation[0]='\0';
	Prev = SetBreakOnLibraryErrors (0);
	
	Pos=strtok (Copy, " ");
	while (Pos!=NULL) {
		errno=0;
		///////////////////////////////////////////////////////////////////////
		#define IF(St) if (strcasecmp(St, Pos)==0) 

		#define Top 	Stack[StackTop-1]	// Element on top of the stack
		#define Second	Stack[StackTop-2]	// Element below the top of the stack
		#define Third	Stack[StackTop-3]	// Third element from the top of the stack

		#define WRONG_TYPE { Ret.Err=PE_WRONGTYPE; goto Error; }
		
		// Check there there is at least 1, 2 or 3 operands on the stack
		#define Check1	{ if (StackTop<1) { Ret.Err=PE_STACKUNDERFLOW; goto Error; } }
		#define Check2	{ if (StackTop<2) { Ret.Err=PE_STACKUNDERFLOW; goto Error; } }
		#define Check3	{ if (StackTop<3) { Ret.Err=PE_STACKUNDERFLOW; goto Error; } }
		
		// Check that there is at least 1 Bool, 2 Bool, 1 Double, etc, on the stack
		#define CheckB1 { Check1; if (Top.Type!=VAL_BOOL) WRONG_TYPE }
		#define CheckB2 { Check2; if (Top.Type!=VAL_BOOL or Second.Type!=VAL_BOOL) WRONG_TYPE }
		#define CheckD1 { Check1; if (Top.Type!=VAL_DOUBLE) WRONG_TYPE }
		#define CheckD2 { Check2; if (Top.Type!=VAL_DOUBLE or Second.Type!=VAL_DOUBLE) WRONG_TYPE }
		#define CheckD3 { Check3; if (Top.Type!=VAL_DOUBLE or Second.Type!=VAL_DOUBLE or Third.Type!=VAL_DOUBLE) WRONG_TYPE }
		#define CheckDA { Check1; int j; for (j=0; j<StackTop; j++) if (Stack[j].Type!=VAL_DOUBLE) WRONG_TYPE }
		#define CheckBDD {Check3; if (Top.Type!=VAL_DOUBLE or Second.Type!=VAL_DOUBLE or Third.Type!=VAL_BOOL  ) WRONG_TYPE }
		
		// Set the type of a result, check for error, display it and continue
		#define EVB { Top.Type=VAL_BOOL;   if (Evaluation!=NULL) sprintf(Evaluation, "%s, %s:%d", Evaluation, Pos, Top.B); goto Next; }
		#define EVD { Top.Type=VAL_DOUBLE; if (Evaluation!=NULL) sprintf(Evaluation, "%s, %s:%f", Evaluation, Pos, Top.D); \
						if (IsInfinity  (Top.D)) { Ret.Err=PE_INF; goto Error; }	\
						if (IsNotANumber(Top.D)) { Ret.Err=PE_NAN; goto Error; }	\
						goto Next; }

		///////////////////////////////////////////////////////////////////////
		// Start of the operator detection
		// If you add operators, put them also in the LIST_* macros
		
		// Boolean unary from boolean
		#define LIST_BuB "not"
		IF("not") { CheckB1; Top.B = ( Top.B ? FALSE : TRUE ); EVB; }	// I have no idea why, but doing Top.B=!Top.B fails when given 1
		
		// Boolean binary from booleans. See "iso646.h"
		#define LIST_BbBB "and or xor eqv neqv nand nor"
		IF("and") { CheckB2; Second.B= (Second.B and Top.B); StackTop--; EVB; }
		IF("or")  { CheckB2; Second.B= (Second.B or  Top.B); StackTop--; EVB; }
		IF("xor") { CheckB2; Second.B= (Second.B xor Top.B); StackTop--; EVB; }
		IF("eqv") { CheckB2; Second.B= (Second.B ==  Top.B); StackTop--; EVB; }
		IF("neqv"){ CheckB2; Second.B= (Second.B !=  Top.B); StackTop--; EVB; }
		IF("nand"){ CheckB2; Second.B=!(Second.B and Top.B); StackTop--; EVB; }
		IF("nor") { CheckB2; Second.B=!(Second.B or  Top.B); StackTop--; EVB; }

		// Boolean binary from doubles
		#define LIST_BbDD "< > <= >= == !="
		/* This is imprecise due to rounding
		IF("<")  { CheckD2; Second.B=(Second.D <  Top.D); StackTop--; EVB; }
		IF(">")  { CheckD2; Second.B=(Second.D >  Top.D); StackTop--; EVB; }
		IF("<=") { CheckD2; Second.B=(Second.D <= Top.D); StackTop--; EVB; }
		IF(">=") { CheckD2; Second.B=(Second.D >= Top.D); StackTop--; EVB; }
		IF("=")  { CheckD2; Second.B=(Second.D == Top.D); StackTop--; EVB; }
		IF("==") { CheckD2; Second.B=(Second.D == Top.D); StackTop--; EVB; }
		IF("!=") { CheckD2; Second.B=(Second.D != Top.D); StackTop--; EVB; }
		IF("<>") { CheckD2; Second.B=(Second.D != Top.D); StackTop--; EVB; }	*/
		
		IF("<")  { CheckD2; Second.B=FP_Compare(Second.D,Top.D) <  0; StackTop--; EVB; }
		IF(">")  { CheckD2; Second.B=FP_Compare(Second.D,Top.D) >  0; StackTop--; EVB; }
		IF("<=") { CheckD2; Second.B=FP_Compare(Second.D,Top.D) <= 0; StackTop--; EVB; }
		IF(">=") { CheckD2; Second.B=FP_Compare(Second.D,Top.D) >= 0; StackTop--; EVB; }
		IF("=")  { CheckD2; Second.B=FP_Compare(Second.D,Top.D) == 0; StackTop--; EVB; }
		IF("==") { CheckD2; Second.B=FP_Compare(Second.D,Top.D) == 0; StackTop--; EVB; }
		IF("!=") { CheckD2; Second.B=FP_Compare(Second.D,Top.D) != 0; StackTop--; EVB; }
		IF("<>") { CheckD2; Second.B=FP_Compare(Second.D,Top.D) != 0; StackTop--; EVB; }
		
		#define CheckErr {   if (errno==EDOM)	{ Ret.Err=PE_EDOM;	goto Error; }	\
						else if (errno==ERANGE)	{ Ret.Err=PE_ERANGE;goto Error; }	\
						else if (errno!=0)		{ Ret.Err=PE_MATH;	goto Error; }	\
						EVD; }

		// Double unary operators from double
		#define LIST_DuD "abs ceil floor frac sign, sin cos tan asin acos atan, sinh cosh tanh, ln log10 exp sqr sqrt, RadToDeg DegToRad CToK KToC"
		IF("abs")	{ CheckD1; Top.D= fabs	(Top.D); CheckErr; }
		IF("ceil")	{ CheckD1; Top.D= ceil	(Top.D); CheckErr; }
		IF("floor")	{ CheckD1; Top.D= floor	(Top.D); CheckErr; }
		IF("frac")	{ CheckD1; Top.D-=floor (Top.D); CheckErr; }	// Warning if negative
		IF("sign")	{ CheckD1; Top.D=Top.D>0?1:Top.D<0?-1:0; CheckErr; }
		
		IF("sin")	{ CheckD1; Top.D= sin	(Top.D); CheckErr; }
		IF("cos")	{ CheckD1; Top.D= cos	(Top.D); CheckErr; }
		IF("tan")	{ CheckD1; Top.D= tan	(Top.D); CheckErr; }
		IF("asin")	{ CheckD1; Top.D= asin	(Top.D); CheckErr; }
		IF("acos")	{ CheckD1; Top.D= acos	(Top.D); CheckErr; }
		IF("atan")	{ CheckD1; Top.D= atan	(Top.D); CheckErr; }
		
		IF("sinh")	{ CheckD1; Top.D= sinh	(Top.D); CheckErr; }
		IF("cosh")	{ CheckD1; Top.D= cosh	(Top.D); CheckErr; }
		IF("tanh")	{ CheckD1; Top.D= tanh	(Top.D); CheckErr; }
		
		IF("ln")	{ CheckD1; Top.D= log	(Top.D); CheckErr; }
		IF("log10")	{ CheckD1; Top.D= log10	(Top.D); CheckErr; }
		IF("exp")	{ CheckD1; Top.D= exp	(Top.D); CheckErr; }
		IF("sqr")	{ CheckD1; Top.D*=       Top.D;  CheckErr; }
		IF("sqrt")	{ CheckD1; Top.D= sqrt	(Top.D); CheckErr; }
		
		IF("RadToDeg")	{ CheckD1; Top.D=       RAD_TO_DEG(Top.D); CheckErr; }
		IF("DegToRad")	{ CheckD1; Top.D=       DEG_TO_RAD(Top.D); CheckErr; }
		IF("CToK")		{ CheckD1; Top.D=CELSIUS_TO_KELVIN(Top.D); CheckErr; }
		IF("KToC")		{ CheckD1; Top.D=KELVIN_TO_CELSIUS(Top.D); CheckErr; }

		// Double binary operators from doubles
		#define LIST_DbDD "+ - * / %% ^, atan2 hypot, max2 min2 fmod"
		IF("+")		{ CheckD2; Second.D=     (Second.D  +  Top.D); 			StackTop--; EVD; }
		IF("-")		{ CheckD2; Second.D=     (Second.D  -  Top.D); 			StackTop--; EVD; }
		IF("*")		{ CheckD2; Second.D=     (Second.D  *  Top.D); 			StackTop--; EVD; }
		IF("/")		{ CheckD2; Second.D=     (Second.D  /  Top.D); 			StackTop--; EVD; }
		IF("%")	    { CheckD2; Second.D=100.*(Second.D  /  Top.D);			StackTop--; EVD; }
		IF("^")		{ CheckD2; Second.D=pow  (Second.D,    Top.D);			StackTop--; CheckErr; }
		IF("atan2")	{ CheckD2; Second.D=atan2(Second.D,    Top.D);			StackTop--; CheckErr; }
		IF("max2")	{ CheckD2; Second.D=MAX  (Second.D,    Top.D);			StackTop--; CheckErr; }
		IF("min2")	{ CheckD2; Second.D=MIN  (Second.D,    Top.D);			StackTop--; CheckErr; }
		IF("hypot")	{ CheckD2; Second.D=sqrt (Second.D*Second.D+Top.D*Top.D);StackTop--;CheckErr; }
		IF("fmod")	{ CheckD2; Second.D=fmod (Second.D,    Top.D);			StackTop--; CheckErr; }

		// Double ternary operators from 1 bool and 2 of either type
		#define LIST_DtBDD "?:"
		IF("?:")  { Check3; if (Third.Type!=VAL_BOOL) WRONG_TYPE
			if (Third.B) switch (   Third.Type=Second.Type) {
					case VAL_BOOL:  Third.B=   Second.B; break;
					case VAL_DOUBLE:Third.D=   Second.D; break;
			} else switch (         Third.Type=Top.Type) {
					case VAL_BOOL:  Third.B=   Top.B;    break;
					case VAL_DOUBLE:Third.D=   Top.D;    break;
			}
			StackTop-=2; EVD;
		}

		// Double multiple operators from the entire stack
		#define LIST_Dm "sum avg prod, max min"
		IF("sum")  { CheckDA; for (i=1; i<StackTop; i++) Stack[0].D +=  Stack[i].D;                         StackTop=1; EVD; }
		IF("prod") { CheckDA; for (i=1; i<StackTop; i++) Stack[0].D *=  Stack[i].D;                         StackTop=1; EVD; }
		IF("avg")  { CheckDA; for (i=1; i<StackTop; i++) Stack[0].D +=  Stack[i].D; Stack[0].D/=StackTop;   StackTop=1; EVD; }
		IF("max")  { CheckDA; for (i=1; i<StackTop; i++) if (Stack[0].D<Stack[i].D) Stack[0].D =Stack[i].D; StackTop=1; EVD; }
		IF("min")  { CheckDA; for (i=1; i<StackTop; i++) if (Stack[0].D>Stack[i].D) Stack[0].D =Stack[i].D; StackTop=1; EVD; }
		
		// Bool multiple operators from the entire stack
		#define LIST_Bm "within"
		IF("within"){Check3; CheckDA; int j; BOOL B=TRUE;	// Check that A B C D Z all agree to abs([A..D]-[A..D])<Z
						for (i=0; i<StackTop-1; i++) 
							for (j=i+1; j<StackTop-1; j++) 
								B&=fabs(Stack[i].D-Stack[j].D)<Top.D; Stack[0].B=B; StackTop=1; EVB; }

		// Also possible to add type-agnostic stack operators
		#define LIST_StackOp "drop switch dup"
		IF("drop")	{ Check1; StackTop--;                             goto Next; }
		IF("switch"){ tPE_Val V; Check2; V=Second; Second=Top; Top=V; goto Next; }
		IF("dup")	{ Check1; StackTop++; Top=Second;                 goto Next; }

		
		///////////////////////////////////////////////////////////////////////
		// Start of the variable name detection //
		
		#define CheckOver if (StackTop==STACK_SIZE) { Ret.Err=PE_STACKOVERFLOW; goto Error; }
		for (i=0; i<NbVar; i++) {
			if (VarList[i].Name==NULL or VarList[i].Name[0]=='\0') continue;
			IF(VarList[i].Name) {
				CheckOver;
				StackTop++;
				switch (VarList[i].Type) {
					case VAL_DOUBLE:Top.Type=VAL_DOUBLE; Top.D=*(double*)VarList[i].pVal; EVD;
					case VAL_BOOL:	Top.Type=VAL_BOOL;   Top.B=*(BOOL  *)VarList[i].pVal; EVB;
					default:		Ret.Err=PE_INVALIDTYPE; goto Error;
				}
			}
		}
		
		///////////////////////////////////////////////////////////////////////
		// Start of the constant detection //
		
		// If you add operators, put them also in the LIST_* macros
		#define LIST_BOOL "TRUE FALSE randB"
		IF("FALSE")		{ CheckOver; StackTop++; Top.Type=VAL_BOOL;   Top.B=0;    				EVB; }
		IF("TRUE")		{ CheckOver; StackTop++; Top.Type=VAL_BOOL;   Top.B=1;    				EVB; }
		IF("randB")		{ CheckOver; StackTop++; Top.Type=VAL_BOOL;   Top.B=rand()&1;    		EVB; }
		
		#define LIST_DOUBLE "pi euler, Planck ElemCharge LightSpeed Grav Avogadro Rydberg MolarGas, rand"
		IF("PI")		{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=PI;					EVD; }
		IF("Euler")		{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=EULER;				EVD; }
		
		IF("Planck")	{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=PLANCK_CONSTANT;	EVD; }
		IF("ElemCharge"){ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=ELEMENTARY_CHARGE;	EVD; }
		IF("LightSpeed"){ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=SPEED_OF_LIGHT;		EVD; }
		IF("Grav")		{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=GRAVITATIONAL_CONSTANT; EVD; }
		IF("Avogadro")	{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=AVOGADRO_CONSTANT;	EVD; }
		IF("Rydberg")	{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=RYDBERG_CONSTANT;	EVD; }
		IF("MolarGas")	{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=MOLAR_GAS_CONSTANT;	EVD; }
		// Yes, I know, rand is not a const...
		IF("rand") 		{ CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=(double)rand()/RAND_MAX; EVD; }
		
		// Manual value
		if (strspn(Pos, ".0123456789+-eE")==strlen(Pos)) {
			CheckOver; StackTop++; Top.Type=VAL_DOUBLE; Top.D=atof(Pos); CheckErr;
		}

		// Everything else
		Ret.Err=PE_UNKNOWN; goto Error;
		
		///////////////////////////////////////////////////////////////////////
		Next:	Pos=strtok(NULL, " ");
	}
	switch (StackTop) {
		default:Ret.Err=PE_REMAIN;		break;
		case 0: Ret.Err=PE_STACKEMPTY;	break;
		case 1: SetBreakOnLibraryErrors(Prev); /*free(Copy);*/ return Stack[0];	// Valid computation returns here
	}

Error:
	SetBreakOnLibraryErrors(Prev);
	Ret.Type=VAL_ERROR; 
//	if (Copy!=NULL) free(Copy);
	return Ret;
}
	
///////////////////////////////////////////////////////////////////////////////
// FUNCTION	PostfixEval_ErrMsg
/// HIFN	Return an error message from the Val.Err value
/// HIPAR	Err/Error code as found in Val.Err when Val.Type is VAL_ERROR
/// HIRET	Error string
///////////////////////////////////////////////////////////////////////////////
char* PostfixEval_ErrMsg(const int Err) {
	static char Str[80]="";
	switch (Err) {
		default:
		case PE_NOERROR:		return "No error";
		
		case PE_MEMOUT:			return "Out of memory";
		
		case PE_PARSING:		if (Pos==NULL) return "Parsing error (invalid formula)";
								else sprintf(Str,     "Parsing error (invalid formula) at token %s", Pos);
								return Str;
		case PE_MISSINGVAR:		return 				  "Missing variable";
		
		case PE_MATH:			if (Pos==NULL) return "Math error";								// Cannot happen ?
								else sprintf(Str,     "Math error at token %s", Pos);
								return Str;
		case PE_INF:			if (Pos==NULL) return "Infinity reached during computation";	// Happens with 1e300*1e300
								else sprintf(Str,     "Infinity reached during computation at token %s", Pos);
								return Str;
		case PE_NAN:			if (Pos==NULL) return "Result is not a number (NaN)";
								else sprintf(Str,     "Result is not a number (NaN) at token %s", Pos);
								return Str;
		case PE_EDOM:			if (Pos==NULL) return "Operand not in domain accepted by operator";
								else sprintf(Str,     "Operand not in domain accepted by operator %s", Pos);
								return Str;
		case PE_ERANGE:			if (Pos==NULL) return "Result out of range";
								else sprintf(Str,     "Result out of range at operator %s", Pos);
								return Str;
		case PE_WRONGTYPE:		if (Pos==NULL) return "Wrong operand(s) type for this operator";
								else sprintf(Str,     "Wrong operand(s) type for operator %s", Pos);
								return Str;
		case PE_STACKEMPTY:		if (Pos==NULL) return "Empty stack";
								else sprintf(Str,     "Empty stack at operator %s", Pos);
								return Str;
		case PE_STACKOVERFLOW:	if (Pos==NULL) sprintf(Str, "Stack overflow (only %d elements allowed)", STACK_SIZE);
								else           sprintf(Str, "Stack overflow (only %d elements allowed) at token %s", STACK_SIZE, Pos);
								return Str;
		case PE_STACKUNDERFLOW:	if (Pos==NULL) return "Stack underflow (not enough operands available for this operator)";
								else sprintf(Str,     "Stack underflow (not enough operands available for operator %s)", Pos);
								return Str;
		case PE_REMAIN:			return                "More than one element remaining on stack at end of computation";
		
		case PE_INVALID:		if (Pos==NULL) return "Invalid value";
								else sprintf(Str,     "Invalid value %s", Pos);
								return Str;
		case PE_UNKNOWN:		if (Pos==NULL) return "Unknown token";
								else sprintf(Str,     "Unknown token %s", Pos);
								return Str;
		case PE_INVALIDTYPE:	if (Pos==NULL) return "Invalid variable type";
								else sprintf(Str,     "Invalid variable type for %s", Pos);
								return Str;
	}
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION	PostfixEval_ErrMsg
/// HIFN	Return a custom help message
/// ARRAY	VarList
/// HIPAR	VarList/Array of variables which can be used in the formula
/// HIPAR	NbVar/Number of variables
/// HIPAR	DispVal/Also display the values of the variables
/// HIPAR	VarPerLine/Max number of variable names per line (0 for unlimited)
/// HIRET	Return a string with help on multiple lines
///////////////////////////////////////////////////////////////////////////////
char *PostfixEval_Help(const tPE_Variable VarList[], const int NbVar, const BOOL DispVal, const BOOL UseColors, const int VarPerLine) {
	int i, VarCount;
	static char *Str=NULL;
	static int LastVar=-1;
	char VarCol[16]="", OpCol[16]="", ConstCol[16]="", NormCol[16]="";
	// This is approximate but should avoid most buffer overflow problems 
	// unless you have gigantic variable names or %f returns 32 digits on your system
	// You can free() the result ONLY if you don't call this function ever again.
	if (LastVar<NbVar)	// Grow only
		Str=realloc(Str, 2000+30*(LastVar=NbVar));
	
	if (UseColors) { 
		sprintf(VarCol,  "\033fg%06X", VAL_DK_RED);
		sprintf(OpCol,   "\033fg%06X", VAL_DK_GREEN); 
		sprintf(ConstCol,"\033fg%06X", VAL_DK_BLUE);
		sprintf(NormCol, "\033fg%06X", VAL_BLACK);
	}

	sprintf(Str, 
		"The evaluation is postfix, stack-based (reverse polish notation)\n"
		"The grammar for a formula is: Token Token Token...\n"
		"Where 'token' can be an Operator, a Variable, or a Constant (detected in that order, case insensitive)\n"
		"There are two types of operands, floats and booleans, which just add their value to the stack.\n"
		"Operators can act on 1, 2 or 3 operands or the entire stack and place one value on the stack.\n"
		"For non-commutative operators, think of the order as left to right: A B / is A/B.\n"
		"In most cases the _type_ of the operands is important.\n\n"
		"   Boolean unary operators returning a boolean: %s" LIST_BuB "\n"
		"   Boolean binary operators returning a boolean: %s" LIST_BbBB "\n"
		"   Float unary operators returning a float: %s" LIST_DuD "\n"
		"   Float binary operators returning a float: %s" LIST_DbDD "\n"
		"   Float binary operators returning a boolean: %s" LIST_BbDD "\n"
		"   Ternary operator from 1 bool and 2 any type returning any type: %s" LIST_DtBDD "\n"
		"   Operators acting on the entire stack returning a float: %s" LIST_Dm "\n"
		"   Operators acting on the entire stack returning a bool: %s" LIST_Bm "\n"
		"   Stack operations (type agnostic): %s" LIST_StackOp "\n\n"
		"Available float variables:\n   %s", OpCol, OpCol, OpCol, OpCol, OpCol, OpCol, OpCol, OpCol, OpCol, VarCol);
	for (i=VarCount=0; i<NbVar; i++) 
		if (VarList[i].Name!=NULL and VarList[i].Name!='\0' and VarList[i].Type==VAL_DOUBLE) {
			if (VarPerLine>0 and ++VarCount>VarPerLine) { sprintf(Str, "%s\n   %s", Str, VarCol); VarCount=0; }
			if (DispVal) sprintf(Str, "%s %s (%f)", Str, VarList[i].Name, *(double*)VarList[i].pVal);
			else sprintf(Str, "%s %s", Str, VarList[i].Name);
		}
	sprintf(Str, "%s\nAvailable boolean variables:\n   %s", Str, VarCol);
	for (i=VarCount=0; i<NbVar; i++) 
		if (VarList[i].Name!=NULL and VarList[i].Name!='\0' and VarList[i].Type==VAL_BOOL) {
			if (VarPerLine>0 and ++VarCount>VarPerLine) { sprintf(Str, "%s\n   %s", Str, VarCol); VarCount=0; }
			if (DispVal) sprintf(Str, "%s %s (%c)", Str, VarList[i].Name, *(BOOL*)VarList[i].pVal?'T':'F');
			else sprintf(Str, "%s %s", Str, VarList[i].Name);
		}
	sprintf(Str, "%s\n"
		"Warning, make sure not to use an operator or constant name as a variable name.\n"
		"Constants are tokenized last:\n\n"
		"   Boolean constants: %s" LIST_BOOL "\n"
		"   Float constants: %s-1.234e30 0 12 " LIST_DOUBLE "\n   %s(Inf and NaN are not allowed)\n"
		"At the end there should be exactly one bool or float value left on the stack: the result of the computation.\n"
		"An error code is returned otherwise.", 
		Str, ConstCol, ConstCol, NormCol);
	return Str;
}

#ifdef TEST_POSTFIXEVAL
///////////////////////////////////////////////////////////////////////////////
// FUNCTION	main
/// HIFN	Only for testing the PostfixEval code
///////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]) {
	double A1=12, D1=23, C1=1.234e5, C2=-20.5e-3, Beh=1;
	BOOL B1=TRUE, B2=FALSE;
	tPE_Variable VarList[10];
	tPE_Val Val;
	int NV=0;
	char Formula[255], 
		/*Evaluation[1024]*/ 	// returns an evaluation string useful for debugging the syntax
		*Evaluation=NULL;		// Does not
	
	VarList[NV].Name="A1";	VarList[NV].pVal=&A1;	VarList[NV++].Type=VAL_DOUBLE;
	VarList[NV].Name="D1";	VarList[NV].pVal=&D1;	VarList[NV++].Type=VAL_DOUBLE;
	VarList[NV].Name="B1";	VarList[NV].pVal=&B1;	VarList[NV++].Type=VAL_BOOL;
	VarList[NV].Name="B2";	VarList[NV].pVal=&B2;	VarList[NV++].Type=VAL_BOOL;
	VarList[NV].Name="C1";	VarList[NV].pVal=&C1;	VarList[NV++].Type=VAL_DOUBLE;
	VarList[NV].Name="C2";	VarList[NV].pVal=&C2;	VarList[NV++].Type=VAL_DOUBLE;
	VarList[NV].Name="13";	VarList[NV].pVal=&Beh;	VarList[NV++].Type=VAL_DOUBLE;	// Yes, we give the value of 1 to 13 (not a good idea) !!!

	// Display help
	puts(PostfixEval_Help(VarList, NV, TRUE));
	
	#define PE_TEST(String)	\
		strcpy(Formula, String);	\
		Val=PostfixEval_Compute(Formula, VarList, NV, Evaluation);	\
		switch (Val.Type) {	\
			case VAL_ERROR: printf("\n%s gave error \"%s\"\nExpected %s\nEvaluation=[%s]\n", 	\
									Formula, PostfixEval_ErrMsg(Val.Err), P, Evaluation==NULL?"N/A":Evaluation); break;	\
			case VAL_BOOL:  printf("\n%s is %s\nExpecting %s%s\nEvaluation=[%s]\n",	\
									Formula, Val.B?"TRUE":"FALSE", B?"TRUE":"FALSE", Val.B!=B?" DISCREPANCY!":"", Evaluation==NULL?"N/A":Evaluation);  break;	\
			case VAL_DOUBLE:printf("\n%s = %f\nExpecting %f%s\nEvaluation=[%s]\n",	\
									Formula, Val.D, R, Val.D!=R?" DISCREPANCY!":"", Evaluation==NULL?"N/A":Evaluation); break;	\
			default:		printf("\nError, invalid result type %d\nEvaluation=[%s]\n",	\
									Val.Type, Evaluation==NULL?"N/A":Evaluation); break;	\
		}
	
	// Expected results
	double R;
	BOOL B;
	char *P;
	
	P="unexpected";		// Testing valid computations
	R=0; B=!!!!FALSE; 					PE_TEST("FALSE not not not not");
	R=0; B=((B1 or B2) == (B1 and B2));	PE_TEST("B1 B2 or B1 B2 and eqv");
	R=(A1+10)/D1; B=0;					PE_TEST("A1 10 + D1 /");
	R=0; B=!(A1>5);						PE_TEST("A1 5 > not");
	R=0; B=C1>=exp(D1-A1);				PE_TEST("C1 e D1 A1 - ^ >=");
	R=sin(PI/4); B=0;					PE_TEST("C1 drop PI 4 / sin");
	R=1+A1+D1+2+3+4-50; R*=R; B=0; P="";PE_TEST("1 A1 D1 2 3 4 -50 sum 2 ^");
	R=0; B=0; P="";						PE_TEST("rand rand rand rand rand rand rand min");	// OK, won't be 0
	R=(B1?A1:C1); B=0; P="";			PE_TEST("B1 A1 C1 ?:");
	R=(A1+2+3+4+5)/5.; B=0; P="";		PE_TEST("A1 2 3 4 5 avg");
	R=0; B=B2; P="";					PE_TEST("D1 B2 switch drop");
	R=C2*SPEED_OF_LIGHT*SPEED_OF_LIGHT; B=0; P=""; PE_TEST("LightSpeed dup * C2 *");
	
	// Valid but beware !
	R=12.34; B=0; P="";					PE_TEST("12.34.56e78");	// Truncation
	R=Beh+1; B=0; P="";					PE_TEST("13 1 +");		// Careful with your variable names !
	
	R=B=0; 	// Testing for error conditions
	P="stack underflow";	PE_TEST("2 tan atan2");
	P="stack underflow";	PE_TEST("2 + 3");
	P="empty stack";		PE_TEST("2 3 + drop");
	P="unkown variable";	PE_TEST("A1 A2 + drop");
	P="remaining values";	PE_TEST("1 2 + 3 4 -");
	P="stack overflow";		PE_TEST("1 2 3 4 5 6 7 8 9 TRUE FALSE 1.2 -2.3 -1e5 A1 D1 C1 C2 10 11 PI E 13 14 15");
	P="unknown token";		PE_TEST("2 prout 4");
	P="math error";			PE_TEST("1 2 3 -1 sqrt");
	P="out of range";		PE_TEST("1.23e2000 sqrt");
	P="out of range";		PE_TEST("1.23e20 exp");
	P="wrong type";			PE_TEST("A1 C1 C2 == hypot");
	P="garbage syntax";		PE_TEST("A1,A2");
	P="extra invalid chars";PE_TEST("12,345");
	P="math overflow";		PE_TEST("9 8 7 6 5 4 3 2 ^ ^ ^ ^ ^ ^ ^");
	P="math overflow";		PE_TEST("1e300 dup * 1 switch /");
	
	return 0;
}
#endif
