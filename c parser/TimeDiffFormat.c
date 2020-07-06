/*******************************************************************************
  PROGRAM: TimeDiffFormat.exe
  PURPOSE: Takes two YYMMDD or YYMMDDHHMMSS (or intermediate) time and 
           converts the difference to whatever format is specified on the 3nd parameter
  COMMAND LINE: TimeDiffFormat.exe YY[MM[DD[HH[MM[SS]]]]] YY[MM[DD[HH[MM[SS]]]]] FormatString
  AUTHOR:  Guillaume Dargaud
  HISTORY: Jan 2001 - First version
           June 2001 - Added NOW option
           Jun 2002 - Added YYYY years option
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Def.h"

#include "StrfTime.h"

#define _WEB_ "http://www.gdargaud.net/"
#define _VER_ "1.2"
static char *Info="   TimeDiffFormat.exe YY[MM[DD[HH[MM[SS]]]]] NOW [FormatString]"
			"\nReturns the difference between current time and YYMMDDHHMMSS using the format string."
			"\n   TimeFormat.exe YY[MM[DD[HH[MM[SS]]]]] YY[MM[DD[HH[MM[SS]]]]] FormatString"
			"\n   TimeFormat.exe /4 YYYY[MM[DD[HH[MM[SS]]]]] YYYY[MM[DD[HH[MM[SS]]]]] FormatString"
			"\nReturns the difference between 2 times formatted according to the FormatString."
			"\n"
			"\nNote that YY=01 corresponds to 2001 and YY=99 to 1999, Y2K compatible and works between 1970 and 2036."
			"\nExamples of format strings: \"%(-)-%(days )D%(hours )h%(min )m%ssec\", \"%(+)+%D days %0h:%0m:%0s\" or "
			"\"%Ss = %Mm %ss = %Hh %mm %ss = %Dd %hh %mm %ss\". If no format string is given, the default \"%(+)+%(-)-%D days %0h:%0m:%0s\" is used."
			"\n"
			"\n   Formatting codes:"
			"\n %D      Number of Days of difference 0..."
			"\n %()D    Optional number of Days of difference 1... (prints nothing if less than one days). In parenthesis, the text to be optionaly printed after"
			"\n %H      Total number of hours of difference 0..."
			"\n %h      Number of hours of difference  0..23"
			"\n %0h     Fills with 0 if less than 10: 00..23"
			"\n %()h    Optional number of Hours of difference 1..23 (prints nothing if less than one hour)"
			"\n %()H    Optional total number of Hours of difference 1..23 (prints nothing if less than one hour, or the number followed by the content of the parenthesis otherwise)"
			"\n %M      Total number of minutes of difference"
			"\n %m      Number of Minutes of difference 0..59"
			"\n %0m     Fills with 0 if less than 10:  00..59"
			"\n %()m    Optional number of Minutes of difference 1..59 (prints nothing if less than one minute)"
			"\n %()M    Optional number of Minutes of difference 1..59 (prints nothing if less than one minute, or the number followed by the content of the parenthesis otherwise)"
			"\n %S      Total number of seconds of difference"
			"\n %s      Number of Seconds of difference 0..59"
			"\n %0s     Fills with 0 if less than 10:  00..59"
			"\n %()s    Optional number of Seconds of difference 1..23 (prints nothing if less than one second)"
			"\n %()S    Optional Total number of Seconds of difference 1..23 (prints nothing if less than one second, or the number followed by the content of the parenthesis otherwise)"
	  		"\n %()+	Prints the content of the parenthesis if Time2 > Time1"
	  		"\n %()-	Prints the content of the parenthesis if Time2 < Time1"
			"\n"
			"\n   v" _VER_ " - (c) 2001-2002 Guillaume Dargaud - " _WEB_ "\n";


/******************************************************************************
  FUNCTION: main
******************************************************************************/
int main (int argc, char *argv[]) {
	time_t Time1=time(NULL), Time2=time(NULL);
	struct tm *TM=localtime(&Time1);
	#define DIM 500
	char ST[DIM], 
		Year[3]="0", Month[3]="1", Day[3]="1",
		Hour[3]="0", Min[3]="0", Sec[3]="0";
	int DI, D2, J;
	BOOL Year4=FALSE;	// Years are input YYYY
	BOOL Now1, Now2, DefaultFrmt, GoodDate;

	if (argc<2) { fprintf(stderr, "%s", Info); return 1; }
	
	Year4 = (strlen(argv[1])==2 and (argv[1][0]=='/' or argv[1][0]=='-') and argv[1][1]=='4');
	J = Year4 ? 2 : 0;		// number of chars to jump
	DI = Year4 ? 2 : 1;		// Position of the date string in the argv
	D2 = Year4 ? 3 : 2;		// Position of the 2nd date string in the argv

	Now1 = (argc>DI) and strlen(argv[DI])==3 and 
				toupper(argv[DI][0])=='N' and toupper(argv[DI][1])=='O' and toupper(argv[DI][2])=='W';

	Now2 = (argc>D2) and strlen(argv[D2])==3 and 
				toupper(argv[D2][0])=='N' and toupper(argv[D2][1])=='O' and toupper(argv[D2][2])=='W';

	DefaultFrmt = (argc==DI+2);

	GoodDate = !(Now1 or Now2) and (Year4 ? argc>=4 : argc>=3) and (
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
					) and (
						(strlen(argv[D2])==2+J  and isdigit(argv[D2][0+J]) and isdigit(argv[D2][1+J])) or
						(strlen(argv[D2])==4+J  and isdigit(argv[D2][0+J]) and isdigit(argv[D2][1+J]) and 
													isdigit(argv[D2][2+J]) and isdigit(argv[D2][3+J])) or
						(strlen(argv[D2])==6+J  and isdigit(argv[D2][0+J]) and isdigit(argv[D2][1+J]) and 
													isdigit(argv[D2][2+J]) and isdigit(argv[D2][3+J]) and 
													isdigit(argv[D2][4+J]) and isdigit(argv[D2][5+J])) or
						(strlen(argv[D2])==8+J  and isdigit(argv[D2][0+J]) and isdigit(argv[D2][1+J]) and 
													isdigit(argv[D2][2+J]) and isdigit(argv[D2][3+J]) and 
													isdigit(argv[D2][4+J]) and isdigit(argv[D2][5+J]) and 
													isdigit(argv[D2][6+J]) and isdigit(argv[D2][7+J])) or
						(strlen(argv[D2])==10+J and isdigit(argv[D2][0+J]) and isdigit(argv[D2][1+J]) and 
													isdigit(argv[D2][2+J]) and isdigit(argv[D2][3+J]) and 
													isdigit(argv[D2][4+J]) and isdigit(argv[D2][5+J]) and 
													isdigit(argv[D2][6+J]) and isdigit(argv[D2][7+J]) and 
													isdigit(argv[D2][8+J]) and isdigit(argv[D2][9+J])) or
						(strlen(argv[D2])==12+J and isdigit(argv[D2][0+J]) and isdigit(argv[D2][1+J]) and 
													isdigit(argv[D2][2+J]) and isdigit(argv[D2][3+J]) and 
													isdigit(argv[D2][4+J]) and isdigit(argv[D2][5+J]) and 
													isdigit(argv[D2][6+J]) and isdigit(argv[D2][7+J]) and 
													isdigit(argv[D2][8+J]) and isdigit(argv[D2][9+J]) and 
													isdigit(argv[D2][10+J])and isdigit(argv[D2][11+J]))
					);

	////////////////////// Reading Command line parameters ////////////////////// 
	if (!(Now1 or Now2) and !GoodDate) {
		fprintf(stderr, "%s", Info);
		return 1;
	}
	

	if (Now1) {
		TM=localtime(&Time1);
		// TM already contains the local time
	}
	else {
		if (strlen(  argv[DI])>=2+J) {
			Year[0]= argv[DI][0+J];
			Year[1]= argv[DI][1+J];
		if (strlen(  argv[DI])>=4+J) {
			Month[0]=argv[DI][2+J];
			Month[1]=argv[DI][3+J];
		if (strlen(  argv[DI])>=6+J) {
			Day[0]=  argv[DI][4+J];
			Day[1]=  argv[DI][5+J];
		if (strlen(  argv[DI])>=8+J) {
			Hour[0]= argv[DI][6+J];
			Hour[1]= argv[DI][7+J];
		if (strlen(  argv[DI])>=10+J) {
			Min[0]=  argv[DI][8+J];
			Min[1]=  argv[DI][9+J];
		if (strlen(  argv[DI])>=12+J) {
			Sec[0]=  argv[DI][10+J];
			Sec[1]=  argv[DI][11+J];
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
	
		Time1=mktime(TM);
	}
	

	if (Now2) {
		TM=localtime(&Time2);
		// TM already contains the local time
	}
	else {
		if (strlen(  argv[D2])>=2+J) {
			Year[0]= argv[D2][0+J];
			Year[1]= argv[D2][1+J];
		if (strlen(  argv[D2])>=4+J) {
			Month[0]=argv[D2][2+J];
			Month[1]=argv[D2][3+J];
		if (strlen(  argv[D2])>=6+J) {
			Day[0]=  argv[D2][4+J];
			Day[1]=  argv[D2][5+J];
		if (strlen(  argv[D2])>=8+J) {
			Hour[0]= argv[D2][6+J];
			Hour[1]= argv[D2][7+J];
		if (strlen(  argv[D2])>=10+J) {
			Min[0]=  argv[D2][8+J];
			Min[1]=  argv[D2][9+J];
		if (strlen(  argv[D2])>=12+J) {
			Sec[0]=  argv[D2][10+J];
			Sec[1]=  argv[D2][11+J];
		}}}}}}

		if (Year4) {
			TM->tm_year=atoi(Year) + (argv[D2][1]=='9' ? 0 : 100);	// 100 is year 2000
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
	
		Time2=mktime(TM);
	}
	

	StrfTimeDiff (ST, DIM, DefaultFrmt ? "%(+)+%(-)-%D days %0h:%0m:%0s" : argv[DI+2], Time1, Time2);
	// StrfTime(ST, DIM, DefaultFrmt ? "%y%m%d%H%M%S" : argv[2], TM);
	printf("%s", ST);
	return 0;
}


// test: 
// NOW 000101, "%(days )D%(hours )h%(min )m%ssec"
// 000101 NOW "%D days %0h:%0m:%0s"
// 00 010203040506 "%Ss = %Mm %ss = %Hh %mm %ss = %Dd %hh %mm %ss"
