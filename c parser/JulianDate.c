/*******************************************************************************
  PROGRAM: JulianDate.exe
  PURPOSE: Takes a julian date YYDDD (001<=DDD<=366) and converts it to YYMMDD.
  		   Takes a julian date YYYYDDD (001<=DDD<=366) and converts it to YYYYMMDD.
  		   Takes a YYMMDD date (01<=MM<=12, 01<=DD<=31) and converts it to julian date YYDDD 
  		   Takes a YYYYMMDD date (01<=MM<=12, 01<=DD<=31) and converts it to julian date YYYYDDD 
  COMMAND LINE: JulianDate.exe [YY]YYMMDD|YYDDD
  AUTHOR:  Guillaume Dargaud
  HISTORY: Nov 2001 - First version
           Jun 2002 - Added 4 digit years
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Def.h"
#include "StrfTime.h"

#define _WEB_ "http://www.gdargaud.net/"
#define _VER_ "1.3"
static char *Info="   JulianDate.exe [YY]YYJJJ | [YY]YYMMDD"
			"\nConverts a date between Julian format [YY]YYJJJ and more usual [YY]YYMMDD format."
			"\nJJJ is 001 to 366, MM is 01 to 12, DD is 00 to 31 and year is either 2 or 4 digits. Examples:"
			"\n  00366  ->   001231"
			"\n2000366  -> 20001231"
			"\n  001231 ->   00366"
			"\n20001231 -> 2000366"
			"\n"
			"\n   v" _VER_ " - (c) 2001-2005 Guillaume Dargaud - " _WEB_ "\n";

/******************************************************************************
  FUNCTION: main
******************************************************************************/
int main (int argc, char *argv[]) {
	time_t Time=time(NULL);
	struct tm *TM=localtime(&Time);
	#define DIM 500
	char ST[DIM], 
		Year[5]="0000", Month[3]="01", Day[3]="01", JJJ[4]="001";
	int NYear, NMonth, NDay, J;
	BOOL Year4=FALSE;	// Years are input YYYY
	BOOL ToJD  =argc==2 and (strlen(argv[1])==6 or strlen(argv[1])==8);	// Convert To JD
	BOOL FromJD=argc==2 and (strlen(argv[1])==5 or strlen(argv[1])==7);	// Convert To JD


	////////////////////// Reading Command line parameters ////////////////////// 
	if (argc!=2 or 		// Two or wrong arguments -> help
		!(ToJD or FromJD) or
		strspn(argv[1], "0123456789")<strlen(argv[1]) ) {
		fprintf(stderr, "%s", Info);
		return 1;
	}

	Year4 = (strlen(argv[1])==7 or strlen(argv[1])==8);	// 4 digit year
	J = Year4 ? 2 : 0;		// number of chars to jump
	
	Year[2]=argv[1][0+J];
	Year[3]=argv[1][1+J];
	if (Year4) {
		Year[0]=argv[1][0];
		Year[1]=argv[1][1];
		NYear=atoi(Year);
	} else {
		NYear = atoi(Year)<70 ? atoi(Year)+2000 : atoi(Year)+1900;
	}
	
	if (strlen(argv[1])==6+J) {
		Month[0]=argv[1][2+J];
		Month[1]=argv[1][3+J];
		Day[0]=argv[1][4+J];
		Day[1]=argv[1][5+J];
		NMonth=atoi(Month);
		NDay=atoi(Day);
	} else if (strlen(argv[1])==5+J) {
		JJJ[0]=argv[1][2+J];
		JJJ[1]=argv[1][3+J];
		JJJ[2]=argv[1][4+J];
		FromJulianDay(NYear, atoi(JJJ), &NMonth, &NDay);
	}


	TM->tm_year=NYear-1900;		// 1900 based
	TM->tm_mon=NMonth-1;		// zero based
	TM->tm_mday=NDay;
	TM->tm_yday=0;   			// ?
	TM->tm_hour=0;
	TM->tm_min=0;
	TM->tm_sec=0;
	TM->tm_isdst=0;				// Daylight saving crap will break when...?
	
	mktime(TM);
	
	strftime(ST, DIM,  ToJD ? (Year4 ? "%Y%j" : "%y%j") : (Year4 ? "%Y%m%d" : "%y%m%d"), TM);
	printf("%s", ST);
	return 0;
}


// test: 
// JulianDate 00001
// JulianDate 000101
