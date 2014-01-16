#include "LoggerAdapter_NSLog.h"

#include "Logger.h"
#import <Foundation/Foundation.h>
#include <string.h>

// These three functions provide a Logger compatibility interface to Logger.
// In all cases, priority, keyword, subkeyword are not useful to NSLog.

// WARNING: The number of bytes written return values are wrong because they don't include the NSLog stamp.
// Since NSLog stamps may contain appliation names and process id numbers (not necessarily with fixed-digits), 
// I don't know how many bytes there are. 
// So I will just return the string length.

int LoggerAdapter_CustomPrintfToNSLog(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...)
{
	va_list argp;
	va_start(argp, format);
	NSString* ns_format = [[NSString alloc] initWithUTF8String:format];
	// In order to get the number of bytes to use as a return value, I must create the full string because NSLog returns void.
	NSString* the_string = [[NSString alloc] initWithFormat:ns_format arguments:argp];
	// Since I already created the full string and used up the argp, I have to pass this.
	NSLog(@"%@", the_string);
	va_end(argp);
	return (int)[the_string length];
}

int LoggerAdapter_CustomPrintfvToNSLogv(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp)
{
	NSString* ns_format = [[NSString alloc] initWithUTF8String:format];
	// In order to get the number of bytes to use as a return value, I must create the full string because NSLog returns void.
	NSString* the_string = [[NSString alloc] initWithFormat:ns_format arguments:argp];
	// Since I already created the full string and used up the argp, I have to pass this.
	NSLog(@"%@", the_string);
	return (int)[the_string length];
}

int LoggerAdapter_CustomPutsToNSPuts(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text)
{
	// NSLog doesn't have a puts equivalent.
	NSLog(@"%s", text);
	return (int)strlen(text);
}

void Logger_Cocoa_SetCustomPrintFunctionToNSLog(Logger* logger)
{
	Logger_SetCustomPrintFunctions(
		logger,
		LoggerAdapter_CustomPutsToNSPuts,
		LoggerAdapter_CustomPrintfToNSLog,
		LoggerAdapter_CustomPrintfvToNSLogv,
		NULL
	);
}

void Logger_Cocoa_ClearCustomPrintFunctions(Logger* logger)
{
	Logger_SetCustomPrintFunctions(
		logger,
		NULL,
		NULL,
		NULL,
		NULL
	);
}

