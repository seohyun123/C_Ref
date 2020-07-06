/*******************************************************************************
  PROGRAM: LengthOfYearMonth.exe
  PURPOSE: If given YY, returns the number of days in the year: 365 ot 366
  		   If given YYMM, returns the number of days in the month: 28 to 31
  COMMAND LINE: LengthOfYearMonth.exe [/4 YY]YY[MM]
  AUTHOR:  Guillaume Dargaud
  HISTORY: Nov 2001 - First version
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Def.h"
#include "StrfTime.h"


/******************************************************************************
  FUNCTION: main
******************************************************************************/
int main (int argc, char *argv[]) {
	char Year[5]="00", Month[3]="01";
	int NYear;
	BOOL Year2=(argc==2 and ( strlen(argv[1])==2 or strlen(argv[1])==4 ));
	BOOL Year4=(argc==3 and ( 
		strlen(argv[1])==2 and (argv[1][0]=='/' or argv[1][0]=='-') and argv[1][1]=='4' and	// /4 or -4
		(strlen(argv[2])==4 or strlen(argv[2])==6)));
	BOOL DoMonth=(argc==2 and strlen(argv[1])==4) or (argc==3 and strlen(argv[2])==6);

	////////////////////// Reading Command line parameters ////////////////////// 
	if (!Year2 and !Year4) {
		fprintf(stderr, "%s", 
			"   LengthOfYearMonth.exe [/4 YY]YY[MM]"
			"\nIf given [/4 YY]YY, returns the number of days in the year: 365 or 366."
			"\nIf given [/4 YY]YYMM, returns the number of days in the month: 28 to 31."
			"\nUse /4 to input 4 digit years"
			"\nMM is 01 to 12\n"
		);
		return 1;
	}
	
	if (Year2) { 
		Year[0]=argv[1][0]; 
		Year[1]=argv[1][1];
		Year[2]='\0';
		NYear=atoi(Year)<70 ? atoi(Year)+2000 : atoi(Year)+1900;
	} else {
		strncpy(Year,argv[2],4);
		Year[4]='\0';
		NYear=atoi(Year);
	}
	
	if (!DoMonth)
		printf("%d", LengthOfYear (NYear));
	else {
		Month[0]=argv[Year4?2:1][Year4?4:2];
		Month[1]=argv[Year4?2:1][Year4?5:3];
		printf("%d", DaysInMonth (NYear, atoi(Month)));
	} 
	// fprintf(stderr, "Syntax Error"); 

	return 0;
}


// test: 
// LengthOfYearMonth 97
// LengthOfYearMonth 0002
