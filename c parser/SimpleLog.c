///////////////////////////////////////////////////////////////////////////////
// MODULE	SimpleLog
// AUTHOR	Guillaume Dargaud
// PURPOSE	Very simple logging facility. Requirements:
//			Standard C, compatible Linux/Windows
//			Uses sprintf syntax for write messages
//			Optional time tagging
//			Works in append mode:
//				- files can be manipulated externally
//				- no need to catch HUP
//				- reliable, if the program crashes, everything is already written 
//				  (except possible repeated messages)
//				- Not highly fast
//			Use log level filters
//			Can write to stderr or files
//			Doesn't repeat last N identical messages
//			Doesn't repeat them unless an optional count or a optional duration has been reached
//
//			Message have the following form with [brakets] being optional
//			[TimeStamp] Severity [Origin] [repeat count] Message
// SVN		$Id: SimpleLog.c 1924 2009-02-18 12:17:54Z /C=FR/O=CNRS/OU=UMR5821/CN=Guillaume Dargaud/emailAddress=dargaud@lpsc.in2p3.fr $
///////////////////////////////////////////////////////////////////////////////

#include <iso646.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "SimpleLog.h"

// Those are the tags that are written with the message depending on the severity
#define SL_MSG_ERROR	"ERROR"	// ERR, alternatively
#define SL_MSG_WARNING	"WARNG"	// WRN
#define SL_MSG_NOTICE	"NTICE"	// NTE
#define SL_MSG_DEBUG	"DEBUG"	// DBG

// Retain setup variables
static char *SL_PathName=NULL;		// defaults to stderr
static char *SL_TimeFormat=NULL;	// Not used by default
static int SL_NoRepeatLastN=0;		// Write all messages by default
static int SL_RepeatMaxCount=1;		// Write the messages after this many times. 0 to disable, 1 to print all
static int SL_RepeatMaxSeconds=1;	// Write the messages after this many seconds. 0 to keep them as long as possible
static int SL_FilterLevel=SL_ALL;	// Write all messages by default
static char SL_Separator[10]=" ";	// Space by default

// Retain repeated messages
typedef struct sMessage {
	time_t TimeStampFirst, TimeStampLast;
	int Severity;	// Of the 1st message, we don't care if this changes
	char *Origin;	// Of the 1st message, we don't care if this changes
	char *Message;
	int Count;		// Number of times this message has been logged
} tMessage;
tMessage *Messages=NULL;

	

///////////////////////////////////////////////////////////////////////////////
// FUNCTION	SimpleLog_Setup
/// HIFN	Set the name of the log file and other parameters. Should be called only once.
/// HIFN	The file is only opened in append mode when we write to it, and then closed.
/// HIFN	This allows an external app to reset it or move it at will.
/// HIFN	Call to this function is optional same as (NULL, NULL, 0, 1, 0, " ")
/// HIPAR	PathName/Pathname of the file to write to. Pass NULL to use stderr.
/// HIPAR	TimeFormat/Optional strftime format string. Pass NULL to disable. 
/// HIPAR	TimeFormat/Suggested "%Y%m%d-%H%M%S" or "%Y/%m/%d-%H:%M:%S" (note the space at the end)
/// HIPAR	NoRepeatLastN/Will avoid repeating the last N messages if they are identical
/// HIPAR	NoRepeatLastN/You can call this function multiple times except to change this parameter
/// HIPAR	RepeatMaxCount/Display the message after it's been sent so many times
/// HIPAR	RepeatMaxCount/0 to keep them forever in memory, 1 to print them all
/// HIPAR	RepeatMaxSeconds/0 to keep them forever in memory, or number of seconds before a flush
/// HIPAR	Separator/Character(s) used as separators between fields
// EXAMPLES	(NULL, NULL, 0, 1, 0, " ") print all messages to stderr, no time stamp
//			("Msg.log", "%H:%M ", 1, 0, 0, ", ") Avoid repeating the last message
//			("Msg.log", "%Y%m%d-%H%M%S ", 10, 20, 0, "\t") Among the last 10 different messages, avoid repeating them at most 20 times
//			(NULL, "%Y%m%d-%H%M%S ", 100, 0, 60, " ") Among the last 100 different messages, avoid repeating them but print them at least once per minute
///////////////////////////////////////////////////////////////////////////////
void SimpleLog_Setup(const char *PathName, 
		const char *TimeFormat, 
		const int NoRepeatLastN, 
		const int RepeatMaxCount,
		const int RepeatMaxSeconds,
		const char *Separator) {
	int i;
	if (PathName!=NULL and PathName[0]!='\0') {
		if (SL_PathName!=NULL) free(SL_PathName);
		SL_PathName=malloc(strlen(PathName)+1);
		strcpy(SL_PathName, PathName);
	}

	if (TimeFormat!=NULL and TimeFormat[0]!='\0') {
		if (SL_TimeFormat!=NULL) free(SL_TimeFormat);
		SL_TimeFormat=malloc(strlen(TimeFormat)+1);
		strcpy(SL_TimeFormat, TimeFormat);
	}
	
	if (Separator!=NULL) {
		strncpy(SL_Separator, Separator, 9);
		SL_Separator[9]='\0';
	}
	
	SL_RepeatMaxCount=RepeatMaxCount;	
	SL_RepeatMaxSeconds=RepeatMaxSeconds;	

	if (Messages==NULL and NoRepeatLastN>0) {	// Can be allocated only once
		SL_NoRepeatLastN=NoRepeatLastN;	
		Messages=calloc(SL_NoRepeatLastN, sizeof(tMessage));
		for (i=0; i<SL_NoRepeatLastN; i++) {
			Messages[i].TimeStampFirst=
			Messages[i].TimeStampLast=
			Messages[i].Severity=
			Messages[i].Count=0;
			Messages[i].Origin=
			Messages[i].Message=NULL;
		}			
	}
	
	// Log itself !
	SimpleLog_Write(SL_NOTICE, "SimpleLog", "Start of logging facility with%s timestamp, "
		"avoid repeat of last %d identical messages among the last %d, "
		"with a flush at least every %d seconds.",
		SL_TimeFormat==NULL?"out":"",
		SL_RepeatMaxCount, SL_NoRepeatLastN, SL_RepeatMaxSeconds);
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION	SimpleLog_FilterLevel
/// HIFN	Set the current filter level
/// HIPAR	FilterLevel/Combination of the various SL_ defines. Messages that don't match will be ignored
/// HIRET	Returns the previous level (so you can do temporary swaps)
// EXAMPLE	(SL_WARNING|SL_NOTICE) from now on will only display warnings and notices (not a good idea to leave SL_ERROR out)
///////////////////////////////////////////////////////////////////////////////
int SimpleLog_FilterLevel(const int FilterLevel) {
	int Previous=SL_FilterLevel;
	SL_FilterLevel=FilterLevel;
	return Previous;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION	DisplayMessage
/// HIFN	Display a message (and its count if more than 1)
/// HIFN	If the file proves non-writable, stderr is used as a fallback
/// HIPAR	Level/Log level to use. If it doesn't match the filter, it's discarded
/// HIPAR	Origin/Optional part of the message that doesn't need to be different 
/// HIPAR	Origin/Suggested value is __func__ to pass the origin function name
/// HIPAR	Message/Message to display
/// HIPAR	Count/If more than 1, assumes teh message has already been printed once before
// NOTE		When printing a repeated message, 
//			the time is of the last print 
//			while the level and origin correspond to the first instance of this message
///////////////////////////////////////////////////////////////////////////////
static void DisplayMessage(time_t MsgTime, const int Level, const char *Origin, const char *Message, const int Count) {
	static int FirstError=1;
	FILE *File=stderr;			// This is the default destination
	
	if (Message==NULL) return;	// But if it's empty we display it
	
	if (SL_PathName!=NULL and SL_PathName[0]!='\0') 
		if ((File=fopen(SL_PathName, "a"))==NULL) {
			File=stderr;
			if (FirstError) {	// Notify this error only once
				FirstError=0;
				fprintf(stderr, "\nSimpleLog Error %d (%s) when trying to append to file %s", 
					errno, strerror(errno), SL_PathName);
			}
		}
  
	// Newline
	if (File==stderr or fprintf(File, "\n")<0)
		fprintf(stderr, "\n");	// Fallback if File fails

	// Optional time string
	if (SL_TimeFormat!=NULL and SL_TimeFormat[0]!='\0') {
		char TimeStr[255];
		if (0<strftime(TimeStr, 255, SL_TimeFormat, localtime(&MsgTime)))
			if (File==stderr or fprintf(File, "%s", TimeStr)<0)
				fprintf(stderr, "%s", TimeStr);	// Fallback if File fails
		// Separator
		if (File==stderr or fprintf(File, "%s", SL_Separator)<0)
			fprintf(stderr, "%s", SL_Separator);	// Fallback if File fails
	}
	
	// Severity message. If severity mixes the levels, we print only the most important one
	#define SEVERITY (	\
				Level & SL_ERROR   ? SL_MSG_ERROR :	\
				Level & SL_WARNING ? SL_MSG_WARNING :	\
				Level & SL_NOTICE  ? SL_MSG_NOTICE :	\
				Level & SL_DEBUG   ? SL_MSG_DEBUG :	\
				"   ")	// Invalid level
		
	if (File==stderr or fprintf(File, "%s%s", SEVERITY, SL_Separator)<0)
		fprintf(stderr, "%s%s", SEVERITY, SL_Separator);	// Fallback if File fails

	// Optional origin message
	if (Origin!=NULL and Origin[0]!='\0')
		if (File==stderr or fprintf(File, "%s%s", Origin, SL_Separator)<0)
			fprintf(stderr, "%s%s", Origin, SL_Separator);	// Fallback if File fails

	// Optional repeat count portion
	if (Count>1)
		if (File==stderr or fprintf(File, "[*%d]%s", Count, SL_Separator)<0)
			fprintf(stderr, "[*%d]%s", Count, SL_Separator);	// Fallback if File fails

	// Write message itself
	if (File==stderr or fprintf(File, "%s", Message)<0)
		fprintf(stderr, "%s", Message);	// Fallback if File fails

	if (File!=stderr) fclose(File);
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION	FlushMessage
/// HIFN	Display the message and its count. Remove it from the list
/// HIPAR	Index/Position in the list
///////////////////////////////////////////////////////////////////////////////
static void FlushMessage(const int Index) {
	if (Messages==NULL or 						// Not in use
		Messages[Index].Message==NULL) return;	// Error
	
	if (Messages[Index].Count>1)				// The 1st one has already been printed
		DisplayMessage(	Messages[Index].TimeStampLast, 
						Messages[Index].Severity, 
						Messages[Index].Origin, 
						Messages[Index].Message, 
						Messages[Index].Count);
	
	// Clear it off the list
	Messages[Index].TimeStampFirst=
	Messages[Index].TimeStampLast=
	Messages[Index].Severity=
	Messages[Index].Count=0;
	if (Messages[Index].Origin !=NULL) free(Messages[Index].Origin ); Messages[Index].Origin=NULL;
	if (Messages[Index].Message!=NULL) free(Messages[Index].Message); Messages[Index].Message=NULL;
	
}
	
///////////////////////////////////////////////////////////////////////////////
// FUNCTION	FindMessage
/// HIFN	Find the message if it's already in the list. Update the list
/// HIPAR	Message/Message to identify and update
/// HIRET	The index in the Messages[] array or -1 if not found
///////////////////////////////////////////////////////////////////////////////
static int FindMessage(char *Message) {
	int i;
	if (Messages==NULL or Message==NULL) return -1;
	
	for (i=0; i<SL_NoRepeatLastN; i++)
		if (Messages[i].Message!=NULL and 
			strcmp(Message, Messages[i].Message)==0) {
			Messages[i].TimeStampLast=time(NULL);	// Now
			if (( ++Messages[i].Count>=SL_RepeatMaxCount and 
				SL_RepeatMaxCount>0 ) or
				( SL_RepeatMaxSeconds>0 and 
				difftime(Messages[i].TimeStampLast, Messages[i].TimeStampFirst)>=SL_RepeatMaxSeconds )) {
				// We went above the count or time limit
				DisplayMessage(	Messages[i].TimeStampLast, 
								Messages[i].Severity, 
								Messages[i].Origin, 
								Messages[i].Message, 
								Messages[i].Count);
				Messages[i].Count=1;	// The current one has just been displayed
				Messages[i].TimeStampFirst=Messages[i].TimeStampLast;	// Now
			}
			return i;
		}
	return -1;	// Not found
}
	
///////////////////////////////////////////////////////////////////////////////
// FUNCTION	AddMessageToList
/// HIFN	Add the message to an empty spot, or flush out the oldest one
/// HIPAR	Message/Message to add
/// HIRET	The index of the added message in the Messages[] array, -1 in case of error
///////////////////////////////////////////////////////////////////////////////
static int AddMessageToList(const int Level, const char *Origin, const char *Message) {
	int i=-1, j;
	time_t Oldest, Now=time(NULL);

	if (Message==NULL) return -1;
	if (Messages==NULL) goto Display;
	
	for (i=0; i<SL_NoRepeatLastN; i++)
		if (Messages[i].Message==NULL) 
			break;	// [i] is available

	if (i==SL_NoRepeatLastN) {					// All spots are taken
		Oldest=Now;
		i=0;
		for (j=0; j<SL_NoRepeatLastN; j++)		// Find the oldest
			if (Messages[j].TimeStampLast<Oldest) { 
				Oldest=Messages[j].TimeStampLast;
				i=j;
			}
		FlushMessage(i);	// Display and clear the oldest. [i] is now available
	}

	Messages[i].Message=malloc(strlen(Message)+1);
	strcpy(Messages[i].Message, Message);

	if (Origin!=NULL) {
		Messages[i].Origin=malloc(strlen(Origin)+1);
		strcpy(Messages[i].Origin, Origin);
	}
	
	Messages[i].TimeStampFirst=Messages[i].TimeStampLast=Now;
	Messages[i].Severity=Level;
	Messages[i].Count=1;
	
Display:
	DisplayMessage(Now, Level, Origin, Message, 1);	// Print the first one
	return i;	
}
	
///////////////////////////////////////////////////////////////////////////////
// FUNCTION	SimpleLog_Write
/// HIFN	Log a message at a given level
/// HIFN	If the file proves non-writable, stderr is used as a fallback
/// HIPAR	Level/Log level to use. If it doesn't match the filter, it's discarded
/// HIPAR	Origin/Optional part of the message that doesn't need to be different 
/// HIPAR	Origin/Suggested value is __func__ to pass the origin function name
/// HIPAR	fmt/Usual printf syntax and extra parameters
// EXAMPLE	(SL_DEBUG, __func__, "Current number %d", Nb)
///////////////////////////////////////////////////////////////////////////////
void SimpleLog_Write(const int Level, const char *Origin, const char *fmt, ... ) {
	int Nb;
	va_list str_args;
	char Message[1024]="\0";
	
	if (!(Level & SL_FilterLevel)) return;	// Nothing to write
		
	va_start( str_args, fmt );
	#ifdef HAS_VSNPRINTF
		// Unfortunately this isn't supported in CVI
		Nb=vsnprintf(Message, 1022, fmt, str_args);
	#else
	{	// To avoid a CRASH if the message is too big or an errant buffer is passed
		int Nb1;
		static FILE *File=NULL;
		#ifdef _NI_linux_
		if (File==NULL) File=fopen("/dev/null", "w");	// Faster than tmp file
		#endif
		if (File==NULL) File=tmpfile();
		
		Nb1=vfprintf(File, fmt, str_args);
		rewind(File);	// We keep it open but always rewrite from the begining so as to avoid having the buffer (ever?) written
		if (0<Nb1 and Nb1<1023) {
			va_end( str_args );			// Is this really necessary ?
			va_start( str_args, fmt );
			Nb=vsprintf(Message, fmt, str_args);
			if (Nb!=Nb1) // Note: there may be an admissible difference due to a "\n" in the message and tmpfile being binary
				strcpy(Message, "/// SimpleLog message discrepancy ///");		// Breakpoint here
		} else {
			if (Nb1<=0) 
				 strcpy(Message, "/// Invalid SimpleLog message ///");		// Breakpoint here
			else strcpy(Message, "/// SimpleLog message is too large ///");	// Breakpoint here
			Nb=strlen(Message);	// Just so we skip the next test
		}
	}
	#endif
	va_end( str_args );

	if (Nb<=0 or Message[0]=='\0' or strlen(Message)>=1023)	// Some margin
		strcpy(Message, "Invalid SimpleLog message");		// Breakpoint here

	// If the message is found, update its counter and timing
	if (FindMessage(Message)<0)
		// Otherwise add it (or replace older one) and display it 
		AddMessageToList(Level, Origin, Message);	
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION	SimpleLog_Flush
/// HIFN	If repeated messages are in memory, write them
/// HIFN	Call this function before exiting or before you want to rotate the log.
///////////////////////////////////////////////////////////////////////////////
void SimpleLog_Flush(void) {
	int i, j;
	time_t Oldest;
	
	do {
		Oldest=time(NULL);
		i=-1;
		for (j=0; j<SL_NoRepeatLastN; j++)		// Find the oldest and print it first
			if (Messages[j].TimeStampLast!=0 and Messages[j].TimeStampLast<=Oldest) { 
				Oldest=Messages[j].TimeStampLast;
				i=j;
			}
		if (i==-1) return;	// List is now empty
		FlushMessage(i);	// Display and clear the oldest. [i] is now available
	} while (1);
}
