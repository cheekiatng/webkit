/*
	Logger
	Copyright (C) 2002, 2010  Eric Wing <ewing . public @ playcontrol.net>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

*/


#include "Logger.h"
#include "LoggerPrimitives.h"
#include "TimeStamp.h"

#include <stdlib.h> /* For malloc */
#include <string.h>
#include <errno.h> /* Used for fopen error. */


/* Visual Studio doesn't define snprintf but _snprintf.
 * Putting this in implementation file instead of header to avoid accidental contamination of public space.
 */
#ifdef _MSC_VER
	#ifndef snprintf
		#define snprintf _snprintf
	#endif
#endif


void Logger_GetLinkedVersion(LoggerVersion* ver)
{
	/* Check the pointer */
	if(NULL == ver)
	{
		/* Do nothing */
		return;
	}
	ver->major = LOGGER_MAJOR_VERSION;
	ver->minor = LOGGER_MINOR_VERSION;
	ver->patch = LOGGER_PATCH_VERSION;
}

int Logger_IsCompiledWithLocking()
{
#ifdef LOGGER_ENABLE_LOCKING
	return 1;
#else
	return 0;
#endif
}


#if 0
/* Currently not used or implemented. See the warning on
 * the main page about the crappy internal error handling 
 * to understand why I haven't finished implementing this.
 */
extern LOGGER_EXPORT const char* LOGGER_CALLCONVENTION Logger_GetErrorString(int error_val)
{
	switch(error_val)
	{
		case 0:
			return "No Error";

			break;
		case 1:
			break;

		default:
			return "Unknown Error";
	}

	return NULL;
}
#endif


Logger* Logger_Create()
{
	Logger* logger;

	logger = LoggerPrimitives_MallocAndInit();
	if(NULL == logger)
	{
		return NULL;
	}

	return logger;
}

Logger* Logger_CreateWithName(const char* base_name)
{
	Logger* logger;
	char* copy_of_base_name;

	/* Make sure the file name is not NULL */
	if(NULL == base_name)
	{
		return NULL;
	}

	logger = LoggerPrimitives_MallocAndInit();
	if(NULL == logger)
	{
		return NULL;
	}

	/* LoggerPrimitives_OpenFile will open the file,
	 * and set the logger variables to 
	 * the correct values.
	 */
	if(!LoggerPrimitives_OpenFile(logger, base_name))
	{
		/* If the file didn't open, we 
		 * need to destroy the logger and 
		 * return NULL.
		 */
		Logger_Free(logger);
		return NULL;
	}

	/* We need to copy the base_name so that 
	 * the logger instance gets its own copy.
	 */
	/* First, allocate enough memory for the string */
	copy_of_base_name = (char*)calloc( (strlen(base_name) + 1), sizeof(char));
	if(NULL == copy_of_base_name)
	{
		/* Something went wrong. Clean up the logger and return NULL. */
		Logger_Free(logger);
		return NULL;
	}
	logger->baseNameMaxSize = strlen(base_name) + 1;
	/* Now copy the string */
	strncpy(copy_of_base_name, base_name, strlen(base_name) );

	/* Now set the logger->baseName to the string.
	 * Notice it is now connected to a const pointer so we shouldn't 
	 * alter the data any more.
	 */
	logger->baseName = copy_of_base_name;

	return logger;
}


Logger* Logger_CreateWithHandle(
	FILE* file_handle,
	const char* base_name)
{
	Logger* logger;
	int ret_flag;

	/* If the file_handle is stdout/stderr then I will
	 * allow the base_name to be NULL. Otherwise, they
	 * must specify a file.
	 */
	if( (stdout != file_handle)
		&& (stderr != file_handle)
	)
	{
		/* Make sure the file name is not NULL */
		if(NULL == base_name)
		{
			return NULL;
		}
	}
	if(NULL == file_handle)
	{
		return NULL;
	}

	logger = LoggerPrimitives_MallocAndInit();
	if(NULL == logger)
	{
		return NULL;
	}

	ret_flag = Logger_SetHandle(logger, file_handle, base_name);

	if(0 == ret_flag)
	{
		/* An error occurred. Need to free memory. */
		Logger_Free(logger);
		return NULL;
	}
	return logger;
}

int Logger_SetHandle(
	Logger* logger,
	FILE* file_handle,
	const char* base_name)
{
	char* copy_of_base_name;

	if(NULL == logger)
	{
		return 0;
	}

	LOGGERPRIMITIVES_LOCKMUTEX(logger);

	if(NULL == file_handle)
	{
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 0;
	}

	/* If the file_handle is stdout/stderr then I will
	 * allow the base_name to be NULL. Otherwise, they
	 * must specify a file.
	 */
	if( (stdout != file_handle)
		&& (stderr != file_handle)
	)
	{
		/* Make sure the file name is not NULL */
		if(NULL == base_name)
		{
			LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
			return 0;
		}
	}
	else
	{
		/* All we have to do is copy the file handle
		 * and we're done since we don't need to copy
		 * a string.
		 */
		logger->fileHandle = file_handle;
		if(NULL != logger->baseName)
		{
			free(logger->baseName);
			logger->baseName = NULL;
			logger->baseNameMaxSize = 0;
		}
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 1;
	}

	logger->fileHandle = file_handle;
	

	/* We need to copy the base_name so that 
	 * the logger instance gets its own copy.
	 */
	/* First, allocate enough memory for the string */
	copy_of_base_name = (char*)calloc( (strlen(base_name) + 1), sizeof(char));
	if(NULL == copy_of_base_name)
	{
		/* Something went wrong. */
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 0;
	}
	/* Now copy the string */
	strcpy(copy_of_base_name, base_name);

	/* I could be more efficient about memory allocations here,
	 * but I don't want to do it right now.
	 */
	if(NULL != logger->baseName)
	{
		free(logger->baseName);
		logger->baseName = NULL;
		logger->baseNameMaxSize = 0;
	}

	/* Now set the logger->baseName to the string.
	 */
	logger->baseName = copy_of_base_name;
	logger->baseNameMaxSize = strlen(base_name) + 1;

	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return 1;
}

void Logger_Free(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}

	/* I'm not sure if I should manually
	 * flush the file streams. Note that 
	 * CloseFile currently does not do anything 
	 * to stdout, so it might not get flushed 
	 * immediately
	 */
	/* Need to close any open file handles */
	LoggerPrimitives_CloseFile(logger);

#ifdef LOGGER_ENABLE_LOCKING
	if(NULL != ((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex)
	{
		LOGGERPRIMITIVES_DESTROYMUTEX( ((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex );
	}
#endif

	if(NULL != logger->opaqueLoggerData)
	{
		free(logger->opaqueLoggerData);
	}
	
	if(NULL != logger->baseName)
	{
		free(logger->baseName);
		logger->baseName = NULL;
	}
	if(NULL != logger->segmentFormatString)
	{
		free(logger->segmentFormatString);
		logger->segmentFormatString = NULL;
	}

	free(logger);
}


int Logger_LogEvent(Logger* logger, unsigned int priority, 
	const char* keyword, const char* subkeyword,
	const char* text, ...)
{
	int byte_counter;
	va_list argp;
	va_start(argp, text);
	byte_counter = Logger_LogEventv(logger, priority, keyword, subkeyword, text, argp);
	va_end(argp);
	return byte_counter;
}

/* 
 * va_list version of LogEvent.
 * va_list version of LogEvent. (Essentially, the va_start() and va_end
 * calls were removed.) Needed to rename from LogEvent to 
 * LogEventv because it was ambiguous which version to
 * call with one optional argument. The compiler
 * wasn't giving me an error. Insted, my program was
 * crashing calling this version instead of the other one.
 * I think the ambiguity was triggered by wrapping the
 * LogEvent call inside a macro.
 */
int Logger_LogEventv(Logger* logger, unsigned int priority, 
	const char* keyword, const char* subkeyword,
	const char* text, va_list argp) 
{
	char time_stamp[LOGGER_TIME_STAMP_SIZE];
	int byte_counter = 0;
	int ret_val;
	int error_flag = 0;
	va_list echo_copy;
	
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);

	if(!logger->loggingEnabled)
	{
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 0;
	}
	if(0 == priority)
	{
		priority = logger->defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < logger->thresholdPriority)
	{
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 0;
	}
	
	/* Get the current time */
	TimeStamp_GetTimeStamp(time_stamp, LOGGER_TIME_STAMP_SIZE);

	
	/* if segmentation is enabled */
	if(logger->minSegmentBytes != 0)
	{
		if(logger->currentByteCount > logger->minSegmentBytes)
		{
			/* SegmentFile() will just return if logger->fileHandle
			 * is not a file.
			 */
			if(!LoggerPrimitives_SegmentFile(logger))
			{
				/* Might still want echo
				 * Commented out for now
				 * logger->loggingEnabled = false;
				 * Pointer should be set to NULL
				 */
			}
			logger->currentByteCount = 0;
		}
	}
	
	/* Log stuff to fileHandle */
	if(logger->fileHandle != NULL)
	{
		int use_custom_print = LOGGER_USE_CUSTOM_PRINT(logger);
		if(0 == use_custom_print)
		{
			ret_val = LoggerPrimitives_PrintHeaderToFileHandle(logger->fileHandle,
				logger->preNewLines,
				priority, time_stamp, keyword, subkeyword);
		}
		else
		{
			ret_val = LoggerPrimitives_PrintHeaderWithCustom(logger,
				logger->preNewLines,
				priority, time_stamp, keyword, subkeyword);
		}

		if(ret_val >= 0)
		{
			byte_counter = ret_val;
		}
		/* Try to record/preserve the first error flag because subsequent 
		 * messages might be less helpful. */
		else if(0 == error_flag)
		{
			error_flag = ret_val;
		}
		
		if(text != NULL)
		{
			if(logger->echoOn && (NULL != logger->echoHandle))
			{
				LOGGER_VA_COPY(echo_copy, argp);
			}
			if(0 == use_custom_print)
			{
				ret_val = vfprintf(logger->fileHandle, text, argp);
			}
			else
			{
				ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPrintfv(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, text, argp);
			}
			if(ret_val >= 0)
			{
				byte_counter = byte_counter + ret_val;
			}
			else if(0 == error_flag)
			{
				/* save the error condition */
				error_flag = ret_val;
			}
		}
		
		if(0 == use_custom_print)
		{
			ret_val = LoggerPrimitives_PrintFooterToFileHandle(logger->fileHandle,
				logger->postNewLines,
				logger->autoFlushEnabled);
		}
		else
		{
			ret_val = LoggerPrimitives_PrintFooterWithCustom(logger,
				priority, keyword, subkeyword,
				logger->postNewLines,
				logger->autoFlushEnabled);
		}
		if(ret_val >= 0)
		{
			byte_counter = byte_counter + ret_val;
		}
		else if(0 == error_flag)
		{
			/* save the error condition */
			error_flag = ret_val;
		}	
		/* Update logger's byte count. */
		logger->currentByteCount += byte_counter;
	}
	/* Repeat again (to terminal) if echo is on. */
	if(logger->echoOn && (NULL != logger->echoHandle))
	{
		ret_val = LoggerPrimitives_PrintHeaderToFileHandle(logger->echoHandle,
			logger->preNewLines,
			priority, time_stamp, keyword, subkeyword);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

		if(text != NULL)
		{
			ret_val = vfprintf(logger->echoHandle, text, echo_copy);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}

			LOGGER_VA_COPYEND(echo_copy);
		}
		
		ret_val = LoggerPrimitives_PrintFooterToFileHandle(logger->echoHandle,
			logger->postNewLines,
			logger->autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}

	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);

	if(error_flag == 0)
	{
		return byte_counter;
	}
	else
	{
		return error_flag;
	}
}

/*
 * Turns out that I need a pass through LogEvent function
 * for security reasons for untrusted input (e.g. Lua scripts, main argv[]).
 * I discovered passing an invalid string or a string that 
 * was not supposed to be formatted could easily crash the printf
 * library. (This is technically not my problem.)
 * But to be safe, I'm creating a pass through function
 * that doesn't evaluate anything.
 * Currently, it assumes the time_stamp has been sanitized. fprintf is still
 * used within this function. However, keyword, subkeyword, and text are
 * handled by fputs.
 */
int Logger_LogEventNoFormat(Logger* logger, unsigned int priority, 
					 const char* keyword, const char* subkeyword,
					 const char* text)
{
	char time_stamp[LOGGER_TIME_STAMP_SIZE];
	int byte_counter = 0;
	int ret_val;
	int error_flag = 0;	

	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);

	if(!logger->loggingEnabled)
	{
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);				
		return 0;
	}
	if(0 == priority)
	{
		priority = logger->defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < logger->thresholdPriority)
	{
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 0;
	}
	
	/* Get the current time */
	TimeStamp_GetTimeStamp(time_stamp, LOGGER_TIME_STAMP_SIZE);
	
	
	/* if segmentation is enabled. */
	if(logger->minSegmentBytes != 0)
	{
		if(logger->currentByteCount > logger->minSegmentBytes)
		{
			/* SegmentFile() will just return if logger->fileHandle
			 * is not a file
			 */
			if(!LoggerPrimitives_SegmentFile(logger))
			{
				/* Might still want echo
				 * Commented out for now
				 * logger->loggingEnabled = false;
				 * Pointer should be set to NULL
				 */
			}
			logger->currentByteCount = 0;
		}
	}
	/* Log stuff to fileHandle */
	if(logger->fileHandle != NULL)
	{
		int use_custom_print = LOGGER_USE_CUSTOM_PRINT(logger);
		if(0 == use_custom_print)
		{
			ret_val = LoggerPrimitives_PrintHeaderToFileHandle(logger->fileHandle,
				logger->preNewLines,
				priority, time_stamp, keyword, subkeyword);
		}
		else
		{
			ret_val = LoggerPrimitives_PrintHeaderWithCustom(logger,
				logger->preNewLines,
				priority, time_stamp, keyword, subkeyword);
		}
		if(ret_val >= 0)
		{
			byte_counter = ret_val;
		}
		/* Try to record/preserve the first error flag because subsequent 
		 * messages might be less helpful. */
		else if(0 == error_flag)
		{
			error_flag = ret_val;
		}

		if(text != NULL)
		{
			if(0 == use_custom_print)
			{
				ret_val = fputs(text, logger->fileHandle);
			}
			else
			{
				ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, text);
			}
			if(ret_val >= 0)
			{
				byte_counter += strlen(text);	
			}
			else if(0 == error_flag)
			{
				/* save the error condition */
				error_flag = ret_val;
			}
		}
		
		if(0 == use_custom_print)
		{
			ret_val = LoggerPrimitives_PrintFooterToFileHandle(logger->fileHandle,
				logger->postNewLines,
				logger->autoFlushEnabled);
		}
		else
		{
			ret_val = LoggerPrimitives_PrintFooterWithCustom(logger,
				priority, keyword, subkeyword,
				logger->postNewLines,
				logger->autoFlushEnabled);
		}
		if(ret_val >= 0)
		{
			byte_counter += ret_val;	
		}
		else if(0 == error_flag)
		{
			/* save the error condition */
			error_flag = ret_val;
		}
	
		/* Update logger's byte count. */
		logger->currentByteCount += byte_counter;
	}
	/* Repeat again (to terminal) if echo is on. */
	if(logger->echoOn && (NULL != logger->echoHandle))
	{
		ret_val = LoggerPrimitives_PrintHeaderToFileHandle(logger->echoHandle,
			logger->preNewLines,
			priority, time_stamp, keyword, subkeyword);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

		if(text != NULL)
		{
			/* Not taking any chances with fprintf. */
			ret_val = fputs(text, logger->echoHandle);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}

		}
		
		ret_val = LoggerPrimitives_PrintFooterToFileHandle(logger->echoHandle,
			logger->postNewLines,
			logger->autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}	
		
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);

	if(error_flag == 0)
	{
		return byte_counter;
	}
	else
	{
		return error_flag;
	}
}

void Logger_Enable(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->loggingEnabled = 1;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

void Logger_Disable(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->loggingEnabled = 0;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

int Logger_IsEnabled(Logger* logger)
{
	int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->loggingEnabled;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

int Logger_Flush(Logger* logger)
{
	int ret_val;
	int error_flag = 0;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	if(logger->fileHandle != NULL)
	{
		error_flag = fflush(logger->fileHandle);
	}
	if(logger->echoOn && (NULL != logger->echoHandle))
	{
		if(logger->echoHandle != NULL)
		{
			ret_val = fflush(logger->echoHandle);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}
		}
	}
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);

	return error_flag;
}

void Logger_EnableAutoFlush(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->autoFlushEnabled = 1;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

void Logger_DisableAutoFlush(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->autoFlushEnabled = 0;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

int Logger_IsAutoFlushEnabled(Logger* logger)
{
	int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->autoFlushEnabled;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetThresholdPriority(Logger* logger, unsigned int thresh_val)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	if(0 == thresh_val)
	{
		logger->thresholdPriority = LOGGER_DEFAULT_THRESHOLD;
	}
	else
	{
		logger->thresholdPriority = thresh_val;
	}
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetThresholdPriority(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->thresholdPriority;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetDefaultPriority(Logger* logger, unsigned int pri_level)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	if(0 == pri_level)
	{
		logger->defaultPriority = LOGGER_DEFAULT_PRIORITY;
	}
	else
	{
		logger->defaultPriority = pri_level;
	}
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetDefaultPriority(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->defaultPriority;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetMinSegmentSize(Logger* logger, unsigned int num_bytes)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->minSegmentBytes = num_bytes;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetMinSegmentSize(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->minSegmentBytes;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_EchoOn(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->echoOn = 1;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

void Logger_EchoOff(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->echoOn = 0;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

int Logger_IsEchoOn(Logger* logger)
{
	int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->echoOn;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetEchoToStdout(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->echoHandle = stdout;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

void Logger_SetEchoToStderr(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->echoHandle = stderr;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

void Logger_SetPreLines(Logger* logger, unsigned int pre_lines)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->preNewLines = pre_lines;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

void Logger_SetPostLines(Logger* logger, unsigned int post_lines)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->postNewLines = post_lines;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

void Logger_SetNewLines(Logger* logger, unsigned int prelines, unsigned int postlines)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	logger->preNewLines = prelines;
	logger->postNewLines = postlines;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetPreLines(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->preNewLines;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

unsigned int Logger_GetPostLines(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	ret_val = logger->postNewLines;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return ret_val;
}

int Logger_WillLog(Logger* logger, unsigned int priority)
{
	if(NULL == logger)
	{
		return 0;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	if(!logger->loggingEnabled)
	{
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 0;
	}
	if(0 == priority)
	{
		priority = logger->defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < logger->thresholdPriority)
	{
		LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
		return 0;
	}
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
	return 1;
}

void Logger_SetSegmentFormatString(Logger* logger, const char* format_string)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	if(NULL == format_string)
	{
		/* Reset the system to use the default format string. */
		format_string = LOGGER_DEFAULT_SEGMENT_FORMAT_STRING;
	}

	logger->segmentFormatString = LoggerPrimitives_CopyDynamicString(
		logger->segmentFormatString, 
		&logger->segmentFormatStringMaxSize, 
		format_string
	);
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}



/* All 3 must be defined or all must be NULL. Only overriding some is not supported. */
void Logger_SetCustomPrintFunctions(Logger* logger,
	int (*custom_puts)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text),
	int (*custom_printf)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...),
    int (*custom_printfv)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp),
	void* custom_callback_user_data
)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGERPRIMITIVES_LOCKMUTEX(logger);
	((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts = custom_puts;
	((LoggerOpaqueData*)logger->opaqueLoggerData)->customPrintf = custom_printf;
	((LoggerOpaqueData*)logger->opaqueLoggerData)->customPrintfv = custom_printfv;
	((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData = custom_callback_user_data;
	LOGGERPRIMITIVES_UNLOCKMUTEX(logger);
}


