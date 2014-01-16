#include "LogWrapper.h"

// Yeah, I know this is bad. http://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
//#include <sys/syslimits.h> // PATH_MAX Apple
#include <limits.h> // PATH_MAX Android
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "Logger.h"

#if defined(__APPLE__)
	#include "LoggerAdapter_NSLog.h"
#elif defined(__ANDROID__)
	#include "LoggerAdapter__android_log.h"
#endif


LogWrapper* LogWrapper_Create()
{
	LogWrapper* log_wrapper;
	log_wrapper = (LogWrapper*)calloc(1, sizeof(LogWrapper));
	{
/*
		NSArray* directory_paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString* documents_directory = [directory_paths firstObject];
		NSString* test_file = [documents_directory stringByAppendingPathComponent:@"test262_runlog.txt"];

		loggerFile = Logger_CreateWithName([test_file fileSystemRepresentation]);
*/
		log_wrapper->loggerFile = NULL;
		log_wrapper->loggerNative = Logger_Create();
		log_wrapper->loggerSocket = NULL;
		log_wrapper->logFilePathAndName = (char*)calloc(PATH_MAX+1, sizeof(char));
//		loggerSocket = Logger_Create();
//		Logger_Disable(loggerSocket);

		// These are backdoor extensions I added to Logger awhile back to allow overriding the built in print functions.
		// Use these to override fprintf/fputs with NSLog.
		/*
		loggerNative->customPrintf = LogWrapper_CustomPrintfToNSLog;
		loggerNative->customPrintfv = LogWrapper_CustomPrintfToNSLogv;
		loggerNative->customPuts = LogWrapper_CustomPrintfToNSPuts;
		*/
#if defined(__APPLE__)
		Logger_Cocoa_SetCustomPrintFunctionToNSLog(log_wrapper->loggerNative);
#elif defined(__ANDROID__)
		Logger_Android_SetCustomPrintFunctionToAndroidLog(log_wrapper->loggerNative);
#else
#endif

		// Since NSLog always prints a timestamp, printing newline padding between log messages just makes more clutter, so disable newline injection.
		Logger_SetNewLines(log_wrapper->loggerNative, 0, 0);

		// Thanks to Unix, where everything is a file, I'll just convert a socket into a FILE* with fdopen.
//		int fd = socket(AF_INET, SOCK_STREAM, 0);
//		FILE *fp = fdopen(fd, "a");

	}
	return log_wrapper;
}

void LogWrapper_Free(LogWrapper* log_wrapper)
{
	Logger_Free(log_wrapper->loggerFile);
	Logger_Free(log_wrapper->loggerNative);
	Logger_Free(log_wrapper->loggerSocket);
	free(log_wrapper->logFilePathAndName);
	free(log_wrapper);
}

void LogWrapper_Flush(LogWrapper* log_wrapper)
{
	Logger_Flush(log_wrapper->loggerFile);
	Logger_Flush(log_wrapper->loggerNative);
	Logger_Flush(log_wrapper->loggerSocket);
}

_Bool LogWrapper_OpenNewFileWithName(LogWrapper* log_wrapper, const char* full_path_and_file)
{
	// close the existing one if it exists
	if(NULL != log_wrapper->loggerFile)
	{
		Logger_Free(log_wrapper->loggerFile);
		log_wrapper->loggerFile = NULL;
	}
	log_wrapper->loggerFile = Logger_CreateWithName(full_path_and_file);
	if(NULL != log_wrapper->loggerFile)
	{
		strncpy(log_wrapper->logFilePathAndName, full_path_and_file, PATH_MAX);
		// remember that the buffer size is PATH_MAX+1, so PATH_MAX is the last character which we want to terminate.
		log_wrapper->logFilePathAndName[PATH_MAX] = '\0';
		return true;
	}
	else
	{
		log_wrapper->logFilePathAndName[0] = '\0';
		return false;
	}
}

void LogWrapper_CloseFile(LogWrapper* log_wrapper)
{
	Logger_Free(log_wrapper->loggerFile);
	log_wrapper->loggerFile = NULL;
	log_wrapper->logFilePathAndName[0] = '\0';
}

const char* LogWrapper_GetLogFilePathAndName(LogWrapper* log_wrapper)
{
	return log_wrapper->logFilePathAndName;
}
