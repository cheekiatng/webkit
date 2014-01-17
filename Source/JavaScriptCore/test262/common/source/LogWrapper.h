//
//  LogWrapper.h
//
//  Created by Eric Wing on 12/30/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#ifndef LOGWRAPPER_H
#define LOGWRAPPER_H

#include "Logger.h"

#define LOGWRAPPER_PRIORITY_CRITICAL 5
#define LOGWRAPPER_PRIORITY_STANDARD 3
#define LOGWRAPPER_PRIORITY_DEBUG 1

#define LOGWRAPPER_PRIMARY_KEYWORD "Test262"
typedef struct LogWrapper LogWrapper;

struct LogWrapper
{
	Logger* loggerFile;
	Logger* loggerNative;
	Logger* loggerSocket;
	char* logFilePathAndName;
};



LogWrapper* LogWrapper_Create(void);
void LogWrapper_Free(LogWrapper* log_wrapper);
void LogWrapper_Flush(LogWrapper* log_wrapper);
_Bool LogWrapper_OpenNewFileWithName(LogWrapper* log_wrapper, const char* full_path_and_file);
void LogWrapper_CloseFile(LogWrapper* log_wrapper);
// This returns a pointer to LogWrapper's internal copy of the string. Make your own copy if you need to keep it around or modify.
const char* LogWrapper_GetLogFilePathAndName(LogWrapper* log_wrapper);


// I use variadic macros here because I want to pass the same string message to multiple functions.
// Without macros, this gets into temporary string creation or nasty platform specific/undefined behavior of va_copy.
#define LogWrapper_LogEvent(log_wrapper, priority, keyword, subkeyword, ...) \
	do \
	{ \
		Logger_LogEvent(log_wrapper->loggerFile, priority, keyword, subkeyword, __VA_ARGS__); \
		Logger_LogEvent(log_wrapper->loggerNative, priority, keyword, subkeyword, __VA_ARGS__); \
		int macro_loggger_retval = Logger_LogEvent(log_wrapper->loggerSocket, priority, keyword, subkeyword, __VA_ARGS__); \
		if(macro_loggger_retval < 0) \
		{ \
			fprintf(stderr, "Error writing to loggerSocket"); \
			Logger_Disable(log_wrapper->loggerSocket); \
		} \
	} \
	while (0)


#if defined(__APPLE__)
#import "LoggerObjC.h"

// I use variadic macros here because I want to pass the same string message to multiple functions.
// Without macros, this gets into temporary string creation or nasty platform specific/undefined behavior of va_copy.
#define LogWrapperObjC_LogEvent(log_wrapper, priority, keyword, subkeyword, ...) \
	do \
	{ \
		LoggerObjC_LogEvent(log_wrapper->loggerFile, priority, keyword, subkeyword, __VA_ARGS__); \
		LoggerObjC_LogEvent(log_wrapper->loggerNative, priority, keyword, subkeyword, __VA_ARGS__); \
		int macro_loggger_retval = LoggerObjC_LogEvent(log_wrapper->loggerSocket, priority, keyword, subkeyword, __VA_ARGS__); \
		if(macro_loggger_retval < 0) \
		{ \
			NSLog(@"Error writing to loggerSocket"); \
			Logger_Disable(log_wrapper->loggerSocket); \
		} \
	} \
	while (0)
#endif

#endif /* LOGWRAPPER_H */


