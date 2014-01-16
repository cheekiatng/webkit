#include "LoggerAdapter__android_log.h"

#include "Logger.h"
#include <android/log.h>


// You may want to change the following function to specify how your Logger priorities map to Android priorities.
static int LoggerAdapter_ConvertLoggerPriorityToAndroidPriority(unsigned int logger_priority)
{
	int android_priority_value;
	switch(logger_priority)
	{
		case 0:
		{
			android_priority_value = ANDROID_LOG_DEFAULT;
			break;
		}
		case 1:
		{
			android_priority_value = ANDROID_LOG_VERBOSE;
			break;
		}
		case 2:
		{
			android_priority_value = ANDROID_LOG_DEBUG;
			break;
		}
		case 3:
		{
			android_priority_value = ANDROID_LOG_INFO;
			break;
		}
		case 4:
		{
			android_priority_value = ANDROID_LOG_WARN;
			break;
		}
		case 5:
		{
			android_priority_value = ANDROID_LOG_ERROR;
			break;
		}
		case 6:
		{
			android_priority_value = ANDROID_LOG_FATAL;
			break;
		}
		default:
		{
			android_priority_value = ANDROID_LOG_SILENT;			
		}
	}
	return android_priority_value;
}


// These three functions provide a Logger compatibility interface to Logger.

int LoggerAdapter_CustomPrintfTo__android_log_print(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...)
{
	int android_priority_value = LoggerAdapter_ConvertLoggerPriorityToAndroidPriority(priority);
	int num_bytes_written;
	va_list argp;
	va_start(argp, format);
	/* Android doesn't document what the return value is.
	 * But it looks like it does what I want and counts all the bytes including their own stamps.
	 */
	num_bytes_written = __android_log_vprint(android_priority_value, keyword, format, argp);

	va_end(argp);
	return num_bytes_written;
}

int LoggerAdapter_CustomPrintfvTo__android_log_vprint(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp)
{
	/* Android doesn't document what the return value is.
	 * But it looks like it does what I want and counts all the bytes including their own stamps.
	 */
	int android_priority_value = LoggerAdapter_ConvertLoggerPriorityToAndroidPriority(priority);
	return __android_log_vprint(android_priority_value, keyword, format, argp);
}

int LoggerAdapter_CustomPutsTo__android_log_write(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text)
{
	int android_priority_value = LoggerAdapter_ConvertLoggerPriorityToAndroidPriority(priority);
	/* Android doesn't document what the return value is.
	 * But it looks like it does what I want and counts all the bytes including their own stamps.
	 */
	int num_bytes_written = __android_log_write(android_priority_value, keyword, text);
	return num_bytes_written;
}

void Logger_Android_SetCustomPrintFunctionToAndroidLog(Logger* logger)
{
	Logger_SetCustomPrintFunctions(
		logger,
		LoggerAdapter_CustomPutsTo__android_log_write,
		LoggerAdapter_CustomPrintfTo__android_log_print,
		LoggerAdapter_CustomPrintfvTo__android_log_vprint,
		NULL
	);
}

void Logger_Android_ClearCustomPrintFunctions(Logger* logger)
{
	Logger_SetCustomPrintFunctions(
		logger,
		NULL,
		NULL,
		NULL,
		NULL
	);
}

