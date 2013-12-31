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
#include "TimeStamp.h"

#include <stdlib.h> /* For malloc */
#include <string.h>
#include <errno.h> /* Used for fopen error. */

/* Visual Studio doesn't define snprintf but _snprintf */
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

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

/* This should be something like ".%04d". This particular 
 * string will be used in an snprintf and be used to 
 * format the log segment extension with 0 padding if needed.
 * .0001, .0002, .0003, etc.
 */
#define LOGGER_DEFAULT_SEGMENT_FORMAT_STRING ".%04d"
/* This MUST be larger than the length of the default format string
 * (including \0) or the program will do bad things on MallocAndInit()
 */
#define LOGGER_DEFAULT_SEGMENT_FORMAT_STRING_SIZE 10


/* This is a fast path, expecting segments to look like .0001 (6 chars)
 * I suppose I could just make the number big so dynamic memory is 
 * rarely used, but how big?
 */
#define LOGGER_PREALLOCATED_SEGMENT_CHARS 8


/* Kind of random. I probably could be smarter about this if
 * I bothered to do research on what the longest file names 
 * (popular) operating systems allow for. I think I saw the
 * number 1022 for OS X floating around. Remember that 
 * everything is Unicode on OS X, so the number of characters
 * is probably div2 or div4. But since this library is C based,
 * I need to specify the byte number.
 */
#define LOGGER_PREALLOCATED_COPY_NAME 1024

/* I know my time stamp library always returns a string of size 24 */
#define LOGGER_TIME_STAMP_SIZE 24


typedef struct
{
	/** Unused. But I want the struct to have at least one 
	 * data field to avoid any potential malloc(0) issues
	 * if I decided to make the mutex conditional.
	 */
	int data1;
	void* loggerMutex;

} LoggerOpaqueData;

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
		static int Logger_LockMutex(Logger* logger)
		{
			return(
				WaitForSingleObject(
					(HANDLE)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex,
					INFINITE
				) != WAIT_FAILED
			);
		}
		static void Logger_UnlockMutex(Logger* logger)
		{
			ReleaseMutex(
				(HANDLE)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex
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
		#define LOGGER_CREATEMUTEX() Logger_CreateMutex()
		#define LOGGER_DESTROYMUTEX(X) Logger_DestroyMutex((X))

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
		static int Logger_LockMutex(Logger* logger)
		{
			return(
				pthread_mutex_lock(
					(pthread_mutex_t*)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex
				) == 0
			);
		}
		static void Logger_UnlockMutex(Logger* logger)
		{
			pthread_mutex_unlock(
				(pthread_mutex_t*)((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex
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

		#define LOGGER_CREATEMUTEX() Logger_CreateMutex()
		#define LOGGER_DESTROYMUTEX(X) Logger_DestroyMutex((X))

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

#if 0
/* Currently not used or implemented. See the warning on
 * the main page about the crappy internal error handling 
 * to understand why I haven't finished implementing this.
 */
const char* Logger_GetErrorString(int error_val)
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
static char* Logger_CopyDynamicString(char* target_string, size_t* target_max_buffer_size, const char* source_string)
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
static Logger* Logger_MallocAndInit()
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
	((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex = LOGGER_CREATEMUTEX();
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
	((LoggerOpaqueData*)logger->opaqueLoggerData)->data1 = 0;


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
static int Logger_OpenFile(Logger* logger, const char* file_name)
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
static int Logger_CloseFile(Logger* logger)
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
static int Logger_SegmentFile(Logger* logger)
{
	/* I'm thinking it 1 (dot) + 4 (0001) + 1 (for '\0') */
	char segment_ext[LOGGER_PREALLOCATED_SEGMENT_CHARS];
	char next_file_string[LOGGER_PREALLOCATED_COPY_NAME + LOGGER_PREALLOCATED_SEGMENT_CHARS];
	/* This is the total number of characters for the extension */
	int total_num_ext_chars;
	size_t copy_string_size;
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
	Logger_CloseFile(logger);
	
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
			fprintf(stderr, "Error in Logger_SegmentFile(). Could not allocate memory");
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
				fprintf(stderr, "Error in Logger_SegmentFile(). Could not allocate memory");
				/* Need to free memory */
				free(dynamic_seg_ext);
				return 0;
			}

			/* Copy the base name over */
			strcpy(dynamic_copy_string, logger->baseName);
			/* Append the extension */
			strcat(dynamic_copy_string, dynamic_seg_ext);

			ret_flag = Logger_OpenFile(logger, dynamic_copy_string);
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

			ret_flag = Logger_OpenFile(logger, next_file_string);
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
				fprintf(stderr, "Error in Logger_SegmentFile(). Could not allocate memory");
				return 0;
			}

			/* Copy the base name over */
			strcpy(dynamic_copy_string, logger->baseName);
			/* Append the extension */
			strcat(dynamic_copy_string, segment_ext);

			ret_flag = Logger_OpenFile(logger, dynamic_copy_string);
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

			ret_flag = Logger_OpenFile(logger, next_file_string);
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



Logger* Logger_Create()
{
	Logger* logger;

	logger = Logger_MallocAndInit();
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

	logger = Logger_MallocAndInit();
	if(NULL == logger)
	{
		return NULL;
	}

	/* Logger_OpenFile will open the file,
	 * and set the logger variables to 
	 * the correct values.
	 */
	if(!Logger_OpenFile(logger, base_name))
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

	logger = Logger_MallocAndInit();
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

	LOGGER_LOCKMUTEX(logger);

	if(NULL == file_handle)
	{
		LOGGER_UNLOCKMUTEX(logger);
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
			LOGGER_UNLOCKMUTEX(logger);
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
		LOGGER_UNLOCKMUTEX(logger);
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
		LOGGER_UNLOCKMUTEX(logger);
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

	LOGGER_UNLOCKMUTEX(logger);
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
	Logger_CloseFile(logger);

#ifdef LOGGER_ENABLE_LOCKING
	if(NULL != ((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex)
	{
		LOGGER_DESTROYMUTEX( ((LoggerOpaqueData*)logger->opaqueLoggerData)->loggerMutex );
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
static int Logger_PrintHeaderToFileHandle(FILE* file_handle, 
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
 * This function prints the header of the log entry.
 * The header includes the timestamp, keyword, subkeyword and priority.
 * It also prints the number of pre_new_lines required.
 * @warning This locks the file handle, but doesn't unlock it.
 * Use the PrintFooter to properly unlock the file handle.
 * This function does not check to see if 
 * the file_handle is NULL.
 * @return The number of bytes printed.
 */
static int Logger_PrintHeaderWithCustom(Logger* logger, 
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
			ret_val = logger->customPuts(logger, logger->customCallbackUserData, "\n");
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
		ret_val = logger->customPrintf(logger, logger->customCallbackUserData, "%s    :    :    PRI=%d\n", 
			time_stamp, priority);
	}
	else if(NULL == subkeyword)
	{
		ret_val += logger->customPrintf(logger, logger->customCallbackUserData, "%s    %s:    :    PRI=%d\n", time_stamp, keyword, priority);

	}
	else if(NULL == keyword)
	{
		ret_val += logger->customPrintf(logger, logger->customCallbackUserData, "%s    :    %s:    PRI=%d\n", time_stamp, subkeyword, priority);
	}
	else
	{		
		ret_val += logger->customPrintf(logger, logger->customCallbackUserData, "%s    %s:    %s:    PRI=%d\n", time_stamp, keyword, subkeyword, priority);
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
static int Logger_PrintFooterToFileHandle(FILE* file_handle, 
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



/**
 * This prints the footer of a log entry.
 * The footer includes the END: marker and the 
 * number of post-new-lines required.
 * @warning This unlocks a file that was presumably left locked by
 * the PrintHeader call. This function does not check to see if 
 * the file_handle is NULL.
 * @return The number of bytes written for the footer.
 */
static int Logger_PrintFooterWithCustom(Logger* logger, 
	unsigned int post_new_lines,
	int auto_flush_enabled)
{
	int byte_counter = 0;
	unsigned int i;
	int ret_val;
	int error_flag = 0;

	error_flag = logger->customPuts(logger, logger->customCallbackUserData, LOGGER_LOGENDTOKEN);
	if(error_flag >= 0)
	{
		byte_counter += sizeof(LOGGER_LOGENDTOKEN);
	}
	
	if(post_new_lines > 0)
	{
		for(i=0; i<post_new_lines; i++)
		{
			ret_val = logger->customPuts(logger, logger->customCallbackUserData, "\n");
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

#define LOGGER_USE_CUSTOM_PRINT(logger) (NULL != logger->customPuts)

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
			if(logger->echoOn && (NULL != logger->echoHandle))
			{
				VA_COPY(echo_copy, argp);
			}
			if(0 == use_custom_print)
			{
				ret_val = vfprintf(logger->fileHandle, text, argp);
			}
			else
			{
				ret_val = logger->customPrintfv(logger, logger->customCallbackUserData, text, argp);
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
			ret_val = vfprintf(logger->echoHandle, text, echo_copy);
			if((ret_val < 0) && (0 == error_flag))
			{
				error_flag = ret_val;
			}

			VA_COPYEND(echo_copy);
		}
		
		ret_val = Logger_PrintFooterToFileHandle(logger->echoHandle,
			logger->postNewLines,
			logger->autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}

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
	
	
	/* if segmentation is enabled. */
	if(logger->minSegmentBytes != 0)
	{
		if(logger->currentByteCount > logger->minSegmentBytes)
		{
			/* SegmentFile() will just return if logger->fileHandle
			 * is not a file
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
				ret_val = fputs(text, logger->fileHandle);
			}
			else
			{
				ret_val = logger->customPuts(logger, logger->customCallbackUserData, text);
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
		ret_val = Logger_PrintHeaderToFileHandle(logger->echoHandle,
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
		
		ret_val = Logger_PrintFooterToFileHandle(logger->echoHandle,
			logger->postNewLines,
			logger->autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}	
		
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

void Logger_Enable(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->loggingEnabled = 1;
	LOGGER_UNLOCKMUTEX(logger);
}

void Logger_Disable(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->loggingEnabled = 0;
	LOGGER_UNLOCKMUTEX(logger);
}

int Logger_IsEnabled(Logger* logger)
{
	int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->loggingEnabled;
	LOGGER_UNLOCKMUTEX(logger);
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
	LOGGER_LOCKMUTEX(logger);
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
	LOGGER_UNLOCKMUTEX(logger);

	return error_flag;
}

void Logger_EnableAutoFlush(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->autoFlushEnabled = 1;
	LOGGER_UNLOCKMUTEX(logger);
}

void Logger_DisableAutoFlush(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->autoFlushEnabled = 0;
	LOGGER_UNLOCKMUTEX(logger);
}

int Logger_IsAutoFlushEnabled(Logger* logger)
{
	int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->autoFlushEnabled;
	LOGGER_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetThresholdPriority(Logger* logger, unsigned int thresh_val)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	if(0 == thresh_val)
	{
		logger->thresholdPriority = LOGGER_DEFAULT_THRESHOLD;
	}
	else
	{
		logger->thresholdPriority = thresh_val;
	}
	LOGGER_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetThresholdPriority(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->thresholdPriority;
	LOGGER_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetDefaultPriority(Logger* logger, unsigned int pri_level)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	if(0 == pri_level)
	{
		logger->defaultPriority = LOGGER_DEFAULT_PRIORITY;
	}
	else
	{
		logger->defaultPriority = pri_level;
	}
	LOGGER_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetDefaultPriority(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->defaultPriority;
	LOGGER_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetMinSegmentSize(Logger* logger, unsigned int num_bytes)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->minSegmentBytes = num_bytes;
	LOGGER_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetMinSegmentSize(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->minSegmentBytes;
	LOGGER_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_EchoOn(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->echoOn = 1;
	LOGGER_UNLOCKMUTEX(logger);
}

void Logger_EchoOff(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->echoOn = 0;
	LOGGER_UNLOCKMUTEX(logger);
}

int Logger_IsEchoOn(Logger* logger)
{
	int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->echoOn;
	LOGGER_UNLOCKMUTEX(logger);
	return ret_val;
}

void Logger_SetEchoToStdout(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->echoHandle = stdout;
	LOGGER_UNLOCKMUTEX(logger);
}

void Logger_SetEchoToStderr(Logger* logger)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->echoHandle = stderr;
	LOGGER_UNLOCKMUTEX(logger);
}

void Logger_SetPreLines(Logger* logger, unsigned int pre_lines)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->preNewLines = pre_lines;
	LOGGER_UNLOCKMUTEX(logger);
}

void Logger_SetPostLines(Logger* logger, unsigned int post_lines)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->postNewLines = post_lines;
	LOGGER_UNLOCKMUTEX(logger);
}

void Logger_SetNewLines(Logger* logger, unsigned int prelines, unsigned int postlines)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	logger->preNewLines = prelines;
	logger->postNewLines = postlines;
	LOGGER_UNLOCKMUTEX(logger);
}

unsigned int Logger_GetPreLines(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->preNewLines;
	LOGGER_UNLOCKMUTEX(logger);
	return ret_val;
}

unsigned int Logger_GetPostLines(Logger* logger)
{
	unsigned int ret_val;
	if(NULL == logger)
	{
		return 0;
	}
	LOGGER_LOCKMUTEX(logger);
	ret_val = logger->postNewLines;
	LOGGER_UNLOCKMUTEX(logger);
	return ret_val;
}

int Logger_WillLog(Logger* logger, unsigned int priority)
{
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
	LOGGER_UNLOCKMUTEX(logger);
	return 1;
}

void Logger_SetSegmentFormatString(Logger* logger, const char* format_string)
{
	if(NULL == logger)
	{
		return;
	}
	LOGGER_LOCKMUTEX(logger);
	if(NULL == format_string)
	{
		/* Reset the system to use the default format string. */
		format_string = LOGGER_DEFAULT_SEGMENT_FORMAT_STRING;
	}

	logger->segmentFormatString = Logger_CopyDynamicString(
		logger->segmentFormatString, 
		&logger->segmentFormatStringMaxSize, 
		format_string
	);
	LOGGER_UNLOCKMUTEX(logger);
}
