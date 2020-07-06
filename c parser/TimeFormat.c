/*******************************************************************************
  PROGRAM: TimeFormat.exe
  PURPOSE: Takes a YYMMDD or YYMMDDHHMMSS (or intermediate) time and 
           converts it to whatever format is specified on the 2nd parameter
  COMMAND LINE: TimeFormat.exe [/4] YY[MM[DD[HH[MM[SS]]]]] FormatString
  AUTHOR:  Guillaume Dargaud
  HISTORY: Nov 2001 - First version
           Jun 2002 - Added YYYY years option
*******************************************************************************/

#define USE_EXTENDED_TIME_FORMAT	// This is an optional definition

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Def.h"

#ifdef USE_EXTENDED_TIME_FORMAT
#include "StrfTime.h"
#else
#define StrfTime strftime
#endif

#define _WEB_ "http://www.gdargaud.net/"
#define _VER_ "1.2"
static char *Info=			"   TimeFormat.exe NOW [FormatString]"
			"\nReturns the current time in YYMMDDHHMMSS format or using the format string."
			"\n"
			"\n   TimeFormat.exe YY[MM[DD[HH[MM[SS]]]]] [FormatString]"
			"\n   TimeFormat.exe /4 YYYY[MM[DD[HH[MM[SS]]]]] [FormatString]"
			"\nReturns the date or time formated accordingy to the FormatString."
			"\nNote that YY=01 corresponds to 2001 and YY=99 to 1999, Y2K compatible and works between 1970 and 2036."
			"\nExample of format strings: \"File%y%o%d_%Hh%M.tfc\""
			"\n"
			"\n   Normal C formatting codes:"
			"\n	%a	abbreviated weekday name	\"Mon\"..."
			"\n	%A	full weekday name		\"Monday\"..."
			"\n	%b	abbreviated month name		\"Jan\"..."
			"\n	%B	full month name			\"January\"..."
			"\n	%c	locale-specific date and time"
			"\n	%d	day of the month as integer	01-31"
			"\n	%H	hour (24-hour clock)		00-23"
			"\n	%I	hour (12-hour clock)		01-12"
			"\n	%j	day of the year as integer	001-366"
			"\n	%m	month as integer		01-12"
			"\n	%M	minute as integer		00-59"
			"\n	%p	locale AM/PM designation	"
			"\n	%S	second as integer		00-61 (allow for up to 2 leap seconds)"
			"\n	%U	week number of the year		00-53 (week number 1 has 1st sunday)"
			"\n	%w	weekday as integer		(0-6, sunday=0)"
			"\n	%W	week number of the year		00-53 (week number 1 has 1st monday)"
			"\n	%x	locale specific date"
			"\n	%X	locale specific time"
			"\n	%y	year without century		00-99"
			"\n	%Y	year with century		1900..."
			"\n	%z	time zone name"
			"\n	%%	a single %%"
#ifdef USE_EXTENDED_TIME_FORMAT
			"\n"
			"\n   Additional formatting codes (example given for Fri Dec 31 23:59:59 1999)"
			"\n	%n	Decimal minutes,           59.983333"
			"\n	%h	Decimal hour,              23.999722"
			"\n	%D	Fraction of day,           0.999988"
			"\n	%J	Decimal julian day,        364.999988"
			"\n	%q	fractional year,           0.9999999682191	(warning, non linear)"
			"\n	%Q	Decimal year,              1999.9999999682191	(warning, non linear)"
			"\n	%o	Letter month, lowercase	   l"
			"\n	%O	Letter month, uppercase	   L"
			"\n	%1	hours since start of year  8759"
			"\n	%2	hours since start of month 743"
			"\n	%3	min since start of year    525599"
			"\n	%4	min since start of month   44639"
			"\n	%5	min since start of day     1439"
			"\n	%6	sec since start of year    31535999"
			"\n	%7	sec since start of month   2678399"
			"\n	%8	sec since start of day     86399"
			"\n	%9	sec since start of hour    3599"
			"\n	%t	time_t internal (compiler specific, number of seconds since 1900 or 1970 or 1904...)  3155666399"
#endif
			"\n"
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
	int DI, J;
	BOOL Year4=FALSE;	// Years are input YYYY
	BOOL Now, DefaultFrmt, GoodDate;

	if (argc<2) { fprintf(stderr, "%s", Info); return 1; }
	Year4 = (strlen(argv[1])==2 and (argv[1][0]=='/' or argv[1][0]=='-') and argv[1][1]=='4');
	J = Year4 ? 2 : 0;		// number of chars to jump
	DI = Year4 ? 2 : 1;		// Position of the date string in the argv

	Now = (argc>DI) and strlen(argv[DI])==3 and 
				toupper(argv[DI][0])=='N' and toupper(argv[DI][1])=='O' and toupper(argv[DI][2])=='W';
	DefaultFrmt = (argc==DI+1);
	GoodDate = !Now and argc>DI and (
						(strlen(argv[DI])==2+J  and isdigit(argv[DI][0+J]) and isdigit(argv[DI][1+J])) or
						(strlen(argv[DI])==4+J  and isdigit(argv[DI][0+J]) and isdigit(argv[DI][1+J]) and 
												    isdigit(argv[DI][2+J]) and isdigit(argv[DI][3+J])) or
						(strlen(argv[DI])==6+J  and isdigit(argv[DI][0+J]) and isdigit(argv[DI][1+J]) and 
												    isdigit(argv[DI][2+J]) and isdigit(argv[DI][3+J]) and 
												    isdigit(argv[DI][4+J]) and isdigit(argv[DI][5+J])) or
						(strlen(argv[DI])==8+J  and isdigit(argv[DI][0+J]) and isdigit(argv[DI][1+J]) and 
												    isdigit(argv[DI][2+J]) and isdigit(argv[DI][3+J]) and 
												    isdigit(argv[DI][4+J]) and isdigit(argv[DI][5+J]) and 
												    isdigit(argv[DI][6+J]) and isdigit(argv[DI][7+J])) or
						(strlen(argv[DI])==10+J and isdigit(argv[DI][0+J]) and isdigit(argv[DI][1+J]) and 
												    isdigit(argv[DI][2+J]) and isdigit(argv[DI][3+J]) and 
												    isdigit(argv[DI][4+J]) and isdigit(argv[DI][5+J]) and 
												    isdigit(argv[DI][6+J]) and isdigit(argv[DI][7+J]) and 
												    isdigit(argv[DI][8+J]) and isdigit(argv[DI][9+J])) or
						(strlen(argv[DI])==12+J and isdigit(argv[DI][0+J]) and isdigit(argv[DI][1+J]) and 
												    isdigit(argv[DI][2+J]) and isdigit(argv[DI][3+J]) and 
												    isdigit(argv[DI][4+J]) and isdigit(argv[DI][5+J]) and 
												    isdigit(argv[DI][6+J]) and isdigit(argv[DI][7+J]) and 
												    isdigit(argv[DI][8+J]) and isdigit(argv[DI][9+J]) and 
												    isdigit(argv[DI][10+J])and isdigit(argv[DI][11+J]))
					);

	////////////////////// Reading Command line parameters ////////////////////// 
	if (!Now and !GoodDate) {
		fprintf(stderr, "%s", Info);
		return 1;
	}
	

	if (Now) {
		// TM already contains the local time
	}
	else {

		if (strlen(argv[DI])>=2+J) {
			Year[0]=argv[DI][0+J];
			Year[1]=argv[DI][1+J];
		if (strlen(argv[DI])>=4+J) {
			Month[0]=argv[DI][2+J];
			Month[1]=argv[DI][3+J];
		if (strlen(argv[DI])>=6+J) {
			Day[0]=argv[DI][4+J];
			Day[1]=argv[DI][5+J];
		if (strlen(argv[DI])>=8+J) {
			Hour[0]=argv[DI][6+J];
			Hour[1]=argv[DI][7+J];
		if (strlen(argv[DI])>=10+J) {
			Min[0]=argv[DI][8+J];
			Min[1]=argv[DI][9+J];
		if (strlen(argv[DI])>=12+J) {
			Sec[0]=argv[DI][10+J];
			Sec[1]=argv[DI][11+J];
		}}}}}}

		if (Year4) {
			TM->tm_year=atoi(Year) + (argv[DI][1]=='9' ? 0 : 100);	// 100 is year 2000
		} else { 
			TM->tm_year=atoi(Year);	if (TM->tm_year<70) TM->tm_year+=100;
		}
		TM->tm_mon=atoi(Month)-1;		// zero based
		TM->tm_mday=atoi(Day);
		TM->tm_yday=0;   // ?
		TM->tm_hour=atoi(Hour);
		TM->tm_min=atoi(Min);
		TM->tm_sec=atoi(Sec);
		TM->tm_isdst=0;				// Daylight saving crap will break when...?
	
		mktime(TM);
	}
	

	StrfTime(ST, DIM, DefaultFrmt ? (Year4 ? "%Y%m%d%H%M%S" : "%y%m%d%H%M%S") : argv[DI+1], TM);
	printf("%s", ST);
	return 0;
}


// test: 
// 010203040506 "Last time was %Y %B %D, %H:%M:%S"
// Now
// Now "File%y%o%d_%Hh%M.tfc"
