
#ifdef __OBJC__

#include "LoggerObjC.h"
#import <Foundation/Foundation.h>
#include "TimeStamp.h"
#include "LoggerPrimitives.h"


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

/* There is too much copy-and-paste here. 
 * But the motivation is to keep performance as high as possible which means doing things
 * like checking the enabled and priorty levels before evaluating the format strings.
 * I don't know any good way to evaluate the NSString without alloc'ing an instance which 
 * is less optimal than the C implementation which avoids malloc calls.
 * But since we have to malloc a string, we can at least reuse it for the echo and avoid reparsing.
 * Hence, there is a lot of code duplication to get access to these subtle optimizations.
 */
int LoggerObjC_LogEventv(Logger* logger, unsigned int priority, 
	const char* keyword, const char* subkeyword,
	NSString* text, va_list argp)
{
	char time_stamp[LOGGER_TIME_STAMP_SIZE];
	size_t byte_counter = 0;
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

	/* Assuming the string will be used at least once, if not twice.
	 * Further optimizations could be done to check this, but it probably isn't worth it.
	 */
	NSString* the_string = [[NSString alloc] initWithFormat:text arguments:argp];
	/* Hoping Obj-C garbage collection and ARC are smart enough to not collect the NSString until after I'm done with the internal pointer. */
	/* For ARC, use look at objc_precise_lifetime if there is a problem. 
	 * ARC complains if I declare as __strong const char*. I removed the __strong hoping GC won't care, but also since it's going away, it's not worth worrying about now.
	 */
	const char* formatted_string = [the_string UTF8String];


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
		
		if(NULL != formatted_string)
		{
			if(0 == use_custom_print)
			{
				ret_val = fputs(formatted_string, logger->fileHandle);
			}
			else
			{
				ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, formatted_string);
			}
			if(ret_val >= 0)
			{
				/* fputs doesn't return the number of bytes written unlike fprintf, so we must compute it ourselves.
					Using the UTF8-string length (via strlen on the UTF8String) because it might be different than the [NSString length].
				*/
				byte_counter = byte_counter + strlen(formatted_string);
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

		if(NULL != formatted_string)
		{
			/* Not taking any chances with fprintf. */
			ret_val = fputs(formatted_string, logger->echoHandle);
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

#if ! __has_feature(objc_arc)
	[the_string release];
#endif

	if(error_flag == 0)
	{
		return (int)byte_counter;
	}
	else
	{
		return error_flag;
	}

}

#endif /* __OBJC__ */

