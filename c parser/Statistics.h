/******************************************************************************
  MODULE: Statistics
  
  PURPOSE: Module providing classes for statistics over single and double variable
		   Invalid results are returned with NAN value (defined in DEF.H)
	   Note that you can compile it for FLOAT or DOUBLE precision
******************************************************************************/
#ifndef STAT
#define STAT

#ifndef S_INNER
    #define S_INNER 0    // 0 for float, 1 for double precision during inner computations
#endif

#ifndef S_INOUT
    #define S_INOUT 0    // 0 for float, 1 for double precision - interface with calling program. Always S_INNER>=S_INOUT
#endif

#if S_INNER==0
    typedef float SInner;    // For inner precision computations.
#elif S_INNER==1 
    typedef double SInner;
#else
    #error Unknown type for internal computations (S_INNER macro)
#endif

#if S_INOUT==0
    typedef float SIO;    // For exchanging data with calling program.
#elif S_INOUT==1 
    typedef double SIO;
#else
    #error Unknown type for input/output (S_INOUT macro)
#endif

/******************************************************************************
  TYPE: CStat
  PURPOSE: Statistics for single variable: average, standard deviation, extrema...
******************************************************************************/
typedef struct CStat {
    unsigned long N;                       // Number of values
    SInner S,S2;                           // Sum of values and sum of squares
    SInner Min, Max;                       // Extrema
} CStat;

extern void StatReset(CStat*);				// Reset variables for new run
extern void StatAdd  (CStat*, const SIO X );// Add a value
extern void StatAddC (CStat*, const SIO X, const long Coef );  // Add a value with a ponderation coef
extern void StatSub  (CStat*, const SIO X );// Cancel a value, but not its effect on extrema
extern SIO StatAvr   (const CStat*);        // Average
extern SIO StatVariance(const CStat*);     	// Variance
extern SIO StatSigma (const CStat*);        // Standard deviation


/******************************************************************************
  TYPE: CStatXY
  PURPOSE: Statistics for dual variables: average, standard deviation, extrema, regression...
******************************************************************************/
typedef struct CStatXY {
    unsigned long N;                           // Number of values
    SInner Sx, Sy, Sxx, Syy, Sxy;              // Sum of values and sum of squares
    SInner MinX, MaxX, MinY, MaxY;             // Extrema                          
    SInner YatminX, YatmaxX, XatminY, XatmaxY; // Values corresponding to extrema
} CStatXY;

extern void StatXYReset(CStatXY*);                		// Reset variables for new run
extern void StatXYAdd  (CStatXY*, const SIO X, const SIO Y ); // Add a couple
extern void StatXYAddC (CStatXY*, const SIO X, const SIO Y, const long Coef );  // Add a couple with a ponderation coef
extern void StatXYSub  (CStatXY*, const SIO X, const SIO Y ); // Cancel a couple, but not its effect on extrema
extern SIO StatXYAvrX  (const CStatXY*);            	// Average of X values
extern SIO StatXYVarX  (const CStatXY*);            	// Variance of X value 
extern SIO StatXYSigmaX(const CStatXY*);          		// Standard deviation of X value
extern SIO StatXYAvrY  (const CStatXY*);            	// Average of Y values
extern SIO StatXYVarY  (const CStatXY*);            	// Variance of Y value 
extern SIO StatXYSigmaY(const CStatXY*);          		// Standard deviation of Y value
extern SIO StatXYAreg  (const CStatXY*);            	// Regression coef: origin
extern SIO StatXYBreg  (const CStatXY*);            	// Regression coef: slide  
extern SIO StatXYCreg  (const CStatXY*);            	// Correlation coef
extern SIO StatXYEvalY (const CStatXY*, const SIO X);	// Evaluate Y for a given X using linear regression
extern SIO StatXYEvalX (const CStatXY*, const SIO Y);	// Evaluate X for a given Y using linear regression

#endif                                           
