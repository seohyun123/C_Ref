// SVN		$Id: SimpleLog.h 1897 2009-01-15 14:15:07Z /C=FR/O=CNRS/OU=UMR5821/CN=Guillaume Dargaud/emailAddress=dargaud@lpsc.in2p3.fr $

#ifndef __SIMPLE_LOG_H
#define __SIMPLE_LOG_H

// Various log levels
#define SL_ERROR	0x8
#define SL_WARNING	0x4
#define SL_NOTICE	0x2
#define SL_DEBUG	0x1

#define SL_ALL		(SL_ERROR|SL_WARNING|SL_NOTICE|SL_DEBUG)
#define SL_QUIET	0x0

extern void SimpleLog_Setup(const char *PathName, 
							const char *TimeFormat, 
							const int NoRepeatLastN, 
							const int RepeatMaxCount,
							const int RepeatMaxSeconds,
							const char *Separator);
extern int SimpleLog_FilterLevel(const int LogLevel);
extern void SimpleLog_Write(const int Level, const char *Origin, const char *fmt, ... );
extern void SimpleLog_Flush(void);

#endif

