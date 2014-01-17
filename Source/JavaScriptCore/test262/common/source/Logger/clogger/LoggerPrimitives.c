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


#include "LoggerPrimitives.h"

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

		void* LoggerPrimitives_CreateMutex()
		{
			return((void*)CreateMutex(NULL, FALSE, NULL));
		}
		void LoggerPrimitives_DestroyMutex(void* mutex)
		{
			if(NULL != mutex)
			{
				CloseHandle( (HANDLE)mutex );
			}
		}
		/* This will return true if locking is successful, false if not.
		 */
		int LoggerPrimitives_LockMutex(Logger* logger)
		{
			return(
				WaitForSingleObject(
					(HANDLE)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex,
					INFINITE
				) != WAIT_FAILED
			);
		}
		void LoggerPrimitives_UnlockMutex(Logger* logger)
		{
			ReleaseMutex(
				(HANDLE)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex
			);
		}
	#else /* Assuming posix, probably should be more robust. */
		#include <pthread.h>
		void* LoggerPrimitives_CreateMutex()
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
		void LoggerPrimitives_DestroyMutex(void* mutex)
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
		int LoggerPrimitives_LockMutex(Logger* logger)
		{
			return(
				pthread_mutex_lock(
					(pthread_mutex_t*)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex
				) == 0
			);
		}
		void LoggerPrimitives_UnlockMutex(Logger* logger)
		{
			pthread_mutex_unlock(
				(pthread_mutex_t*)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex
			);
		}

	#endif
#else
	/* Basically, I want to turn everything here into no-op's */
#endif

#ifdef LOGGER_ENABLE_FILELOCKING
	#if defined(_WIN32) && !defined(__CYGWIN32__)
		#include <windows.h>
		#include <winbase.h> /* For CreateMutex(), LockFile() */

		/* Wow, I'm really screwed. There is no way to go from 
		 * an ANSI FILE* to a Microsoft HANDLE.
		 * Look at _open_osfhandle with _fdopen to open a native Windows 
		 * file and convert to stdio, but that requires some reworking and more
		 * platform specific code I would like to avoid.
		 */
		/*
		void Logger_LockFile(FILE* file_handle)
		{
			/ * This should lock for both threads and processes * /
			LockFile(file_handle);
		}
		void Logger_UnlockFile(FILE* file_handle)
		{
			UnlockFile(file_handle);
		}
		*/
	#else /* posix */
/*
		void Logger_LockFile(FILE* file_handle)
		{
			flockfile(file_handle);
		}
		void Logger_UnlockFile(FILE* file_handle)
		{
			funlockfile(file_handle);
		}
*/
	#endif
#else /* LOGGER_ENABLE_FILELOCKING */

#endif


/**
 * Copies a source string, potentially to a target string, and returns 
 * the pointer to the copied string.
 * This function is a intended to be an efficient string copy function.
 * It's purpose is to copy a string into a string with preallocated memory
 * and avoid dynamic memory allocation if possible. If memory must 
 * be allocated, then the old string will be destroyed.
 *
 * This is only to be used where target_string was created with dynamic 
 * memory. This function will destroy the memory and allocate new memory
 * if there is not enough space in the target string.
 *
 * @param target_string This is the string you would like to try 
 * to copy into. If there is not enough space, a new string will
 * be created and the target_string will be freed. This string 
 * must have been created dynamically. This may be NULL if you 
 * wish for this function to dynamically create a new string 
 * for you.
 *
 * @param target_max_buffer_size This is a pointer that points to 
 * an address containing the size of the preallocated target_string. 
 * This size is the maximum buffer length which includes the '\\0'
 * character as part of that count. This pointer may not be NULL.
 * If you pass in NULL for the target_string (indicating you want 
 * a new string allocated for you), then the size should be set to 0.
 * When the function completes, the size will be set to the new 
 * max buffer size of the string if the string needed to be reallocated.
 *
 * @param source_string This is the string you want to copy. If it's NULL,
 * the target_string will have it's memory freed.
 *
 * @return Will return a pointer to the duplicated string. Be aware 
 * of several things:
 * - The returned pointer address may not be the same address as the 
 * target string passed in (due to a possible reallocation).
 * - If the pointer to the source and target string 
 * are the same, the pointer to the target string will be returned.
 * - If the source string is NULL, the target string
 * will be freed and will return NULL.
 * - If an error occurs, NULL will be returned.
 *
 * Also note that the value at the address target_max_buffer_size points 
 * to will be filled with the new max buffer size for the string.
 *
 * Example:
 * @code
 *
 * int main()
 * {
 * 		const char* original1 = "Hello World";
 * 		const char* original2 = "Smaller";
 * 		const char* original3 = "Good-Bye World";
 * 		char* ret_val;
 * 		char* target = NULL;
 * 		size_t target_max_buffer_size = 0;
 *
 * 		ret_val = CopyDynamicString(target, &target_max_buffer_size, original1);
 *
 * 		if(ret_val)
 * 		{
 * 			fprintf(stderr, "Target is '%s' with max size = %d\n", ret_val, target_max_buffer_size);
 * 		}
 * 		else
 * 		{
 * 			fprintf(stderr, "Error in function\n");
 * 		}
 *		target = ret_val;
 *
 *		ret_val = CopyDynamicString(target, &target_max_buffer_size, original2);
 * 		fprintf(stderr, "Target is '%s' with max size = %d\n", ret_val, target_max_buffer_size);
 *
 * 		target = ret_val; *
 * 		ret_val = CopyDynamicString(target, &target_max_buffer_size, original3);
 * 		fprintf(stderr, "Target is '%s' with max size = %d\n", ret_val, target_max_buffer_size);
 *
 * 		return 0;
 * }
 * @endcode
 * This outputs:
 * @code
 * Target is 'Hello World' with max size = 12
 * Target is 'Smaller' with max size = 12
 * Target is 'Good-Bye World' with max size = 15
 * @endcode
 */
char* LoggerPrimitives_CopyDynamicString(char* target_string, size_t* target_max_buffer_size, const char* source_string)
{
	/* If the pointers are the same, no copy is needed. */
	if(source_string == target_string)
	{
		/* I don't feel like asserting if the sizes are the same. */
		/* Return 1 instead of 0 because maybe this isn't an error?
		 */
		return target_string;
	}

	/* Make sure the size pointer is valid. */
	if(NULL == target_max_buffer_size)
	{
		return NULL;
	}

	/* Yikes, if the string is NULL, should we make the target string NULL? */
	if(NULL == source_string)
	{
		*target_max_buffer_size = 0;
		free(target_string);
		target_string = NULL;
		return NULL;
	}

	/* If target_string is NULL, the *target_max_buffer_size should also be 0.
	 * Technically, the user should set this and this would be an error,
	 * but I'll be nice for now. An alternate implementation might suggest 
	 * that the size would be the desired size the user wants for a new string.
	 */
	if( (NULL == target_string) && (0 != *target_max_buffer_size) )
	{
		*target_max_buffer_size = 0;
	}

	/* If there is not enough preallocated memory in the target string,
	 * then we need to reallocate enough memory.
	 */
	if( *target_max_buffer_size < (strlen(source_string) + 1) )
	{
		*target_max_buffer_size = 0;
		if(NULL != target_string)
		{
			free(target_string);
		}
		target_string = (char*)calloc( (strlen(source_string) + 1), sizeof(char) );
		if(NULL == target_string)
		{
			return NULL;
		}
		*target_max_buffer_size = strlen(source_string) + 1;
	}

	/* At this point, there should be enough preallocated 
	 * memory to call strncpy.
	 */
	strncpy(target_string, source_string, *target_max_buffer_size);

	return target_string;
}


/**
 * Private function to malloc a new logger instance
 * and initialize its values to default settings.
 * @return Returns a pointer to a new logger instance
 * with all its fields initialized to default values.
 * On malloc failure, it will return NULL.
 */
Logger* LoggerPrimitives_MallocAndInit()
{
	Logger* logger;
	char* initial_format_string;

	/* Yeah, calloc might be a bit slower, but I don't 
	 * want to take any chances with accidentally missing a \0.
	 * I just added the custom callback functions. This makes it easy to initialize.
	 */
	logger = (Logger*)calloc(1, sizeof(Logger));
	if(NULL == logger)
	{
		return NULL;
	}
	/* Yeah, calloc might be a bit slower, but I don't 
	 * want to take any chances with accidentally missing a \0.
	 */
	initial_format_string = (char*)calloc(LOGGER_DEFAULT_SEGMENT_FORMAT_STRING_SIZE, sizeof(char));
	if(NULL == initial_format_string)
	{
		/* Need to free the memory from the logger allocation. */
		free(logger);
		return NULL;
	}
	logger->opaqueLoggerData = calloc(1, sizeof(LoggerOpaqueData));
	if(NULL == logger->opaqueLoggerData)
	{
		free(initial_format_string);
		free(logger);
		return NULL;
	}
#ifdef LOGGER_ENABLE_LOCKING
	/* Create the Mutex only if locking is enabled. */
	((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex = LOGGERPRIMITIVES_CREATEMUTEX();
	if(NULL == ((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex)
	{
		free(logger->opaqueLoggerData);
		free(initial_format_string);
		free(logger);
		return NULL;
	}
#else
	((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex = NULL;
#endif



	/* Give logger it's own copy of the format string. This 
	 * is a little wasteful since it is a literal, but if 
	 * I allow the user to set a new format string, the code 
	 * will get harder to understand if I'm checking to see if I'm
	 * using the literal version or a version I had to allocate 
	 * memory for.
	 */
	strncpy(initial_format_string, LOGGER_DEFAULT_SEGMENT_FORMAT_STRING, LOGGER_DEFAULT_SEGMENT_FORMAT_STRING_SIZE-1);

	logger->segmentFormatString = initial_format_string;
	logger->segmentFormatStringMaxSize = LOGGER_DEFAULT_SEGMENT_FORMAT_STRING_SIZE;


	logger->baseName = NULL;
	logger->baseNameMaxSize = 0;
	logger->fileHandle = stdout;
	logger->echoHandle = stdout;
	logger->thresholdPriority = LOGGER_DEFAULT_THRESHOLD;
	logger->defaultPriority = LOGGER_DEFAULT_PRIORITY;
	logger->echoOn = 0;
	logger->loggingEnabled = 1;
	logger->autoFlushEnabled = 0;
	logger->preNewLines = LOGGER_DEFAULT_PRE_NEWLINES;
	logger->postNewLines = LOGGER_DEFAULT_POST_NEWLINES;
	logger->segmentNumber = 0;
	logger->currentByteCount = 0;
	logger->minSegmentBytes = LOGGER_DEFAULT_MIN_SEGMENT_SIZE;

	/* Initialize the unused field for consistency. */
/*	((LoggerOpaqueData*)logger->opaqueLoggerData)->data1 = 0; */


	return logger;

}

/**
 * Opens a new file.
 * This is a private function for logger to open a new file 
 * and save the file handle to it's private member data.
 * @param logger The pointer to the logger instance.
 * @param file_name The name of the file to create.
 * @return Returns 1 on success, 0 on error. A statement
 * is currently printed to stderr on error.
 */
int LoggerPrimitives_OpenFile(Logger* logger, const char* file_name)
{
	if(NULL == logger)
	{
		return 0;
	}

	logger->fileHandle = fopen(file_name, "w");
	if(NULL == logger->fileHandle)
	{
		fprintf(stderr, "Error: Could not open file %s for logging: %s\n", file_name, strerror(errno));
		return 0;
	}
	return 1;
}
/**
 * Closes the current logger fileHandle.
 * Currently, SegmentFile depends on the behavior
 * that if fileHandle is NULL, the function returns 1.
 * The logger instance will get it's fileHandle set to NULL.
 * @param logger The pointer to the logger instance.
 * @return Returns 1 on success, 0 on error. A statement
 * is currently printed to stderr on error.
 */
int LoggerPrimitives_CloseFile(Logger* logger)
{
	if(NULL == logger)
	{
		return 0;
	}

	/* For stdout and stderr, we don't actually
	 * have a file to close.
	 */
	if( (stdout == logger->fileHandle) 
		|| (stderr == logger->fileHandle)
		|| (NULL == logger->fileHandle) )
	{
		logger->fileHandle = NULL;
		return 1;
	}
	if(fclose(logger->fileHandle) == EOF)
	{
		fprintf(stderr, "Error: Close file failed in logging\n");
		return 0;
	}
	logger->fileHandle = NULL;
	return 1;
}

/**
 * This is a private function that will segment a file.
 * This will start by closing the current file handle. 
 * Then it will attempt to open a new file by the same
 * base name. So if the base name was mylog.log, then 
 * the new file will be mylog.log.0001, mylog.log.0002,
 * etc.
 * If you're looking at the implementation below, 
 * you might be wondering why I didn't do one unified 
 * code path for creating the string name. Basically,
 * dynamic memory allocation worries me, especially for 
 * a system like Logger which is supposed to be a line of 
 * defense for error handling. If a memory error had occurred 
 * and logger was supposed to report it, it would not be 
 * good trying to allocate more memory. It's also very 
 * easy to mess up and write a buffer overflow. (I'm 
 * trying to be careful here.) There are also 
 * secondary considerations like performance, though I 
 * suppose it's not a real issue since the file system
 * will be far slower.
 * So my feeling is that the common usage won't create 
 * more than 1000 files. If you need more and don't want
 * dynamic memory, you could always beef up the constant.
 */
int LoggerPrimitives_SegmentFile(Logger* logger)
{
	/* I'm thinking it 1 (dot) + 4 (0001) + 1 (for '\0') */
	char segment_ext[LOGGER_PREALLOCATED_SEGMENT_CHARS];
	char next_file_string[LOGGER_PREALLOCATED_COPY_NAME + LOGGER_PREALLOCATED_SEGMENT_CHARS];
	/* This is the total number of characters for the extension */
	int total_num_ext_chars;
	int copy_string_size;
	int ret_flag; 
	const char* format_string = NULL;


	if(NULL == logger)
	{
		return 0;
	}

	/* Don't segment if there is no file.
	 * It's possible that a segment open may have failed
	 * but the log may continue when the next segment comes so
	 * don't give up on filehandle == NULL
	 */
	if( (stdout == logger->fileHandle) 
			|| (stderr == logger->fileHandle)
			|| (NULL == logger->baseName)
	)
	{
		return 1;
	}
	
	/* Corner case: 
	 * CloseFile will check for NULL pointer and return true if NULL
	 */
	LoggerPrimitives_CloseFile(logger);
	
	/* Append the segment number. 
	 * The number needs to be converted to a string. 
	 */

	/* This should never be NULL, but just in case it is,
	 * I'll try to handle it.
	 */
	if(NULL == logger->segmentFormatString)
	{
		format_string = LOGGER_DEFAULT_SEGMENT_FORMAT_STRING;
	}
	else
	{
		format_string = logger->segmentFormatString;
	}


	/* 
	 * Damn Windows doesn't have snprintf, but _snprintf.
	 * 
	 */
	total_num_ext_chars = snprintf(
		segment_ext, 
		LOGGER_PREALLOCATED_SEGMENT_CHARS,
		format_string, /* ".%04d" */
		logger->segmentNumber
	);
	/* If the string conversion yielded more characters,
	 * than we could handle, we need to do this with dynamic
	 * memory. Yuck.
	 */
	if(total_num_ext_chars >= LOGGER_PREALLOCATED_SEGMENT_CHARS)
	{
		char* dynamic_seg_ext;
		/* Yeah, calloc might be a bit slower, but I don't 
		 * want to take any chances with accidentally missing a \0.
		 */
		dynamic_seg_ext = (char*)calloc((total_num_ext_chars + 1), sizeof(char));
		if(NULL == dynamic_seg_ext)
		{
			fprintf(stderr, "Error in LoggerPrimitives_SegmentFile(). Could not allocate memory");
			return 0;
		}
		snprintf(dynamic_seg_ext, 
			total_num_ext_chars + 1, /* +1 because total didn't include \0 */
			format_string, /* ".%04d" */
			logger->segmentNumber
		);

		/* Now we have to make sure that
		 * basename + '.' + ext + '\0' <= size of the copy string.
		 * Note that the '.' is already included in the dynamic_seg_ext.
		 */
		copy_string_size = strlen(logger->baseName) + strlen(dynamic_seg_ext) + 1;
		if( copy_string_size >
			LOGGER_PREALLOCATED_COPY_NAME
		)
		{
			/* Yuck, again, we need to create more memory */
			char* dynamic_copy_string;
			dynamic_copy_string = (char*)calloc(copy_string_size, sizeof(char));
			if(NULL == dynamic_copy_string)
			{
				fprintf(stderr, "Error in LoggerPrimitives_SegmentFile(). Could not allocate memory");
				/* Need to free memory */
				free(dynamic_seg_ext);
				return 0;
			}

			/* Copy the base name over */
			strcpy(dynamic_copy_string, logger->baseName);
			/* Append the extension */
			strcat(dynamic_copy_string, dynamic_seg_ext);

			ret_flag = LoggerPrimitives_OpenFile(logger, dynamic_copy_string);
			/* Now clean up the temporary string */
			free(dynamic_copy_string);
		}
		else
		{
			/* We have enough preallocated memory to do this.
			 */
			/* Copy the base name over */
			strcpy(next_file_string, logger->baseName);
			/* Append the extension */
			strcat(next_file_string, dynamic_seg_ext);

			ret_flag = LoggerPrimitives_OpenFile(logger, next_file_string);
		}

		/* For both paths, we need to remember to clean up memory */
		free(dynamic_seg_ext);

	}
	/* Else, the segment fits in preallocated memory 
	 */
	else
	{
	
		/* Now we have to make sure that
		 * basename + '.' + ext + '\0' <= size of the copy string.
		 * Note that the '.' is already included in the dynamic_seg_ext.
		 */
		copy_string_size = strlen(logger->baseName) + strlen(segment_ext) + 1;
		if( copy_string_size >
			LOGGER_PREALLOCATED_COPY_NAME
		)
		{
			/* Yuck, again, we need to create more memory */
			char* dynamic_copy_string;
			dynamic_copy_string = (char*)calloc(copy_string_size, sizeof(char));
			if(NULL == dynamic_copy_string)
			{
				fprintf(stderr, "Error in LoggerPrimitives_SegmentFile(). Could not allocate memory");
				return 0;
			}

			/* Copy the base name over */
			strcpy(dynamic_copy_string, logger->baseName);
			/* Append the extension */
			strcat(dynamic_copy_string, segment_ext);

			ret_flag = LoggerPrimitives_OpenFile(logger, dynamic_copy_string);
			/* Now clean up the temporary string */
			free(dynamic_copy_string);
		}
		else
		{
			/* We have enough preallocated memory to do this.
			 */
			/* Copy the base name over */
			strcpy(next_file_string, logger->baseName);
			/* Append the extension */
			strcat(next_file_string, segment_ext);

			ret_flag = LoggerPrimitives_OpenFile(logger, next_file_string);
		}
	}

	/* For reference, this is the C++ way. This was the main 
	 * motivation to write this thing in C++ to begin with.
	 * Manipulating strings in C makes me edgy, especially
	 * with all the dynamic memory allocation I have to do
	 * and the lack of Microsoft providing snprintf (they
	 * provide (_snprintf) and some of the nicer things
	 * like asprintf.
	 */
	/* This will dump the integer into the stringstream???
	 * string segment_ext;
	 * string nextfile;
	 * ostringstream format_stream;
	 * The std::setw(4) makes sure the width is 4
	 * The std::setfill('0') uses 0 as the fill character
	 * std::right is the default, but makes sure the value is right adjusted
	 * format_stream << std::setw(4) << std::setfill('0') << std::right << i;
	 * format_stream << setw(4) << setfill('0') << segment_number;
	 * stupid VC6 doesn't like the one liner, so I need a temp var
	 * filename = basename + format_stream.str() + ext;
	 * segment_ext = format_stream.str();
	 * nextfile = file_basename + '.' + segment_ext;
	 */

	/* Increment the segment number for the next time around. */
	logger->segmentNumber++;

	return ret_flag;
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
int LoggerPrimitives_PrintHeaderToFileHandle(FILE* file_handle, 
	unsigned int pre_new_lines,
	unsigned int priority,
	const char* time_stamp, 
	const char* keyword, 
	const char* subkeyword)
{
	int byte_counter = 0;
	unsigned int i;
	int ret_val;

	LOGGERPRIMITIVES_LOCKFILE(file_handle);
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
		ret_val = fprintf(file_handle, "%s    %s:    :    PRI=%d\n", time_stamp, keyword, priority);

	}
	else if(NULL == keyword)
	{
		ret_val = fprintf(file_handle, "%s    :    %s:    PRI=%d\n", time_stamp, subkeyword, priority);
	}
	else
	{		
		ret_val = fprintf(file_handle, "%s    %s:    %s:    PRI=%d\n", time_stamp, keyword, subkeyword, priority);
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
 * This function prints the header of the log entry.
 * The header includes the timestamp, keyword, subkeyword and priority.
 * It also prints the number of pre_new_lines required.
 * @warning This locks the file handle, but doesn't unlock it.
 * Use the PrintFooter to properly unlock the file handle.
 * This function does not check to see if 
 * the file_handle is NULL.
 * @return The number of bytes printed.
 */
int LoggerPrimitives_PrintHeaderWithCustom(Logger* logger, 
	unsigned int pre_new_lines,
	unsigned int priority,
	const char* time_stamp, 
	const char* keyword, 
	const char* subkeyword
)
{
	int byte_counter = 0;
	unsigned int i;
	int ret_val;

	/* Note: File Locking doesn't make sense here. */
	if(pre_new_lines > 0)
	{
		for(i=0; i<pre_new_lines; i++)
		{
			/* customPuts should return the number of bytes written since it is impossible to know what it actually wrote. */
			ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, "\n");
			if(ret_val < 0)
			{
				/* Safe to return early here, but remember 
				 * that UNLOCKFILE is called in the footer. */
				return ret_val;
			}
			/*
			byte_counter += sizeof("\n");
			*/
			byte_counter += ret_val;
		}
	}
	
	/* YYYY-MM-DD_HH:MM:SS.mmm    keyword:    subkeyword:    PRI=#
	 * Note 4 spaces between words
	 * 2002-10-29_13:00:00.000    Sample:    Comment:    PRI=0
	 */
	if( (NULL == keyword) && (NULL == subkeyword) )
	{
		ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPrintf(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, "%s    :    :    PRI=%d\n", 
			time_stamp, priority);
	}
	else if(NULL == subkeyword)
	{
		ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPrintf(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, "%s    %s:    :    PRI=%d\n", time_stamp, keyword, priority);

	}
	else if(NULL == keyword)
	{
		ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPrintf(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, "%s    :    %s:    PRI=%d\n", time_stamp, subkeyword, priority);
	}
	else
	{		
		ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPrintf(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, "%s    %s:    %s:    PRI=%d\n", time_stamp, keyword, subkeyword, priority);
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
int LoggerPrimitives_PrintFooterToFileHandle(FILE* file_handle, 
	unsigned int post_new_lines,
	int auto_flush_enabled)
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
	LOGGERPRIMITIVES_UNLOCKFILE(file_handle);
	if(error_flag == 0)
	{
		return byte_counter;
	}
	else
	{
		return error_flag;
	}
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
int LoggerPrimitives_PrintFooterWithCustom(Logger* logger, 
	unsigned int priority, const char* keyword, const char* subkeyword,
	unsigned int post_new_lines,
	int auto_flush_enabled)
{
	int byte_counter = 0;
	unsigned int i;
	int ret_val;
	int error_flag = 0;

	error_flag = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, LOGGER_LOGENDTOKEN);
	if(error_flag >= 0)
	{
		/* customPuts should return the number of bytes written since it is impossible to know what it actually wrote. */
		/*
		byte_counter += sizeof(LOGGER_LOGENDTOKEN);
		*/
		byte_counter += error_flag;		
	}
	
	if(post_new_lines > 0)
	{
		for(i=0; i<post_new_lines; i++)
		{
			ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, "\n");
			/* Try to record/preserve the first error flag because subsequent 
			 * messages might be less helpful. */
			if(ret_val >= 0)
			{
				/*
				byte_counter += sizeof("\n");
				*/
				byte_counter += ret_val;		
			}
			else if(0 == error_flag)
			{
				error_flag = ret_val;
			}
		}
	}
	/* Note: AutoFlush and File Unlocking don't make sense here. */
	if(error_flag == 0)
	{
		return byte_counter;
	}
	else
	{
		return error_flag;
	}
}



