///////////////////////// StrfTime.h //////////////////////////////////////////
#ifndef _STRFTIME
#define _STRFTIME

#include <time.h>

#ifndef CVIFUNC
#define CVIFUNC
#endif

// Takes into account 1900 not being leap year and 2000 being leap year
#define LengthOfYear(YYYY) (((YYYY)%400==0 or ((YYYY)%100!=0 and (YYYY)%4==0)) ? 366 : 365)

extern int CVIFUNC DaysInMonth(const int YYYY, const int Month);

extern int CVIFUNC JulianDay(const int Year, const int Month, int Day);
extern void CVIFUNC FromJulianDay(const int YYYY, const int Julian, int *Month, int *Day);

extern size_t CVIFUNC StrfTime(char *s, size_t maxsize, const char* format, const struct tm *timeptr);

extern size_t CVIFUNC StrfTimeDiff(char *s, size_t maxsize, const char* format, const time_t Time1, const time_t Time2);

#endif // STRFTIME
