/*******************************************************************************
  PROGRAM: TimeJump.exe
  PURPOSE: Takes a YYMMDD or YYMMDDHHMMSS (or intermediate) time and 
           returns the previous/next value
  COMMAND LINE: TimeJump.exe [/4] YY[MM[DD[HH[MM[SS]]]]] [[-|+]Number]
  AUTHOR:  Guillaume Dargaud
  HISTORY: Jan 2002 - First version
           Jun 2002 - Added Y2K 4 digit years
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Def.h"
//#include "StrfTime.h"

#define _WEB_ "http://www.gdargaud.net/"
#define _VER_ "1.2"
static char *Info="TimeJump.exe YY[MM[DD[HH[MM[SS]]]]] [[-|+]Number]"
			"\nTimeJump.exe /4 YYYY[MM[DD[HH[MM[SS]]]]] [[-|+]Number]"
			"\nTimeJump.exe [/4] NOW [[-|+]Number]"
			"\nReturns the previous/next date or time."
			"\nOption: [[-]Number] to return the previous time/date by Number units. "
			"\nOption: [/4] to use 4 digits years, both in input and output. "
			"\nIf YY is given then unit is year, if YYMM then it's in months, etc..."
			"\nIf NOW is given then default unit is day (YYMMDD or YYYYMMDD)."
			"\nExample: \"TimeJump.exe 0001 -2\" returns 9911"
			"\nExample: \"TimeJump.exe 9912312359 +70\" returns 000101000109"
			"\nExample: \"TimeJump.exe /4 199912312359 +70\" returns 20000101000109"
			"\n   v" _VER_ " - (c) 2001-2002 Guillaume Dargaud - " _WEB_ "\n";


/******************************************************************************
  FUNCTION: main
******************************************************************************/
int main (int argc, char *argv[]) {
	time_t Time=time(NULL);
	struct tm *TM=localtime(&Time);
	#define DIM 500
	char ST[DIM], 
		Year[3]="0", Month[3]="1", Day[3]="1", 
		Hour[3]="0", Min[3]="0", Sec[3]="0";
	int U, Depth, DI, J;
	BOOL Year4=FALSE;	// Years are input YYYY
	
	
	////////////////////// Reading Command line parameters ////////////////////// 
	if (argc<2) { fprintf(stderr, "%s", Info); return 1; }
	Year4 = (strlen(argv[1])==2 and (argv[1][0]=='/' or argv[1][0]=='-') and argv[1][1]=='4');
	J = Year4 ? 2 : 0;		// number of chars to jump
	DI = Year4 ? 2 : 1;		// Position of the date string in the argv
	if (!(argc==DI+1 or argc==DI+2) or 
		!(strlen(argv[DI])==2 or strlen(argv[DI])==3 or strlen(argv[DI])==4 or strlen(argv[DI])==6
		 or strlen(argv[DI])==8 or strlen(argv[DI])==10 or strlen(argv[DI])==12)) {
		fprintf(stderr, "%s", Info);
		return 1;
	}
	

	if (argc==DI+2)		// Optional jump argument
		U=atoi(argv[DI+1]);
	else U=+1;

	if (strlen(argv[DI])==3 and 
		toupper(argv[DI][0])=='N' and toupper(argv[DI][0])=='N' and toupper(argv[DI][0])=='N') {
		Depth=3;		// Just keep the current date
	} else if (strlen(argv[DI])==3) {
		fprintf(stderr, "%s", Info); return 1;
	} else {
		Depth=(strlen(argv[DI])-J)/2;	// 1 for YY, 2 for YYMM...
		switch (Depth) {
			case 6:	Sec  [0]=argv[DI][10+J];Sec  [1]=argv[DI][11+J];
			case 5:	Min  [0]=argv[DI][8+J];	Min  [1]=argv[DI][9+J];
			case 4:	Hour [0]=argv[DI][6+J];	Hour [1]=argv[DI][7+J];
			case 3:	Day  [0]=argv[DI][4+J];	Day  [1]=argv[DI][5+J];
			case 2:	Month[0]=argv[DI][2+J];	Month[1]=argv[DI][3+J];
			case 1:	Year [0]=argv[DI][0+J];	Year [1]=argv[DI][1+J];
		}

		if (Year4) {
			TM->tm_year=atoi(Year) + (argv[DI][1]=='9' ? 0 : 100);	// 100 is year 2000
		} else { 
			TM->tm_year=atoi(Year);	
			if (TM->tm_year<70) TM->tm_year+=100+(Depth==1?U:0); 
		}
		TM->tm_mon=atoi(Month)-1+(Depth==2?U:0);		// zero based
		TM->tm_mday=atoi(Day)+(Depth==3?U:0);
		TM->tm_yday=0;   // ?
		TM->tm_hour=atoi(Hour)+(Depth==4?U:0);
		TM->tm_min=atoi(Min)+(Depth==5?U:0);
		TM->tm_sec=atoi(Sec)+(Depth==6?U:0);
		TM->tm_isdst=0;
	}
	
	mktime(TM);

	if (Year4) switch (Depth) {
		case 1:	strftime(ST, 20, "%Y",           TM); break;
		case 2:	strftime(ST, 20, "%Y%m",         TM); break;
		case 3:	strftime(ST, 20, "%Y%m%d",       TM); break;
		case 4:	strftime(ST, 20, "%Y%m%d%H",     TM); break;
		case 5:	strftime(ST, 20, "%Y%m%d%H%M",   TM); break;
		case 6:	strftime(ST, 20, "%Y%m%d%H%M%S", TM); break;
	} else  switch (Depth) {
		case 1:	strftime(ST, 20, "%y",           TM); break;
		case 2:	strftime(ST, 20, "%y%m",         TM); break;
		case 3:	strftime(ST, 20, "%y%m%d",       TM); break;
		case 4:	strftime(ST, 20, "%y%m%d%H",     TM); break;
		case 5:	strftime(ST, 20, "%y%m%d%H%M",   TM); break;
		case 6:	strftime(ST, 20, "%y%m%d%H%M%S", TM); break;
	} 
	
	printf("%s", ST);
	return 0;
}
