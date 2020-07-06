/******************************************************************************
  MODULE: StrfTime.c
  
  PURPOSE: extension of the C strftime function with more options
			This functions behaves otherwise like the <time.h> strftime function

  Additional formatting codes (example given for Fri Dec 31 23:59:59 1999)
	%n	Decimal minutes,           59.983333
	%h	Decimal hour,              23.999722
	%D	Fraction of day,           0.999988
	%J	Decimal julian day,        364.999988
	%q	fractional year,           0.9999999682191		(warning, non linear)
	%Q	Decimal year,              1999.9999999682191	(warning, non linear)
	%o	Letter month, lowercase	   l
	%O	Letter month, uppercase	   L
	%1	hours since start of year  8759
	%2	hours since start of month 743
	%3	min since start of year    525599
	%4	min since start of month   44639
	%5	min since start of day     1439
	%6	sec since start of year    31535999
	%7	sec since start of month   2678399
	%8	sec since start of day     86399
	%9	sec since start of hour    3599
	%t	time_t internal (compiler specific, number of seconds since 1900 or 1970)  3155666399
  
  Normal formatting codes:
	%a	abbreviated weekday name	"Mon"...
	%A	full weekday name			"Monday"...
	%b	abbreviated month name		"Jan"...
	%B	full month name				"January"...
	%c	locale-specific date and time
	%d	day of the month as integer	01-31
	%H	hour (24-hour clock)		00-23
	%I	hour (12-hour clock)		01-12
	%j	day of the year as integer	001-366
	%m	month as integer			01-12
	%M	minute as integer			00-59
	%p	locale AM/PM designation	
	%S	second as integer			00-61 (allow for up to 2 leap seconds)
	%U	week number of the year		00-53 (week number 1 has 1st sunday)
	%w	weekday as integer			(0-6, sunday=0)
	%W	week number of the year		00-53 (week number 1 has 1st monday)
	%x	locale specific date
	%X	locale specific time
	%y	year without century		00-99
	%Y	year with century			1900...
	%z	time zone name
	%%	a single %%

  AUTHOR:  Guillaume Dargaud
  HISTORY: Jan 2001 - tested under Unix
******************************************************************************/
#include <iso646.h>	// for 'and', 'or'...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "StrfTime.h"

static char *StrBuf=NULL, *Before=NULL, *After=NULL, *Additional=NULL, *New=NULL;
static int LastMaxSize=0;		// Remember the size of the strings, so we don't reallocate all the time

#define Calloc(n,type) (type *)calloc(n, sizeof(type))
#define CONCAT	strncpy(Before, StrBuf, Pos); Before[Pos]='\0';\
				strcpy(After,  &StrBuf[Pos+LFC]); /*After[Pos]='0';*/\
				if (strlen(Before)+strlen(New)+strlen(After) > maxsize) return NULL;\
				sprintf(StrBuf, "%s%s%s", Before, New, After)



static int Pos, LFC=0;		// OK, that's dirty, so what ?
static int POS(const char* Str, const char* fc) {
	char *Sub=strstr(Str, fc), *EndPar;
	int P;
	
	if (Sub==NULL) return Pos=-1;		// Not found

	if (fc[1]=='(') {
		EndPar=strchr (Sub, ')');
		if (EndPar==NULL) return Pos=-1;	// Format string unterminated
		LFC=EndPar-Sub+2;				// LFC is number of char to remove from the string
	}
	else LFC=strlen(fc) ;
	
	if (Sub>Str and Sub[-1]=='%')	// we have double %% before the letter, ignore
		return Pos=(P=POS(Str+LFC, fc), P==-1 ? -1 : P+Sub-Str+1);	// look for other occurence
	else return Pos=Sub-Str;	// position of the substring
}


/******************************************************************************
  FUNCTION: StrfTime
  PURPOSE: format a single date/time into a string
  OUT: s: output string
  IN: maxsize: the maximum number of characters to place into s String, including the terminating ASCII NUL byte.
	  format: time format string
	  tm: time structure pointer
  RETURN: length of the string
******************************************************************************/
size_t CVIFUNC StrfTime(char *s, size_t maxsize, const char* format, const struct tm *timeptr) {
	size_t TempSize;
	
	if (maxsize>LastMaxSize and StrBuf!=NULL) {
		free(Before);
		free(After);
		free(StrBuf);
		free(Additional);
		free(New);
		Before=After=StrBuf=Additional=New=NULL;
	}
	
	if (StrBuf==NULL) {
		StrBuf=Calloc(maxsize, char); 	if (StrBuf==NULL) return NULL;
		Before=Calloc(maxsize, char);	if (Before==NULL) return NULL;
		After =Calloc(maxsize, char);	if (After==NULL) return NULL;
		Additional=Calloc(maxsize, char);	if (Additional==NULL) return NULL;
		New   =Calloc(maxsize, char);	if (New==NULL) return NULL;
		LastMaxSize=maxsize;
	}

	strcpy(StrBuf, format);

	// decimal minutes, 23h59m59s -> 59.98333333
	while (POS(StrBuf,"%n")>-1) {
		sprintf(New, "%f", timeptr->tm_min + timeptr->tm_sec/60. );
		CONCAT;
	}

	// decimal hour, 23h59m59s -> 23.99972222
	while (POS(StrBuf,"%h")>-1) {
		//sprintf(New, "%2d.%02d", timeptr->tm_hour, (int)(timeptr->tm_min*10./6.) );
		sprintf(New, "%f", timeptr->tm_hour + timeptr->tm_min/60. + timeptr->tm_sec/3600. );
		CONCAT;
	}
	
	// fraction of day, 23h59m59s -> 0.9999884259
	while (POS(StrBuf,"%D")>-1) {
		sprintf(New, "%f", (timeptr->tm_sec+60*(timeptr->tm_min+60.*timeptr->tm_hour))/86400. );
		CONCAT;
	}
	
	// decimal julian day, 99/12/31 23h59m59s -> 364.9999884259
	while (POS(StrBuf,"%J")>-1) {
		sprintf(New, "%f", timeptr->tm_yday + timeptr->tm_hour/24. + timeptr->tm_min/1440. + timeptr->tm_sec/86400. );
		CONCAT;
	}
	
	// fractional year, 99/12/31 23h59m59s -> 0.9999999682191
	while (POS(StrBuf,"%q")>-1) {
		sprintf(New, "%f", (timeptr->tm_yday + timeptr->tm_hour/24. + timeptr->tm_min/1440. + timeptr->tm_sec/86400.) / LengthOfYear(timeptr->tm_year+1900) );
		CONCAT;
	}
	
	// Decimal year, 99/12/31 23h59m59s -> 1999.9999999682191
	while (POS(StrBuf,"%Q")>-1) {
		sprintf(New, "%f", 1900 + timeptr->tm_year + (timeptr->tm_yday + timeptr->tm_hour/24. + timeptr->tm_min/1440. + timeptr->tm_sec/86400.) / LengthOfYear(timeptr->tm_year+1900) );
		CONCAT;
	}
	
	// Month as a lowercase letter (a to l)
	while (POS(StrBuf,"%o")>-1) {
		sprintf(New, "%c", timeptr->tm_mon+'a' );
		CONCAT;
	}
	
	// Month as an uppercase letter (A to L)
	while (POS(StrBuf,"%O")>-1) {
		sprintf(New, "%c", timeptr->tm_mon+'A' );
		CONCAT;
	}
	

	// hours since start of year, 99/12/31 23h59m59s -> 8759
	while (POS(StrBuf,"%1")>-1) {
		sprintf(New, "%d", timeptr->tm_hour + timeptr->tm_yday*24);
		CONCAT;
	}

	// hours since start of month, 99/12/31 23h59m59s -> 743
	while (POS(StrBuf,"%2")>-1) {
		sprintf(New, "%d", timeptr->tm_hour + (timeptr->tm_mday-1)*24);
		CONCAT;
	}


	// minutes since start of year, 99/12/31 23h59m59s -> 525599
	while (POS(StrBuf,"%3")>-1) {
		sprintf(New, "%d", timeptr->tm_min + 60*(timeptr->tm_hour + timeptr->tm_yday*24));
		CONCAT;
	}

	// minutes since start of month, 99/12/31 23h59m59s -> 44639
	while (POS(StrBuf,"%4")>-1) {
		sprintf(New, "%d", timeptr->tm_min + 60*(timeptr->tm_hour + (timeptr->tm_mday-1)*24));
		CONCAT;
	}
	
	// minutes since start of day, 99/12/31 23h59m59s -> 1439
	while (POS(StrBuf,"%5")>-1) {
		sprintf(New, "%d", timeptr->tm_min + 60*timeptr->tm_hour);
		CONCAT;
	}
	

	// Seconds since start of year, 99/12/31 23h59m59s -> 31535999
	while (POS(StrBuf,"%6")>-1) {
		sprintf(New, "%d", timeptr->tm_sec + 60*(timeptr->tm_min + 60*(timeptr->tm_hour + timeptr->tm_yday*24)));
		CONCAT;
	}

	// Seconds since start of month, 99/12/31 23h59m59s -> 2678399
	while (POS(StrBuf,"%7")>-1) {
		sprintf(New, "%d", timeptr->tm_sec + 60*(timeptr->tm_min + 60*(timeptr->tm_hour + (timeptr->tm_mday-1)*24)));
		CONCAT;
	}
	
	// Seconds since start of day, 99/12/31 23h59m59s -> 86399
	while (POS(StrBuf,"%8")>-1) {
		sprintf(New, "%d", timeptr->tm_sec + 60*(timeptr->tm_min + 60*timeptr->tm_hour));
		CONCAT;
	}
	
	// Seconds since start of hour, 99/12/31 23h59m59s -> 3599
	while (POS(StrBuf,"%9")>-1) {
		sprintf(New, "%d", timeptr->tm_sec + 60*timeptr->tm_min);
		CONCAT;
	}
	

	// time_t (seconds since 1900 or 1970, depending on compiler implementation)
	while (POS(StrBuf,"%t")>-1) {
		sprintf(New, "%u", mktime((struct tm*)timeptr));
		CONCAT;
	}
	

//	free(Before);
//	free(After);

	TempSize=strftime(s, maxsize, StrBuf, timeptr);	// take care of other format codes

//	free(StrBuf); 
	return TempSize;
} // end TimeFormat




// return the length of the month
int CVIFUNC DaysInMonth(const int YYYY, const int Month) {
	switch (Month) {
		case 1:return 31;
		case 2:return LengthOfYear(YYYY)==365 ? 28 : 29;
		case 3:return 31;
		case 4:return 30;
		case 5:return 31;
		case 6:return 30;
		case 7:return 31;
		case 8:return 31;
		case 9:return 30;
		case 10:return 31;
		case 11:return 30;
		case 12:return 31;
		default:return 0;
	}
}

// return the julian day (from 1 to 366)
int CVIFUNC JulianDay(const int YYYY, const int Month, int Day) {
	switch (Month) {
		case 12:Day+=30;
		case 11:Day+=31;
		case 10:Day+=30;
		case 9:Day+=31;
		case 8:Day+=31;
		case 7:Day+=30;
		case 6:Day+=31;
		case 5:Day+=30;
		case 4:Day+=31;
		case 3:Day+= (LengthOfYear(YYYY)==365 ? 28 : 29);
		case 2:Day+=31;
		case 1:return Day;
		default:return 0;
	}
}

// return the Month and day string from a Julian day
void CVIFUNC FromJulianDay(const int YYYY, const int Julian, int *Month, int *Day) {
	*Day=Julian; *Month=1;
	if (*Day<=31) return;	// Jan

	*Day-=31; (*Month)++;
	if (*Day<=(LengthOfYear(YYYY)==365 ? 28 : 29)) return;	// Feb
	
	*Day-=(LengthOfYear(YYYY)==365 ? 28 : 29); (*Month)++;
	if (*Day<=31) return;	// Mar
	
	*Day-=31; (*Month)++;
	if (*Day<=30) return;	// Apr
	
	*Day-=30; (*Month)++;
	if (*Day<=31) return;	// May
	
	*Day-=31; (*Month)++;
	if (*Day<=30) return;	// Jun
	
	*Day-=30; (*Month)++;
	if (*Day<=31) return;	// Jul
	
	*Day-=31; (*Month)++;
	if (*Day<=31) return;	// Aug
	
	*Day-=31; (*Month)++;
	if (*Day<=30) return;	// Sep
	
	*Day-=30; (*Month)++;
	if (*Day<=31) return;	// Oct
	
	*Day-=31; (*Month)++;
	if (*Day<=30) return;	// Nov
	
	*Day-=30; (*Month)++;
	if (*Day<=31) return;	// Dec
	
	*Day-=31; (*Month)++;		// Whatever
}


/******************************************************************************
  FUNCTION: StrfTimeDiff
  PURPOSE: Format the difference of two single dates/times into a string
			This kind of formatting is absent from standard ANSI C, so I
			just made up some formatting codes with 4 basic options on what
			the needs might be:
			- a normal number, IE 0..59 for minutes
			- a total number: 48 hours or 2 days
			- a zero filled number: 00:01:06 looks better than 0:1:6
			- optionals: "1 minute 10 seconds" instead of "0 days 0 hours 1
			minute 10 seconds"

  OUT: s: output string

  IN: maxsize: the maximum number of characters to place into s String, including the terminating ASCII NUL byte.
	  format: Difference time format string
	  			%D		Number of Days of difference 0...
	  			%()D	Optional number of Days of difference 1... (prints nothing if less than one days)
	  					In parenthesis, the text to be optionaly printed after
	  			
	  			%H		Total number of hours of difference 0...
	  			%h		Number of hours of difference  0..23
	  			%0h		Fills with 0 if less than 10: 00..23
	  			%()h	Optional number of Hours of difference 1..23 (prints nothing if less than one hour)
	  			%()H	Optional total number of Hours of difference 1..23 (prints nothing if less than one hour, or the number followed by the content of the parenthesis otherwise)
	  			
	  			%M		Total number of minutes of difference
	  			%m		Number of Minutes of difference 0..59
	  			%0m		Fills with 0 if less than 10:  00..59
	  			%()m	Optional number of Minutes of difference 1..59 (prints nothing if less than one minute)
	  			%()M	Optional number of Minutes of difference 1..59 (prints nothing if less than one minute, or the number followed by the content of the parenthesis otherwise)
	  			
	  			%S		Total number of seconds of difference
	  			%s		Number of Seconds of difference 0..59
	  			%0s		Fills with 0 if less than 10:  00..59
	  			%()s	Optional number of Seconds of difference 1..23 (prints nothing if less than one second)
	  			%()S	Optional Total number of Seconds of difference 1..23 (prints nothing if less than one second, or the number followed by the content of the parenthesis otherwise)

	  			%()+	Prints the content of the parenthesis if Time2 > Time1
	  			%()-	Prints the content of the parenthesis if Time2 < Time1
	  
	  Time1/2: time values to compare
  
  RETURN: length of the string

  EXAMPLE:	
	Dif\Fmt		"%(days )D%(hours )h%(min )m%ssec"	"%D days %0h:%0m:%0s"	"%Ss = %Mm %ss = %Hh %mm %ss = %Dd %hh %mm %ss"
	0			0sec								0 days 00:00:00			0s = 0m 0s = 0h 0m 0s = 0d 0h 0m 0s
	1000000		11days 13hours 46min 40sec			11 days 13:46:40		1000000s = 16666m 40s = 277h 46m 40s = 11d 13h 46m 40s
	1000		16min 40sec							0 days 00:16:40			1000s = 16m 40s = 0h 16m 40s = 0d 0h 16m 40s
	8			8sec								0 days 00:00:08			8s = 0m 8s = 0h 0m 8s = 0d 0h 0m 8s
******************************************************************************/
size_t CVIFUNC StrfTimeDiff(char *s, size_t maxsize, const char* format, 
			const time_t Time1, const time_t Time2) {
	unsigned long TSec=abs(difftime(Time1, Time2)),
		TMin=TSec/60,
		THour=TMin/60,
		TDay=THour/24,
		Hour=(TSec%(60*60*24))/(60*60),
		Min=  (TSec%(60*60))/60,
		Sec=   TSec%60;
	char* EndPar=NULL;

	if (maxsize>LastMaxSize and StrBuf!=NULL) {
		free(Before);
		free(After);
		free(StrBuf);
		free(Additional);
		free(New);
		Before=After=StrBuf=Additional=New=NULL;
	}
	
	if (StrBuf==NULL) {
		StrBuf=Calloc(maxsize, char); 		if (StrBuf==NULL) return NULL;
		Before=Calloc(maxsize, char);		if (Before==NULL) return NULL;
		After =Calloc(maxsize, char);		if (After==NULL) return NULL;
		Additional=Calloc(maxsize, char);	if (Additional==NULL) return NULL;
		New   =Calloc(maxsize, char);	if (New==NULL) return NULL;
		LastMaxSize=maxsize;
	}

	strcpy(StrBuf, format);
	
	// Total Days
	while (POS(StrBuf,"%D")>-1) {
		sprintf(New, "%d", TDay );
		CONCAT;
	}

	// Total Hours
	while (POS(StrBuf,"%H")>-1) {
		sprintf(New, "%d", THour );
		CONCAT;
	}

	// Total Minutes
	while (POS(StrBuf,"%M")>-1) {
		sprintf(New, "%d", TMin );
		CONCAT;
	}

	// Total Sec
	while (POS(StrBuf,"%S")>-1) {
		sprintf(New, "%u", TSec );
		CONCAT;
	}


	///////////////////////////////
	// Partial Hours
	while (POS(StrBuf,"%h")>-1) {
		sprintf(New, "%d", Hour );
		CONCAT;
	}

	// Partial Minutes
	while (POS(StrBuf,"%m")>-1) {
		sprintf(New, "%d", Min );
		CONCAT;
	}

	// Partial Sec
	while (POS(StrBuf,"%s")>-1) {
		sprintf(New, "%d", Sec );
		CONCAT;
	}


	///////////////////////////////
	// Filled partial Hours
	while (POS(StrBuf,"%0h")>-1) {
		sprintf(New, "%02d", Hour );
		CONCAT;
	}

	// Filled partial Minutes
	while (POS(StrBuf,"%0m")>-1) {
		sprintf(New, "%02d", Min );
		CONCAT;
	}

	// Filled partial Sec
	while (POS(StrBuf,"%0s")>-1) {
		sprintf(New, "%02d", Sec );
		CONCAT;
	}

	
	///////////////////////////////
	// Conditional partial Hours
	while (POS(StrBuf,"%(")>-1) {
		strncpy(Additional, &StrBuf[Pos+2], LFC-4);
		Additional[LFC-4]='\0';
		New[0]='\0';
		switch (StrBuf[Pos+LFC-1]) {			// Which format char ?
			case 'D':if (TDay >0) sprintf(New, "%d%s", TDay , Additional ); break;
			case 'H':if (THour>0) sprintf(New, "%d%s", THour, Additional ); break;
			case 'h':if (Hour >0) sprintf(New, "%d%s", Hour , Additional ); break;
			case 'M':if (TMin >0) sprintf(New, "%d%s", TMin , Additional ); break;
			case 'm':if ( Min >0) sprintf(New, "%d%s",  Min , Additional ); break;
			case 'S':if (TSec >0) sprintf(New, "%d%s", TSec , Additional ); break;
			case 's':if ( Sec >0) sprintf(New, "%d%s",  Sec , Additional ); break;
			case '+':if (Time2>Time1) sprintf(New, "%s",  Additional ); break;
			case '-':if (Time2<Time1) sprintf(New, "%s",  Additional ); break;
		}
		CONCAT;
	}


//	free(Before);
//	free(After);

//	TempSize=strftime(s, maxsize, StrBuf, timeptr);	// take care of other format codes
	strcpy(s, StrBuf);		// ??? No interim buffer
	
//	free(StrBuf); 
	return strlen(s);		
}


#if 0    /* formerly excluded lines */
// Testing StrfTimeDiff
int main (int argc, char *argv[]) {
	time_t Time=time(NULL), Time1=Time-1000000, Time2=Time+1000, Time3=Time-8;
	#define DIM 500
	char ST[DIM];
	int Len;
	
	#define Do(TIME) \
		Len=StrfTimeDiff(ST, DIM, "%(days )D%(hours )h%(min )m%ssec", Time, TIME); 				printf("\n%s\n", ST);\
		Len=StrfTimeDiff(ST, DIM, "%D days %0h:%0m:%0s", Time, TIME); 				 	 			printf("%s\n", ST);\
		Len=StrfTimeDiff(ST, DIM, "%Ss = %Mm %ss = %Hh %mm %ss = %Dd %hh %mm %ss", Time, TIME); 	printf("%s\n", ST);
	
	Do(Time);
	Do(Time1);
	Do(Time2);
	Do(Time3);
}
#endif   /* formerly excluded lines */


#if 0    /* formerly excluded lines */
// Testing purpose
int main (int argc, char *argv[]) {
	time_t Time=time(NULL);
	struct tm *TM=localtime(&Time);
	#define DIM 500
	char ST[DIM];
	int Len;

#if 0    /* formerly excluded lines */
		TM->tm_sec=59;
		TM->tm_min=59;
		TM->tm_hour=23;
		TM->tm_mday=31;
		TM->tm_mon=11;
		TM->tm_year=99;
		TM->tm_yday=364;
		mktime(TM);
#endif   /* formerly excluded lines */

	Len=StrfTime(ST, DIM, "%y%o%d %Hh%M.tfc", TM); 				 	 printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "Normal: %a %b %d %H:%M:%S %Y", TM);		 printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "Locale specific: %c", TM); 				 printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%n: Decimal minutes,           %n", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%h: Decimal hour,              %h", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%D: Fraction of day,           %D", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%J: Decimal julian day,        %J", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%q: Fractional year,           %q", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%Q: Decimal year,              %Q", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "Mixed: %y/%m/%d %H:%M:%S = %Q %J %h %n", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%o: Letter month,       %b=%m=%o", TM);  printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%O: Letter month,       %b=%m=%O", TM);  printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%1: hours since start of year  %1", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%2: hours since start of month %2", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%3: min since start of year    %3", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%4: min since start of month   %4", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%5: min since start of day     %5", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%6: sec since start of year    %6", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%7: sec since start of month   %7", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%8: sec since start of day     %8", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%9: sec since start of hour    %9", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "%%t: time_t internal            %t", TM); printf("%s\n", ST);
	Len=StrfTime(ST, DIM, "MT file name: %d%o%yh%H.%Ma", TM); 		 printf("%s\n", ST);
	return 0;
}
#endif   /* formerly excluded lines */



/*
Why there 24 hours in a day ?

Ah - this is complicated. It comes from ancient Egypt.
The Egyptian civil calender was divided into months consisting of three decades (ten days) each,
with 5 or so extra days tacked on at the end (New Year was fixed at the rising of Sirius, 
which signaled the inundation of the Nile - the most important event in the calender). 
This gives 36 decades in a year. The start of each decade during the year was marked by 
the rising of a particular star, and these stars were called decans. During any
particular night you would see at least twelve decans rise, at roughly equal intervals. 
So it was natural to divide the night into 12 time periods. 
By analogy the day was also divided into 12 periods (actually, 10 - one was set aside for 
the twilight at both the begining and the end of the day). 
Hence the 24 period day came about.
*/
