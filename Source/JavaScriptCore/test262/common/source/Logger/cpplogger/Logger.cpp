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


#include "Logger.hpp"
#include "TimeStamp.h"

//#include <cstdio>
//#include <cstdarg>
#include <cerrno>

#include <sstream> // needed for the segment_ext numbers formating
#include <iomanip> // needed for the segment_ext numbers formating

/* For echoing, I need va_copy or the undefined system behavior to work 
 * like va_copy. Otherwise, echoing will not work and you will get the 
 * wrong values echoed in your format strings.
 */
#ifndef va_copy
	#ifdef __va_copy
		#define va_copy(a, b) __va_copy(a, b)
		#define VA_COPYEND(a) va_end(a)
	#else
		/* This is a fallback that people claim most compilers 
		 * without va_copy will support. If not, you need to 
		 * disable echoing in the code. 
		 * If you simply get a type casting error, you might also 
		 * try modifying the code to not use va_copy but instead
		 * directly reuse the argp variable. This actually worked for
		 * me before I discovered I needed va_copy on some systems.
		 */
		#define VA_COPY(a, b) ((a) = (b))
		#define VA_COPYEND(a) 
	#endif
#else
	#define VA_COPY(a, b) va_copy(a, b)
	#define VA_COPYEND(a) va_end(a)
#endif


/* I'm not sure if I should use #define or const
 * and if I should put this here (hidden) or in the header file.
 * Also, rememeber if you change these values, you need to update
 * the documentation in the header.
 */
#define LOGGER_DEFAULT_MIN_SEGMENT_SIZE 0
#define LOGGER_DEFAULT_THRESHOLD 1
#define LOGGER_DEFAULT_PRIORITY 5
#define LOGGER_DEFAULT_PRE_NEWLINES 0
#define LOGGER_DEFAULT_POST_NEWLINES 1

#if 0 // Unfortunately, not supported at the moment.
/* This should be something like ".%04d". This particular 
 * string will be used in an snprintf and be used to 
 * format the log segment extension with 0 padding if needed.
 * .0001, .0002, .0003, etc.
 */
const std::string LOGGER_DEFAULT_SEGMENT_FORMAT_STRING = ".%04d";
#endif

#define LOGGER_DEFAULT_SEGMENT_MIN_WIDTH 3

/* I know my time stamp library always returns a string of size 24 */
#define LOGGER_TIME_STAMP_SIZE 24

//using namespace std;


/* This is for basic thread safety. I'm on the fence about if I want 
 * to deal with multi-process safety right now, but I suppose if 
 * I did, I would add/remove an flock/funlock to everything.
 * This is not tested on Windows. This whole thing should be
 * considered experimental.
 */
#ifdef LOGGER_ENABLE_LOCKING
	#if defined(_WIN32) && !defined(__CYGWIN32__)
		#include <windows.h>
		#include <winbase.h> /* For CreateMutex(), LockFile() */

		static void* Logger_CreateMutex()
		{
			return((void*)CreateMutex(NULL, FALSE, NULL));
		}
		static void Logger_DestroyMutex(void* mutex)
		{
			if(NULL != mutex)
			{
				CloseHandle( (HANDLE)mutex );
			}
		}
		/* This will return true if locking is successful, false if not.
		 */
		static int Logger_LockMutex(void* mutex)
		{
			return(
				WaitForSingleObject(
					(HANDLE)mutex,
					INFINITE
				) != WAIT_FAILED
			);
		}
		static void Logger_UnlockMutex(void* mutex)
		{
			ReleaseMutex(
				(HANDLE)mutex
			);
		}
		/* Wow, I'm really screwed. There is no way to go from 
		 * an ANSI FILE* to a Microsoft HANDLE.
		 */
		/*
		static void Logger_LockFile(FILE* file_handle)
		{
			/ * This should lock for both threads and processes * /
			LockFile(file_handle);
		}
		static void Logger_UnlockFile(FILE* file_handle)
		{
			UnlockFile(file_handle);
		}
		*/

		#define LOGGER_LOCKMUTEX(X) Logger_LockMutex((X))
		#define LOGGER_UNLOCKMUTEX(X) Logger_UnlockMutex((X))

		/* Because of the type problems, I can't do what I want without
		 * some major rewriting. Instead, I'll take the cheap way 
		 * out and hope that the mutex locks are enough.
		 */
		#define LOGGER_LOCKFILE(X)
		#define LOGGER_UNLOCKFILE(X)

	#else /* Assuming posix, probably should be more robust. */
		#include <pthread.h>
		static void* Logger_CreateMutex()
		{
			int ret_val;
			pthread_mutex_t* m = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)); 
			if(NULL == m)
			{
				return NULL;
			}
			ret_val = pthread_mutex_init(m, NULL);
			if(0 != ret_val)
			{
				free(m);
				return NULL;
			}
			return((void*)m);
		}
		static void Logger_DestroyMutex(void* mutex)
		{
			if(NULL != mutex)
			{
				pthread_mutex_destroy((pthread_mutex_t*) (mutex));
				free(mutex);
			}
		}
		/* This will return true if locking is successful, false if not.
		 * (This is the opposite of pthread_mutex_lock which returns 
		 * 0 for success.)
		 */
		static int Logger_LockMutex(void* mutex)
		{
			return(
				pthread_mutex_lock(
					(pthread_mutex_t*)mutex
				) == 0
			);
		}
		static void Logger_UnlockMutex(void* mutex)
		{
			pthread_mutex_unlock(
				(pthread_mutex_t*)mutex
			);
		}
		static void Logger_LockFile(FILE* file_handle)
		{
			flockfile(file_handle);
		}
		static void Logger_UnlockFile(FILE* file_handle)
		{
			funlockfile(file_handle);
		}

		#define LOGGER_LOCKMUTEX(X) Logger_LockMutex((X))
		#define LOGGER_UNLOCKMUTEX(X) Logger_UnlockMutex((X))

		#define LOGGER_LOCKFILE(X) Logger_LockFile((X))
		#define LOGGER_UNLOCKFILE(X) Logger_UnlockFile((X))
	#endif
#else
	/* Basically, I want to turn everything here into no-op's */
	#define LOGGER_LOCKMUTEX(X)
	#define LOGGER_UNLOCKMUTEX(X)
	#define LOGGER_LOCKFILE(X)
	#define LOGGER_UNLOCKFILE(X)
#endif


LoggerVersion Logger::GetLinkedVersion()
{
	LoggerVersion ver;
	ver.major = LOGGER_MAJOR_VERSION;
	ver.minor = LOGGER_MINOR_VERSION;
	ver.patch = LOGGER_PATCH_VERSION;
	return ver;
}

bool Logger::IsCompiledWithLocking()
{
#ifdef LOGGER_ENABLE_LOCKING
	return true;
#else
	return false;
#endif
}

Logger::Logger()
#ifdef LOGGER_ENABLE_LOCKING
	: loggerMutex(Logger_CreateMutex())
#endif
{
	Init();
}

bool Logger::Init()
{
	LOGGER_LOCKMUTEX(loggerMutex);

	baseName = static_cast<std::string>("");
	fileHandle = stdout;
	echoHandle = stdout;
	thresholdPriority = LOGGER_DEFAULT_THRESHOLD;
	defaultPriority = LOGGER_DEFAULT_PRIORITY;
	echoOn = false;
	loggingEnabled = true;
	autoFlushEnabled = false;
	preNewLines = LOGGER_DEFAULT_PRE_NEWLINES;
	postNewLines = LOGGER_DEFAULT_POST_NEWLINES;
	segmentNumber = 0;
	currentByteCount = 0;
	minSegmentBytes = LOGGER_DEFAULT_MIN_SEGMENT_SIZE;

#if 0 // Unfortunately, not supported at the moment.
	segmentFormatString = LOGGER_DEFAULT_SEGMENT_FORMAT_STRING;
#endif
	segmentMinWidth = LOGGER_DEFAULT_SEGMENT_MIN_WIDTH;

	LOGGER_UNLOCKMUTEX(loggerMutex);

	return true;
}

bool Logger::Init(const std::string& base_name)
{
	bool ret_flag;

	
	// If the system has been Init before with an open file
	// we should close everything and try to start over.
	// Don't lock here because both Close() and Init() lock
	Close();
	Init();

	LOGGER_LOCKMUTEX(loggerMutex);

	ret_flag = OpenFile(base_name);
	if(false == ret_flag)
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);
		return false;
	}
	baseName = base_name;
	
	LOGGER_UNLOCKMUTEX(loggerMutex);
	
	return true;
}

bool Logger::Init(FILE* file_handle, const std::string& base_name)
{
	
	// If the system has been Init before with an open file
	// we should close everything and try to start over.
	// Don't lock here because both Close() and Init() lock
	Close();
	Init();

	LOGGER_LOCKMUTEX(loggerMutex);
	
	if(NULL == file_handle)
	{
		return false;
	}
	if( (stdout == file_handle) 
		|| (stderr == file_handle)
	)
	{
		baseName = "";
		fileHandle = file_handle;
		LOGGER_UNLOCKMUTEX(loggerMutex);
		return true;
	}
	
	/* Make sure the file name is not empty for a real file */
	if(base_name.empty())
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);
		return false;
	}
	
	fileHandle = file_handle;
	baseName = base_name;
	
	LOGGER_UNLOCKMUTEX(loggerMutex);
	
	return true;
}

Logger::~Logger()
{
	Close();
#ifdef LOGGER_ENABLE_LOCKING
	if(NULL != loggerMutex)
	{
		Logger_DestroyMutex(loggerMutex);
	}
#endif
}

void Logger::Close()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	CloseFile();
	fileHandle = NULL;
	echoHandle = NULL;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

bool Logger::OpenFile(const std::string& file_name)
{
	fileHandle = fopen(file_name.c_str(), "w");
	if(NULL == fileHandle)
	{
		fprintf(stderr, "Error: Could not open file %s for logging: %s\n", file_name.c_str(), strerror(errno));
		return false;
	}
	return true;
}
				
bool Logger::CloseFile()
{
	if( (fileHandle == stdout) 
			|| (fileHandle == stderr)
			|| (fileHandle == NULL) )
	{
		return true;
	}
	if(fclose(fileHandle) == EOF)
	{
		perror("Error: Close file failed in logging");
		return 0;
	}
	return true;
}

bool Logger::SegmentFile()
{
	std::string segment_ext;
	std::string nextfile;
	std::ostringstream format_stream;
	
	/* Don't segment if there is no file.
	 * It's possible that a segment open may have failed
	 * but the log may continue when the next segment comes so
	 * don't give up on filehandle == NULL
	 */
	if( (stdout == fileHandle) 
			|| (stderr == fileHandle)
			|| (baseName.empty())
	)
	{
		return true;
	}
	
	// Corner case: 
	// CloseFile will check for NULL pointer and return true if NULL
	CloseFile();
	
	// Append the segment number
	
	// This will dump the integer into the stringstream???
	// The std::setw(4) makes sure the width is 4
	// The std::setfill('0') uses 0 as the fill character
	// std::right is the default, but makes sure the value is right adjusted
	// format_stream << std::setw(4) << std::setfill('0') << std::right << i;
	format_stream << std::setw(segmentMinWidth) 
		<< std::setfill('0') << segmentNumber;
	// stupid VC6 doesn't like the one liner, so I need a temp var
	// filename = basename + format_stream.str() + ext;
	segment_ext = format_stream.str();
	nextfile = baseName + '.' + segment_ext;
	
	segmentNumber++;
	
	return OpenFile(nextfile);
}


/**
 * This function prints the header of the log entry.
 * The header includes the timestamp, keyword, subkeyword and priority.
 * It also prints the number of pre_new_lines required.
 * @warning This locks the file handle, but doesn't unlock it.
 * Use the PrintFooter to properly unlock the file handle.
 * This function does not check to see if 
 * the file_handle is NULL.
 * @return The number of bytes printed.
 */
int Logger::PrintHeaderToFileHandle(FILE* file_handle, 
	unsigned int pre_new_lines,
	unsigned int priority,
	const char* time_stamp, 
	const char* keyword, 
	const char* subkeyword)
{
	int byte_counter = 0;
	unsigned int i;
	int ret_val;

	LOGGER_LOCKFILE(file_handle);
	if(pre_new_lines > 0)
	{
		for(i=0; i<pre_new_lines; i++)
		{
			ret_val = fputs("\n", file_handle);
			if(ret_val < 0)
			{
				/* Safe to return early here, but remember 
				 * that UNLOCKFILE is called in the footer. */
				return ret_val;
			}
			byte_counter += sizeof("\n");
		}
	}
	
	/* YYYY-MM-DD_HH:MM:SS.mmm    keyword:    subkeyword:    PRI=#
	 * Note 4 spaces between words
	 * 2002-10-29_13:00:00.000    Sample:    Comment:    PRI=0
	 */
	if( (NULL == keyword) && (NULL == subkeyword) )
	{
		ret_val = fprintf(file_handle, "%s    :    :    PRI=%d\n", 
			time_stamp, priority);
	}
	else if(NULL == subkeyword)
	{
		ret_val += fprintf(file_handle, "%s    %s:    :    PRI=%d\n", time_stamp, keyword, priority);

	}
	else if(NULL == keyword)
	{
		ret_val += fprintf(file_handle, "%s    :    %s:    PRI=%d\n", time_stamp, subkeyword, priority);
	}
	else
	{		
		ret_val += fprintf(file_handle, "%s    %s:    %s:    PRI=%d\n", time_stamp, keyword, subkeyword, priority);
	}

	if(ret_val >= 0)
	{
		byte_counter = byte_counter + ret_val;
	}
	else
	{
		/* return the error condition instead */
		byte_counter = ret_val;
	}
	return byte_counter;
}

/**
 * This prints the footer of a log entry.
 * The footer includes the END: marker and the 
 * number of post-new-lines required.
 * @warning This unlocks a file that was presumably left locked by
 * the PrintHeader call. This function does not check to see if 
 * the file_handle is NULL.
 * @return The number of bytes written for the footer.
 */
int Logger::PrintFooterToFileHandle(FILE* file_handle, 
	unsigned int post_new_lines,
	bool auto_flush_enabled)
{
	int byte_counter = 0;
	unsigned int i;
	int ret_val;
	int error_flag = 0;

	error_flag = fputs(LOGGER_LOGENDTOKEN, file_handle);
	if(error_flag >= 0)
	{
		byte_counter += sizeof(LOGGER_LOGENDTOKEN);
	}
	
	if(post_new_lines > 0)
	{
		for(i=0; i<post_new_lines; i++)
		{
			ret_val = fputs("\n", file_handle);
			/* Try to record/preserve the first error flag because subsequent 
			 * messages might be less helpful. */
			if(ret_val >= 0)
			{
				byte_counter += sizeof("\n");
			}
			else if(0 == error_flag)
			{
				error_flag = ret_val;
			}
		}
	}
	if(auto_flush_enabled)
	{
		ret_val = fflush(file_handle);
		/* Try to record/preserve the first error flag because subsequent 
		 * messages might be less helpful. */
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}
	}
	LOGGER_UNLOCKFILE(file_handle);
	if(error_flag == 0)
	{
		return byte_counter;
	}
	else
	{
		return error_flag;
	}
}

int Logger::LogEvent(unsigned int priority, 
	const char* keyword, const char* subkeyword,
	const char* text, ...)
{
	int byte_counter;
	va_list argp;
	va_start(argp, text);
	byte_counter = LogEventv(priority, keyword, subkeyword, text, argp);
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
int Logger::LogEventv(unsigned int priority, 
	const char* keyword, const char* subkeyword,
	const char* text, va_list argp) 
{
	char time_stamp[LOGGER_TIME_STAMP_SIZE];
	int byte_counter = 0;
	int ret_val;
	int error_flag = 0;
	va_list echo_copy;
	
	LOGGER_LOCKMUTEX(loggerMutex);

	if(!loggingEnabled)
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);		
		return 0;
	}
	if(0 == priority)
	{
		priority = defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < thresholdPriority)
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);				
		return 0;
	}
	
	/* Get the current time */
	TimeStamp_GetTimeStamp(time_stamp, LOGGER_TIME_STAMP_SIZE);

	
	/* if segmentation is enabled */
	if(minSegmentBytes != 0)
	{
		if(currentByteCount > minSegmentBytes)
		{
			/* SegmentFile() will just return if fileHandle
			 * is not a file.
			 */
			if(!SegmentFile())
			{
				/* Might still want echo
				 * Commented out for now
				 * loggingEnabled = false;
				 * Pointer should be set to NULL
				 */
			}
			currentByteCount = 0;
		}
	}
	
	/* Log stuff to fileHandle */
	if(fileHandle != NULL)
	{
		ret_val = PrintHeaderToFileHandle(fileHandle,
			preNewLines,
			priority, time_stamp, keyword, subkeyword);
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
			if(echoOn && (NULL != echoHandle))
			{
				VA_COPY(echo_copy, argp);
			}
			ret_val = vfprintf(fileHandle, text, argp);
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
		
		ret_val = PrintFooterToFileHandle(fileHandle,
			postNewLines,
			autoFlushEnabled);
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
		currentByteCount += byte_counter;
	}
	/* Repeat again (to terminal) if echo is on. */
	if(echoOn && (NULL != echoHandle))
	{
		ret_val = PrintHeaderToFileHandle(echoHandle,
			preNewLines,
			priority, time_stamp, keyword, subkeyword);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

		if(text != NULL)
		{
			ret_val = vfprintf(echoHandle, text, echo_copy);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}

			VA_COPYEND(echo_copy);
		}
		
		ret_val = PrintFooterToFileHandle(echoHandle,
			postNewLines,
			autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}

	LOGGER_UNLOCKMUTEX(loggerMutex);

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
int Logger::LogEventNoFormat(unsigned int priority, 
					 const char* keyword, const char* subkeyword,
					 const char* text)
{
	char time_stamp[LOGGER_TIME_STAMP_SIZE];
	int byte_counter = 0;
	int ret_val;
	int error_flag = 0;	
	
	LOGGER_LOCKMUTEX(loggerMutex);

	if(!loggingEnabled)
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);		
		return 0;
	}
	if(0 == priority)
	{
		priority = defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < thresholdPriority)
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);				
		return 0;
	}
	
	/* Get the current time */
	TimeStamp_GetTimeStamp(time_stamp, LOGGER_TIME_STAMP_SIZE);
	
	
	/* if segmentation is enabled. */
	if(minSegmentBytes != 0)
	{
		if(currentByteCount > minSegmentBytes)
		{
			/* SegmentFile() will just return if fileHandle
			 * is not a file
			 */
			if(!SegmentFile())
			{
				/* Might still want echo
				 * Commented out for now
				 * loggingEnabled = false;
				 * Pointer should be set to NULL
				 */
			}
			currentByteCount = 0;
		}
	}
	/* Log stuff to fileHandle */
	if(fileHandle != NULL)
	{
		ret_val = PrintHeaderToFileHandle(fileHandle,
			preNewLines,
			priority, time_stamp, keyword, subkeyword);
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
			/* Not taking any chances with fprintf. */
			ret_val = fputs(text, fileHandle);
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
		
		ret_val = PrintFooterToFileHandle(fileHandle,
			postNewLines,
			autoFlushEnabled);
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
		currentByteCount += byte_counter;
	}
	/* Repeat again (to terminal) if echo is on. */
	if(echoOn && (NULL != echoHandle))
	{
		ret_val = PrintHeaderToFileHandle(echoHandle,
			preNewLines,
			priority, time_stamp, keyword, subkeyword);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

		if(text != NULL)
		{
			/* Not taking any chances with fprintf. */
			ret_val = fputs(text, echoHandle);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}

		}
		
		ret_val = PrintFooterToFileHandle(echoHandle,
			postNewLines,
			autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}	
		
	LOGGER_UNLOCKMUTEX(loggerMutex);

	if(error_flag == 0)
	{
		return byte_counter;
	}
	else
	{
		return error_flag;
	}
}

void Logger::Enable()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	loggingEnabled = true;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

void Logger::Disable()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	loggingEnabled = false;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

bool Logger::IsEnabled() const
{
	bool ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = loggingEnabled;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

int Logger::Flush()
{
	int ret_val;
	
int error_flag = 0;
	LOGGER_LOCKMUTEX(loggerMutex);
	if(fileHandle != NULL)
	{
		error_flag = fflush(fileHandle);
	}
	if(echoOn && (NULL != echoHandle))
	{
		if(echoHandle != NULL)
		{
			ret_val = fflush(echoHandle);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}
		}
	}
	LOGGER_UNLOCKMUTEX(loggerMutex);

	return error_flag;
}

void Logger::EnableAutoFlush()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	autoFlushEnabled = true;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

void Logger::DisableAutoFlush()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	autoFlushEnabled = false;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

bool Logger::IsAutoFlushEnabled() const
{
	bool ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = autoFlushEnabled;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

void Logger::SetThresholdPriority(unsigned int thresh_val)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	if(0 == thresh_val)
	{
		thresholdPriority = LOGGER_DEFAULT_THRESHOLD;
	}
	else
	{
		thresholdPriority = thresh_val;
	}
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

unsigned int Logger::GetThresholdPriority() const
{
	unsigned int ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = thresholdPriority;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

void Logger::SetDefaultPriority(unsigned int pri_level)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	if(0 == pri_level)
	{
		defaultPriority = LOGGER_DEFAULT_PRIORITY;
	}
	else
	{
		defaultPriority = pri_level;
	}
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

unsigned int Logger::GetDefaultPriority() const
{
	unsigned int ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = defaultPriority;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

void Logger::SetMinSegmentSize(unsigned int num_bytes)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	minSegmentBytes = num_bytes;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

unsigned int Logger::GetMinSegmentSize() const
{
	unsigned int ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = minSegmentBytes;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

void Logger::EchoOn()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	echoOn = true;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

void Logger::EchoOff()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	echoOn = false;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

bool Logger::IsEchoOn() const
{
	bool ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = echoOn;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

void Logger::SetEchoToStdout()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	echoHandle = stdout;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

void Logger::SetEchoToStderr()
{
	LOGGER_LOCKMUTEX(loggerMutex);
	echoHandle = stderr;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

void Logger::SetPreLines(unsigned int pre_lines)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	preNewLines = pre_lines;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

void Logger::SetPostLines(unsigned int post_lines)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	postNewLines = post_lines;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

void Logger::SetNewLines(unsigned int prelines, unsigned int postlines)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	preNewLines = prelines;
	postNewLines = postlines;
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

unsigned int Logger::GetPreLines() const
{
	unsigned int ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = preNewLines;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

unsigned int Logger::GetPostLines() const
{
	unsigned int ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = postNewLines;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

bool Logger::WillLog(unsigned int priority) const
{
	LOGGER_LOCKMUTEX(loggerMutex);
	if(!loggingEnabled)
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);
		return false;
	}
	if(0 == priority)
	{
		priority = defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < thresholdPriority)
	{
		LOGGER_UNLOCKMUTEX(loggerMutex);
		return false;
	}
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return true;
}

#if 0
/* Unfortunately, not supported at the moment.
 * The reason I'm  reluctant to copy over the code from the C version
 * for this is because the code that uses this is so complicated. 
 * Because it is a printf formatted string, there are many risks of 
 * buffer-overflows.
 * The C code went to great lengths to avoid overflows, but 
 * C++ strings are just so much safer to use. For now, I'm not
 * compelled enough to bring over all that code and potentially 
 * introduce overflow issues in this code.
 */
void Logger::SetSegmentFormatString(const std::string& format_string)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	if(format_string.empty())
	{
		/* Reset the system to use the default format string. */
		segmentFormatString = LOGGER_DEFAULT_SEGMENT_FORMAT_STRING;
	}
	else
	{
		segmentFormatString = format_string;
	}
	LOGGER_UNLOCKMUTEX(loggerMutex);
}
#endif

void Logger::SetSegmentMinWidth(unsigned int width)
{
	LOGGER_LOCKMUTEX(loggerMutex);
	if(0 == width)
	{
		segmentMinWidth = LOGGER_DEFAULT_SEGMENT_MIN_WIDTH;
	}
	else
	{
		segmentMinWidth = width;
	}
	LOGGER_UNLOCKMUTEX(loggerMutex);
}

unsigned int Logger::GetSegmentMinWidth() const
{
	int ret_val;
	LOGGER_LOCKMUTEX(loggerMutex);
	ret_val = segmentMinWidth;
	LOGGER_UNLOCKMUTEX(loggerMutex);
	return ret_val;
}

