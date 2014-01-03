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
#ifndef LOGGER_PRIMITIVES_H
#define LOGGER_PRIMITIVES_H


/** Windows needs to know explicitly which functions to export in a DLL. */
/* TODO: Make exporting LOGGERPRIMITIVES optional */
#ifdef LOGGER_BUILD_AS_DLL
	#ifdef WIN32
		#define LOGGERPRIMITIVES_EXPORT __declspec(dllexport)
	#elif defined(__GNUC__) && __GNUC__ >= 4
		#define LOGGERPRIMITIVES_EXPORT __attribute__ ((visibility("default")))
	#else
		#define LOGGERPRIMITIVES_EXPORT
	#endif
#else
	#define LOGGERPRIMITIVES_EXPORT
#endif /* LOGGER_BUILD_AS_DLL */


#include "Logger.h"
#include <stdlib.h> /* For malloc */
#include <string.h>
#include <errno.h> /* Used for fopen error. */
#include <stdarg.h>

/* Visual Studio doesn't define snprintf but _snprintf */
#ifdef _MSC_VER
	#ifndef snprintf
		#define snprintf _snprintf
	#endif
#endif

/* For echoing, I need va_copy or the undefined system behavior to work 
 * like va_copy. Otherwise, echoing will not work and you will get the 
 * wrong values echoed in your format strings.
 */
#ifndef va_copy
	#ifdef __va_copy
		#define LOGGER_VA_COPY(a, b) __va_copy(a, b)
		#define LOGGER_VA_COPYEND(a) va_end(a)
	#else
		/* This is a fallback that people claim most compilers 
		 * without va_copy will support. If not, you need to 
		 * disable echoing in the code. 
		 * If you simply get a type casting error, you might also 
		 * try modifying the code to not use va_copy but instead
		 * directly reuse the argp variable. This actually worked for
		 * me before I discovered I needed va_copy on some systems.
		 */
		#define LOGGER_VA_COPY(a, b) ((a) = (b))
		#define LOGGER_VA_COPYEND(a) 
	#endif
#else
	#define LOGGER_VA_COPY(a, b) va_copy(a, b)
	#define LOGGER_VA_COPYEND(a) va_end(a)
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
/*	int data1; */
	void* loggerMutex;

    int (*customPuts)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text);
	int (*customPrintf)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...);
    int (*customPrintfv)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp);
	void* customCallbackUserData;

} LoggerOpaqueData;

#define LOGGER_USE_CUSTOM_PRINT(logger) (NULL != ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts)

/* This is for basic thread safety. I'm on the fence about if I want 
 * to deal with multi-process safety right now, but I suppose if 
 * I did, I would add/remove an flock/funlock to everything.
 * This is not tested on Windows. This whole thing should be
 * considered experimental.
 */
#ifdef LOGGER_ENABLE_LOCKING

		extern LOGGERPRIMITIVES_EXPORT void* LOGGER_CALLCONVENTION LoggerPrimitives_CreateMutex(void);
		extern LOGGERPRIMITIVES_EXPORT void LOGGER_CALLCONVENTION LoggerPrimitives_DestroyMutex(void* mutex);
		/* This will return true if locking is successful, false if not.
		 */
		extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_LockMutex(Logger* logger);
		extern LOGGERPRIMITIVES_EXPORT void LOGGER_CALLCONVENTION LoggerPrimitives_UnlockMutex(Logger* logger);
		/* Wow, I'm really screwed. There is no way to go from 
		 * an ANSI FILE* to a Microsoft HANDLE.
		 */
		/*
		void Logger_LockFile(FILE* file_handle);
		void Logger_UnlockFile(FILE* file_handle);
		*/
		#define LOGGERPRIMITIVES_CREATEMUTEX() LoggerPrimitives_CreateMutex()
		#define LOGGERPRIMITIVES_DESTROYMUTEX(X) LoggerPrimitives_DestroyMutex((X))

		#define LOGGERPRIMITIVES_LOCKMUTEX(X) LoggerPrimitives_LockMutex((X))
		#define LOGGERPRIMITIVES_UNLOCKMUTEX(X) LoggerPrimitives_UnlockMutex((X))
#else
	/* Basically, I want to turn everything here into no-op's */
	#define LOGGERPRIMITIVES_LOCKMUTEX(X)
	#define LOGGERPRIMITIVES_UNLOCKMUTEX(X)

#endif
	
/* Originally, I thought I needed this, but I don't think I really do and isn't even desirable.
 * Also, the Windows implementation doesn't support flockfile or converting 
 * stdio file handles to native so the implementation would require rework
 * that doesn't seem worth doing.
 * But if anybody needs it, I left some of the infrastructure in place.
 */
#ifdef LOGGER_ENABLE_FILELOCKING
	#warning "File locking disabled because the Windows side is not implemented."
	#define LOGGERPRIMITIVES_LOCKFILE(X)
	#define LOGGERPRIMITIVES_UNLOCKFILE(X)
#else
	#define LOGGERPRIMITIVES_LOCKFILE(X)
	#define LOGGERPRIMITIVES_UNLOCKFILE(X)
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
extern LOGGERPRIMITIVES_EXPORT char* LOGGER_CALLCONVENTION LoggerPrimitives_CopyDynamicString(char* target_string, size_t* target_max_buffer_size, const char* source_string);


/**
 * Private function to malloc a new logger instance
 * and initialize its values to default settings.
 * @return Returns a pointer to a new logger instance
 * with all its fields initialized to default values.
 * On malloc failure, it will return NULL.
 */
extern LOGGERPRIMITIVES_EXPORT Logger* LOGGER_CALLCONVENTION LoggerPrimitives_MallocAndInit(void);

/**
 * Opens a new file.
 * This is a private function for logger to open a new file 
 * and save the file handle to it's private member data.
 * @param logger The pointer to the logger instance.
 * @param file_name The name of the file to create.
 * @return Returns 1 on success, 0 on error. A statement
 * is currently printed to stderr on error.
 */
extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_OpenFile(Logger* logger, const char* file_name);

/**
 * Closes the current logger fileHandle.
 * Currently, SegmentFile depends on the behavior
 * that if fileHandle is NULL, the function returns 1.
 * The logger instance will get it's fileHandle set to NULL.
 * @param logger The pointer to the logger instance.
 * @return Returns 1 on success, 0 on error. A statement
 * is currently printed to stderr on error.
 */
extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_CloseFile(Logger* logger);

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
extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_SegmentFile(Logger* logger);

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
extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_PrintHeaderToFileHandle(FILE* file_handle,
	unsigned int pre_new_lines,
	unsigned int priority,
	const char* time_stamp, 
	const char* keyword, 
	const char* subkeyword);


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
extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_PrintHeaderWithCustom(Logger* logger,
	unsigned int pre_new_lines,
	unsigned int priority,
	const char* time_stamp, 
	const char* keyword, 
	const char* subkeyword
);


/**
 * This prints the footer of a log entry.
 * The footer includes the END: marker and the 
 * number of post-new-lines required.
 * @warning This unlocks a file that was presumably left locked by
 * the PrintHeader call. This function does not check to see if 
 * the file_handle is NULL.
 * @return The number of bytes written for the footer.
 */
extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_PrintFooterToFileHandle(FILE* file_handle,
	unsigned int post_new_lines,
	int auto_flush_enabled);


/**
 * This prints the footer of a log entry.
 * The footer includes the END: marker and the 
 * number of post-new-lines required.
 * @warning This unlocks a file that was presumably left locked by
 * the PrintHeader call. This function does not check to see if 
 * the file_handle is NULL.
 * @return The number of bytes written for the footer.
 */
extern LOGGERPRIMITIVES_EXPORT int LOGGER_CALLCONVENTION LoggerPrimitives_PrintFooterWithCustom(Logger* logger,
	unsigned int priority, const char* keyword, const char* subkeyword,
	unsigned int post_new_lines,
	int auto_flush_enabled);


#endif /* LOGGER_PRIMITIVES_H */

