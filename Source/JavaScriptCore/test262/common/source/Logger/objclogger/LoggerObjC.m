
#ifdef __OBJC__

#include "LoggerObjC.h"
#import <Foundation/Foundation.h>


/* This is a hack.
 The problem is that the Obj-C implementation requires on private static functions in the Logger.c file.
 I could re-do them to make them private API but potentially visible, but this is faster/easier.
 Make sure to only compile this file and not Logger.c if you use this file.
 */
#include "../clogger/Logger.c"


int LoggerObjC_LogEvent(Logger* logger, unsigned int priority, 
	const char* keyword, const char* subkeyword,
	NSString* text, ...)
{
	int byte_counter;
	va_list argp;
	va_start(argp, text);
	byte_counter = LoggerObjC_LogEventv(logger, priority, keyword, subkeyword, text, argp);
	va_end(argp);
	return byte_counter;
}

int LoggerObjC_LogEventv(Logger* logger, unsigned int priority, 
	const char* keyword, const char* subkeyword,
	NSString* text, va_list argp)
{
	char time_stamp[LOGGER_TIME_STAMP_SIZE];
	int byte_counter = 0;
	int ret_val;
	int error_flag = 0;
	
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);

	if(!logger->loggingEnabled)
	{
		LOGGER_UNLOCKMUTEX(logger);
		return 0;
	}
	if(0 == priority)
	{
		priority = logger->defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < logger->thresholdPriority)
	{
		LOGGER_UNLOCKMUTEX(logger);
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
			if(!Logger_SegmentFile(logger))
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

	/* Assuming the string will be used at least once, if not twice.
	 * Further optimizations could be done to check this, but it probably isn't worth it.
	 */
	NSString* the_string = [[NSString alloc] initWithFormat:text arguments:argp];
	const char* formatted_string = [the_string UTF8String];

	/* Log stuff to fileHandle */
	if(logger->fileHandle != NULL)
	{
		int use_custom_print = LOGGER_USE_CUSTOM_PRINT(logger);
		if(0 == use_custom_print)
		{
			ret_val = Logger_PrintHeaderToFileHandle(logger->fileHandle,
				logger->preNewLines,
				priority, time_stamp, keyword, subkeyword);
		}
		else
		{
			ret_val = Logger_PrintHeaderWithCustom(logger,
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
				ret_val = fputs(formatted_string, logger->fileHandle);
			}
			else
			{
				ret_val = logger->customPuts(logger, logger->customCallbackUserData, formatted_string);
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
			ret_val = Logger_PrintFooterToFileHandle(logger->fileHandle,
				logger->postNewLines,
				logger->autoFlushEnabled);
		}
		else
		{
			ret_val = Logger_PrintFooterWithCustom(logger,
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
		ret_val = Logger_PrintHeaderToFileHandle(logger->echoHandle,
			logger->preNewLines,
			priority, time_stamp, keyword, subkeyword);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

		if(text != NULL)
		{
			ret_val = fputs(formatted_string, logger-> echoHandle);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}
		}
		
		ret_val = Logger_PrintFooterToFileHandle(logger->echoHandle,
			logger->postNewLines,
			logger->autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}
#if ! __has_feature(objc_arc)
	[the_string release];
#endif

	LOGGER_UNLOCKMUTEX(logger);

	if(error_flag == 0)
	{
		return byte_counter;
	}
	else
	{
		return error_flag;
	}
}


#endif /* __OBJC__ */

