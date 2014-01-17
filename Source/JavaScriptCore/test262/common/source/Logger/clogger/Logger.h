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

#ifndef LOGGER_H
#define LOGGER_H

/**
 * @file
 * 
 * Logger is a general purpose logging system for events, simulation,
 * errors, etc. It provides a standard formated output to allow for 
 * easy parsing (for automation). It also allows the for the filtering
 * of log entries (by priority) and the easy redirection of where the 
 * output goes (file, stdout, stderr).
 * Logger also provides a segmentation feature which will auto-split
 * the logs if the files grow beyond a specified size.
 * 
 * The output format is:
 * 
 * @code
 * YYYY-MM-DD_HH:MM:SS.mmm    Keyword:    Subkeyword:    PRI=n
 * User text goes here.
 * Could be multiple lines.
 * ^@END:$
 * @endcode
 * 
 * Sample Usage:
 * @code
 * Logger* logger = Logger_CreateWithName("myoutput.log");
 * Logger_LogEvent(logger, 5, "Error", "SDL_image::SDL_glLoadImage",
 * 		"Error found: %s", SDL_GetError());
 * Logger_Free(logger);
 * @endcode
 *
 * Sample Output:
 * @code
 * 2002-10-29_19:59:32.321    Error:    SDL_image::SDL_glLoadImage():    PRI=5
 * Error found: Error in SDL_GLhelper. Unsupported bit depth(=2)
 * ^@END:$
 * @endcode
 * 
 * @note This is a C adaptation of a version of my logging system written in 
 * C++ which was an adaptation from one I wrote in Perl.
 * This version attempts to provide (very) basic thread safety if the #define's 
 * are enabled. It will lock logger member variables when accessed and lock
 * the output file when writing to. The user must still ensure that the 
 * strings and other data passed into logger are properly locked. There is 
 * also currently no safety for creating and/or destroying instances 
 * of Logger while trying to use it, so don't create or destroy when 
 * there is possible contention.
 * Further optimization probably could be achieved by providing finer 
 * grain locks and smarter locking procedures.
 *
 * @warning Logger internal error reporting sucks (which is somewhat ironic).
 * I currently either silently fail or print directly to stderr.
 * I might want to do a string based method, but would need to make
 * it thread safe. This means I would need to create a map that 
 * placed a different error message in a different thread. Of course,
 * that requires more work as the thread APIs are not universal.
 * Perhaps returning an error number with a system that converts 
 * the error number would be easier, but gets hard to deal with when
 * return values are already being used. 
 *
 * @author Eric Wing
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> /* For FILE* */
#include <stdarg.h> /* for va_list */

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */

/* Note: For Doxygen to produce clean output, you should set the 
 * PREDEFINED option to remove LOGGER_EXPORT, LOGGER_CALLCONVENTION, and
 * the DOXYGEN_SHOULD_IGNORE_THIS blocks.
 * PREDEFINED = DOXYGEN_SHOULD_IGNORE_THIS=1 LOGGER_EXPORT= LOGGER_CALLCONVENTION=
 */

/** Windows needs to know explicitly which functions to export in a DLL. */
#ifdef LOGGER_BUILD_AS_DLL
	#ifdef WIN32
		#define LOGGER_EXPORT __declspec(dllexport)
	#elif defined(__GNUC__) && __GNUC__ >= 4
		#define LOGGER_EXPORT __attribute__ ((visibility("default")))
	#else
		#define LOGGER_EXPORT
	#endif
#else
	#define LOGGER_EXPORT
#endif /* LOGGER_BUILD_AS_DLL */
	

/* For Windows, by default, use the C calling convention */
#ifndef LOGGER_CALLCONVENTION
	#ifdef WIN32
		#define LOGGER_CALLCONVENTION __cdecl
	#else
		#define LOGGER_CALLCONVENTION
	#endif
#endif /* LOGGER_CALLCONVENTION */

/**
 * This is the end marker of a log entry.
 * Customize this to change your end token.
 */
#define LOGGER_LOGENDTOKEN "\n^@END:$\n"
	
/* Version number is set here.
 * Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
 */
#define LOGGER_MAJOR_VERSION		0
#define LOGGER_MINOR_VERSION		4
#define LOGGER_PATCH_VERSION		0

/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */

/**
 * Struct that contains the version information of this library.
 * This represents the library's version as three levels: major revision
 * (increments with massive changes, additions, and enhancements),
 * minor revision (increments with backwards-compatible changes to the
 * major revision), and patchlevel (increments with fixes to the minor
 * revision).
 * @see LOGGER_GET_COMPILED_VERSION, Logger_GetLinkedVersion
 */
typedef struct
{
	int major; /**< major revision. */
	int minor; /**< minor revision. */
	int patch; /**< patch revision. */
} LoggerVersion;


/**
 * This macro fills in a Logger_Version structure with the version of the
 * library you compiled against. This is determined by what header the
 * compiler uses. Note that if you dynamically linked the library, you might
 * have a slightly newer or older version at runtime. That version can be
 * determined with Logger_GetLinkedVersion(), which, unlike 
 * LOGGER_GET_COMPILED_VERSION, is not a macro.
 *
 * @param X A pointer to a Logger_Version struct to initialize.
 *
 * @see LoggerVersion, Logger_GetLinkedVersion
 */
#define LOGGER_GET_COMPILED_VERSION(X) 		\
{											\
	if(NULL != (X))							\
	{										\
		(X)->major = LOGGER_MAJOR_VERSION;	\
		(X)->minor = LOGGER_MINOR_VERSION;	\
		(X)->patch = LOGGER_PATCH_VERSION;	\
	}										\
}

/**
 * Gets the library version of Logger you are using.
 * This gets the version of Logger that is linked against your program.
 * If you are using a shared library (DLL) version of Logger, then it is
 * possible that it will be different than the version you compiled against.
 *
 * This is a real function; the macro LOGGER_GET_COMPILED_VERSION tells 
 * you what version of Logger you compiled against:
 *
 * @code
 * LoggerVersion compiled;
 * LoggerVersion linked;
 *
 * LOGGER_GET_COMPILED_VERSION(&compiled);
 * Logger_GetLinkedVersion(&linked);
 * printf("We compiled against Logger version %d.%d.%d ...\n",
 *           compiled.major, compiled.minor, compiled.patch);
 * printf("But we linked against Logger version %d.%d.%d.\n",
 *           linked.major, linked.minor, linked.patch);
 * @endcode
 *
 * @see LoggerVersion, LOGGER_GET_COMPILED_VERSION
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_GetLinkedVersion(LoggerVersion* ver);

/**
 * Returns true if the library version of logger you are using was 
 * compiled with locking enabled.
 * If the library is compiled with locking enabled, then most functions 
 * will lock the logger instances while in use and lock the file handles
 * when appriopriate to provide some basic thread safety. This 
 * currently does not lock files for multiple processes.
 * @return Returns 1 if compiled with locking enabled. 0 if not.
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_IsCompiledWithLocking(void);

typedef struct Logger Logger;
/**
 * The Logger struct should be considered as your typical opaque datatype.
 * Use Logger_Create to create a new instance of Logger. Pass that instance 
 * pointer to every logger function. This is effectively Object-Oriented-C.
 * This probably should be all hidden inside a void*, but for now, I do 
 * this for clarity.
 */
struct Logger
{
	char* baseName; /**< Base file name of logging file. */
	size_t baseNameMaxSize; /**< Max size of baseName with \\0. */
	
	FILE* fileHandle; /**< File handle for log file. */
	FILE* echoHandle; /**< File handle for echo. */

	unsigned int thresholdPriority; /**< Min value events need to be printed. */
	unsigned int defaultPriority; /**< The value Pri=0 is interpreted as. */
	
	int echoOn; /**< Flag for echoing messages. */
	int loggingEnabled; /**< Flag for logging events. */
	int autoFlushEnabled; /**< Flag for autoflush. */

	unsigned int preNewLines; /**< Number of newlines before an entry. */
	unsigned int postNewLines; /**< Number of newlines after an entry. */

	unsigned int segmentNumber; /**< Segment number of current log file. */
	unsigned int currentByteCount; /**< Current byte count for this segment. */
	unsigned int minSegmentBytes; /**< Size needed before segmenting. */
	
	char* segmentFormatString; /**< Format str for ext like ".%04d". */
	size_t segmentFormatStringMaxSize; /**< Max size of the fmtstr with \\0. */

    int (*customPrintf)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...);
    int (*customPrintfv)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp);
    int (*customPuts)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text);
	void* customCallbackUserData;
	
	/** 
	 * This is reserved for future use. All data members might
	 * be moved in here in future revisions to make data private.
	 * Or, if new members are needed, but the ABI must be preserved,
	 * this can be used as a backdoor.
	 */
	void* opaqueLoggerData; 
	
};

/**
 * This creates a new logger instance.
 * This will create a new logger instance. All the logger values 
 * will be set to the defaults.
 * Currently the defaults are:
 * - fileHandle = stdout
 * - echoOn = false
 * - echoHandle = NULL
 * - minSegmentBytes = 0
 * - thresholdPriority = 1
 * - defaultPriority = 5
 * - preNewLines = 0
 * - postNewLines = 1
 * - loggingEnabled = 1
 * - autoFlushEnabled = 0
 * 
 * If you want to log to an actual file, you may be better off
 * calling Logger_CreateWithName or Logger_CreateWithHandle.
 * 
 * @return Returns a pointer to a Logger struct which is the 
 * instance variable (if successful) or NULL on failure.
 *
 * @see Logger_CreateWithName, Logger_CreateWithHandle
 */
extern LOGGER_EXPORT Logger* LOGGER_CALLCONVENTION Logger_Create(void);

/**
 * This creates a new logger instance connected to a file with the 
 * specified basename.
 * This creates a new logger instance with a log file by the specified 
 * basename. The basename would be something like "Mylog.log". The 
 * system will segment the logs if you specify a segment size. The segments
 * will take the name "Mylog.log.0001", etc. If the file already exists,
 * this function will try to overwrite it.
 *
 * @param base_name The file name you want for the starting log file.
 * Do not pass in NULL. (Use Logger_Create() instead.)
 * The logger instance will get make its own copy of the string so you are 
 * free to do what you will with your string.
 *
 * @return Returns a pointer to a Logger struct which is the 
 * instance variable (if successful) or NULL on failure.
 *
 * @see Logger_Create, Logger_CreateWithHandle, Logger_Free
 */
extern LOGGER_EXPORT Logger* LOGGER_CALLCONVENTION Logger_CreateWithName(const char* base_name);

/**
 * This creates a new logger instance connected to a preopened file handle.
 * This creates a new logger instance connected to a preopened file handle.
 * In general, you should not use the file handle for anything else after
 * you pass it to Logger. Logger will attempt to manage the file handle
 * and will close the file when segmentation or destruction occurs.
 *
 * @param file_handle A preopened file handle you want logger to log to.
 * This may alse be stdout or stderr.
 *
 * @param base_name The file name associated with the file handle you supply.
 * This information is needed by logger so it can determine what to name
 * the segmented parts if segmentation occurs. Passing a base_name like
 * MyLog.log would result in the system in creating a MyLog.log.0001 if
 * segmentation occured. 
 * Do not pass in NULL unless you are using 
 * stdout or stderr. (Use Logger_Create() instead.)
 * If you must use this function and if you don't have a file 
 * name for some reason, just put in a fake name and make sure segmentation 
 * is off.
 *
 *
 * The logger instance will get make its own copy of the string so you are 
 * free to do what you will with your string.
 * 
 * @return Returns a pointer to a Logger struct which is the 
 * instance variable (if successful) or NULL on failure.
 *
 * @see Logger_Create, Logger_CreateWithName, Logger_Free
 */
extern LOGGER_EXPORT Logger* LOGGER_CALLCONVENTION Logger_CreateWithHandle(
	FILE* file_handle,
	const char* base_name);

/**
 * This frees a logger instance.
 * This properly frees the memory of a logger instance created by the
 * Logger_Create*() family of functions. Whenever you create a Logger 
 * instance, you should always remember to balance it with a 
 * Logger_Free() call.
 *
 * @param logger A pointer to the logger instance you want to free.
 *
 * @see Logger_Create, Logger_CreateWithName, Logger_CreateWithHandle,
 * Logger_Free
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_Free(Logger* logger);


/**
 * This sets the file handle in an already created Logger instance.
 * This will set the file handle for logging in a precreated Logger instance.
 * This is provided as a convenience in case for some reason that you 
 * needed to allocate a logger first without a file, and later need
 * to set the file. You may also use this to set the handle to stdout
 * or stderr.
 * Changing a file handle after the first logging message has been sent
 * is not encouraged. It may or may not work. You need to be careful about 
 * the old file. Logger will attempt to close the old file before switching.
 * You need to be careful, particularly with threads.
 *
 * @param logger The pointer to the logger instance.
 *
 * @param file_handle A preopened file handle you want logger to log to.
 * This may also be stdout or stderr.
 *
 * @param base_name The file name associated with the file handle you supply.
 * This information is needed by logger so it can determine what to name
 * the segmented parts if segmentation occurs. Passing a base_name like
 * MyLog.log would result in the system in creating a MyLog.log.0001 if
 * segmentation occured. Do not pass in NULL unless you are using 
 * stdout or stderr. If you don't have a file 
 * name for some reason, put in a fake name and make sure segmentation 
 * is off.
 * The logger instance will get make its own copy of the string so you are 
 * free to do what you will with your string.
 * 
 * @return Returns 1 on success or 0 on error (e.g. memory allocation failure).
 *
 * @see Logger_Create, Logger_CreateWithName, Logger_CreateWithHandle
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_SetHandle(
	Logger* logger,
	FILE* file_handle,
	const char* base_name);
	
/**
 * This is the function that actually does the logging.
 * This function writes the actual log entries. The 
 * function usage is similar to fprintf (it actually calls
 * fprintf), but you have a few extra arguments to pass.
 * 
 * @param logger The logger instance you want to log with.
 *
 * @param priority The priority allows the entry to be filtered
 * by threshold. Higher numbers are higher priority. If the 
 * priority is higher than the logger's current threshold, the 
 * message will be printed
 * 1 = lowest (least important)
 * Highest will be determined by your team's conventions. 
 * For SClone, 5 = highest.
 * Priority 0 is a special value. 0 will automatically get remapped
 * to the logger's current Default Priorty. This gives you a little 
 * flexibilty if you want to be able to reassign a priority level later.
 *
 * @param keyword The keyword is the first identifying string to help
 * classification. The keyword is intended to help automated tools in
 * their parsing of logs. Your team should decide on a good convention
 * for keywords. Logger will always append a colon after the keyword. 
 * My recommendation (though not required) is that keywords are always
 * exactly one word with no spaces. Alpha-numeric is the 
 * best thing you can do. This helps make logs easy to parse.
 *
 * @param subkeyword Like the keyword, the subkeyword is another identifier
 * string to help you classify events. Once again, it is up to your
 * team to decide on a good set of subkeywords. Single word, alphanumeric
 * format is recommended. A colon will always be appended to the keyword.
 *
 * @param text This is where your message text goes. It is compatible with
 * printf style format strings. There is no imposed limit on how long 
 * the strings can be or what the format of your message should be. The 
 * only suggestion is that you should avoid having lines by itself that
 * look like the LOGGER_LOGENDTOKEN
 * as this is the marker that Logger uses to denote the end of a 
 * log event. (Parsing tools and people could get confused.)
 *
 * @param ... This is the variable argument list used to accomodate 
 * printf style format strings.
 *
 * @return Returns the number of bytes written to the log. This conforms 
 * with fprintf.
 *
 * @see Logger_SetThresholdPriority, Logger_SetDefaultPriority,
 * Logger_LogEventv, Logger_LogEventNoFormat
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_LogEvent(
	Logger* logger,
	unsigned int priority,
	const char* keyword, const char* subkeyword,
	const char* text, ...);

/**
 * This is the va_list version of Logger_LogEvent.
 * This is the va_list version of Logger_LogEvent in the case that you need it.
 * The same rules apply to this as with Logger_LogEvent.
 *
 * @see Logger_LogEvent
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_LogEventv(
	Logger* logger, 
	unsigned int priority,
	const char* keyword, const char* subkeyword,
	const char* text, va_list argp); 

/**
 * This is a "No Format Strings Allowed" version of LogEvent.
 * This version of LogEvent disallows the use of format strings.
 * This was written if you needed to expose the LogEvent function to
 * an untrusted source because it is quite easy to crash a system (or worse)
 * with an invalid format string. An untrusted source might include 
 * arguments passed through the command line, any user input that 
 * gets fed to Logger, or strings taken from runtime generated sources
 * like scripts.
 * I was probably being overly paranoid when I created this function,
 * and there may be ways to achive this level of safety without 
 * this function, but here it is anyway.
 * This function has nothing to do with format strings related to 
 * segmentation.
 * @see Logger_LogEvent
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_LogEventNoFormat(
	Logger* logger,
	unsigned int priority,
	const char* keyword, const char* subkeyword,
	const char* text);

/**
 * Enables logging output.
 * This will enable logging output so events are reported. 
 * By default this is on.
 *
 * @param logger The pointer to the logger instance.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_Enable(Logger* logger);

/**
 * Disables logging output.
 * This will disable logging output so events not are reported. 
 *
 * @param logger The pointer to the logger instance.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_Disable(Logger* logger);

/**
 * Returns true if logger is enabled.
 * Returns 1 or 0 if logger is enabled or disabled.
 * 
 * @param logger The pointer to the logger instance.
 * @return Returns 1 if enabled, 0 if disabled.
 * @see Logger_Enable, Logger_Disable
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_IsEnabled(Logger* logger);

/**
 * Flushes the file stream buffer.
 * This will call fflush on the current stream buffers.
 * 
 * @param logger The pointer to the logger instance.
 * @return Returns the value returned by fflush,
 * 0 on success, negative on error.
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_Flush(Logger* logger);

/**
 * Enables flushing of the buffers after every LogEvent.
 * This will call fflush() on the file handle after every LogEvent.
 * This will make sure that data is recorded immediately, with 
 * all the usual side effects associated with calling fflush().
 * By default, autoflush is disabled.
 *
 * @param logger The pointer to the logger instance.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_EnableAutoFlush(Logger* logger);

/**
 * Disables flushing of the buffers after every LogEvent.
 * This will allow the system to decide when a stream gets written.
 * This will generally give better performance, but if there is a system
 * crash, the last events to be recorded may not get written.
 * By default, autoflush is disabled.
 *
 * @param logger The pointer to the logger instance.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_DisableAutoFlush(Logger* logger);

/**
 * Returns true if autoflush is enabled.
 * Returns 1 or 0 if autoflush is enabled or disabled.
 * 
 * @param logger The pointer to the logger instance.
 * @return Returns 1 if enabled, 0 if disabled.
 * @see Logger_EnableAutoFlush, Logger_DisableAutoFlush
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_IsAutoFlushEnabled(Logger* logger);

/**
 * Sets the threshold priority.
 * This will set the threshold priority which all priority values 
 * are compared with to determine what gets printed.
 * Logger only prints entries with
 * Priority >= Threshold Priority
 * The default threshold = 1
 *
 * @param logger The pointer to the logger instance.
 * 
 * @param thresh_val The threshold value you desire. 
 * If you set it to 0, it will reset the threshold to the default value.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetThresholdPriority(Logger* logger, unsigned int thresh_val);

/**
 * Gets the currently set threshold value.
 * This returns the current threshold value.
 *
 * @param logger The pointer to the logger instance.
 * @return Returns the current threshold priority.
 * @see Logger_SetThresholdPriority
 */
extern LOGGER_EXPORT unsigned int LOGGER_CALLCONVENTION Logger_GetThresholdPriority(Logger* logger);

/**
 * Sets the level that Priority=0 gets mapped to.
 * This will treat any entries in LogEvent called with Priority 0,
 * as whatever you enter for this value (Default = 5)
 * So by default, if you enter 0 as the priority to LogEvent, the 
 * message will be treated as if you had entered priority=5.
 *
 * @param logger The pointer to the logger instance.
 *
 * @param pri_level The priority level you want 0-values to be mapped to.
 * If you set it to 0, it will reset the priorty to the default value.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetDefaultPriority(Logger* logger, unsigned int pri_level);

/**
 * Gets the currently set default priority.
 * This returns the current default priority.
 *
 * @param logger The pointer to the logger instance.
 * @return Returns the current default priority.
 * @see Logger_SetDefaultPriority
 */
extern LOGGER_EXPORT unsigned int LOGGER_CALLCONVENTION Logger_GetDefaultPriority(Logger* logger);

/**
 * Set the target size for how large a file may be before segmenting.
 * Logger will close the current file and create a new one after the 
 * number of bytes written for the file exceeds this number of bytes.
 * This is a soft limit. A log entry will not be divided between logs.
 * So any segmentation will occur after a log entry is complete.
 * 
 * @param logger The pointer to the logger instance.
 * 
 * @param num_bytes The number of bytes a file needs before segmenting.
 * The number 0 will disable the segmentation feature.
 * 0 is the default.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetMinSegmentSize(Logger* logger, unsigned int num_bytes);

/**
 * Gets the minimum segment size.
 * This returns the minimum size (in bytes) that is needed 
 * before a log will be segmented.
 *
 * @param logger The pointer to the logger instance.
 * @return Returns the minimum size (in bytes) that is needed 
 * before a log will be segmented.
 * @see Logger_SetMinSegmentSize
 */
extern LOGGER_EXPORT unsigned int LOGGER_CALLCONVENTION Logger_GetMinSegmentSize(Logger* logger);

/**
 * Enables echoing (to terminal).
 * This will enable echoing to the echo handle. Typically when you 
 * use this feature, you will want the file handle to be reporting 
 * to a file, and you would echo to stdout or stderr.
 * By default, echo is off.
 *
 * @param logger The pointer to the logger instance.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_EchoOn(Logger* logger);

/**
 * Disables echoing.
 * This will disable echoing to the echo handle. 
 * By default, echo is off.
 *
 * @param logger The pointer to the logger instance.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_EchoOff(Logger* logger);

/**
 * Returns true if Echo On is enabled.
 * Returns 1 if echo is on or 0 if echo is off.
 * 
 * @param logger The pointer to the logger instance.
 * @return Returns 1 if enabled, 0 if disabled.
 * @see Logger_EchoOn, Logger_EchoOff
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_IsEchoOn(Logger* logger);

/**
 * If echo is on, this will echo the output
 * to be written to stdout. If the file handle is 
 * also set to stdout, you will see the message appear
 * twice in the stdout stream.
 * This is the default echo handle.
 *
 * @param logger The pointer to the logger instance.
 * @see Logger_EchoOn, Logger_EchoOff, Logger_SetEchoToStderr
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetEchoToStdout(Logger* logger);

/**
 * If echo is on, this will echo the output
 * to be written to stderr. If the file handle is 
 * also set to stderr, you will see the message appear
 * twice in the stdout stream.
 *
 * @param logger The pointer to the logger instance.
 * @see Logger_EchoOn, Logger_EchoOff, Logger_SetEchoToStdout
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetEchoToStderr(Logger* logger);

/* Note: Future enhancements might include a customizable
 * echo handle. For now, this can be achieved through the backdoor
 * by altering the Logger struct echo handle directly, but 
 * this is unsupported. Keep in mind that echo handles will not
 * be segmented.
 */
   
/**
 * This sets the number of blank lines preceding a log entry.
 * This is the number of newlines that will be printed
 * right before a log entry.
 * The default is 0.
 * @param logger The pointer to the logger instance.
 * @param pre_lines The number of newlines to print before each log entry.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetPreLines(Logger* logger, unsigned int pre_lines);

/**
 * This sets the number of blank lines succeeding a log entry.
 * This is the number of newlines that will be printed
 * right after a log entry.
 * The default is 1.
 * @param logger The pointer to the logger instance.
 * @param post_lines The number of newlines to print after each log entry.
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetPostLines(Logger* logger, unsigned int post_lines);

/**
 * Sets the number of blank lines preceding a log entry and after a log entry.
 * This is a convenience function that allows you to set the number 
 * of pre_lines and post_lines in the same call.
 *
 * @param logger The pointer to the logger instance.
 * @param pre_lines The number of newlines to print before each log entry.
 * @param post_lines The number of newlines to print after each log entry.
 * @see Logger_SetPreLines, Logger_SetPostLines
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetNewLines(Logger* logger, unsigned int pre_lines, unsigned int post_lines);

/**
 * Gets the number of prelines.
 * This returns the currently set number of prelines.
 *
 * @param logger The pointer to the logger instance.
 * @return Returns the currently set number of prelines.
 * @see Logger_SetPreLines, Logger_SetNewLines
 */
extern LOGGER_EXPORT unsigned int LOGGER_CALLCONVENTION Logger_GetPreLines(Logger* logger);

/**
 * Gets the number of postlines.
 * This returns the currently set number of postlines.
 *
 * @param logger The pointer to the logger instance.
 * @return Returns the currently set number of postlines.
 * @see Logger_SetPostLines, Logger_SetNewLines
 */
extern LOGGER_EXPORT unsigned int LOGGER_CALLCONVENTION Logger_GetPostLines(Logger* logger);


/**
 * Returns true if logger will print with the specified priority.
 * This is a convenience function that will tell you if the priorty 
 * you submit will pass the threshold test. If the test passes, it means
 * a log event with that priorty would be printed.
 *
 * @param logger The pointer to the logger instance.
 * @param which_priority The priorty level you want to know if it will print.
 * @return Returns 1 if the priority you specified would pass the threshold
 * test and be logged. Returns 0 if it would not be printed.
 * @see Logger_SetThresholdPriority, Logger_GetThresholdPriority
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION Logger_WillLog(Logger* logger, unsigned int which_priority);


/**
 * Sets the format string that is used to create the segment file extensions.
 * When segmentation occurs, a new file must be created. The scheme is 
 * to take the base file name and append a numeric value represening the 
 * current segment number (starting with 0). 
 * 
 * The default format is ".%04d".
 * 
 * This will produce files like Mylog.log, Mylog.log.0000, Mylog.log.0001, etc.
 * 
 * Notice that the period is specified in that format string, so if you 
 * want something else in the segment, you may specify it.
 * 
 * You must have a %d somewhere in the format string. The underlying
 * implementation uses a snprintf(target, fmtstr, seg_number, maxsize),
 * so an integer will try to be inserted into the string.
 * 
 * You should also never change the format string after LogEvent* has 
 * been called, (i.e. don't change the file name schemes after segmentation 
 * may have begun). While this may actually work, it's not something that
 * was really thought about and won't be supported.
 *
 * In general, you should be very careful when using this function.
 *
 * This function will make it's own internal copy of the string 
 * you pass in, so you may do what you want with your string after this call.
 *
 * @param logger The pointer to the logger instance.
 *
 * @param format_string The format string (the kind used in the printf family).
 * NULL will reset the logger instance to use the default format.
 *
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetSegmentFormatString(Logger* logger, const char* format_string);


/**
 * Sets custom override functions for the internal fputs, fprintf, vfprintf.
 * This function is designed to allow overriding the internal 
 * fputs/fprintf/vfprintf calls used to print to the log.
 * This is intended for special cases where Logger is mostly sufficient, but needs
 * to be slightly tweaked.
 * The original case for this was for redirecting logging through a socket because
 * on Windows, fdopen does not work with a Windows socket so the stdio family doesn't work.
 * @warning Your custom fputs function must return the number of bytes written instead of what the normal fputs returns.
 * This is because it is impossible to know what your custom function actually wrote, so the API needs a feedback mechanism
 * since strlen() on the passed in string is insufficient. 
 * Failure to do this will break log segementation features as well as anybody checking return values.
 * If it is impossible to know the exact number (e.g. NSLog), a sensible number can still be useful (e.g. strlen)
 * to avoid completely breaking calling APIs.
 * @warning Features like autoflush (fflush) and segmentation may need to be 
 * disabled because they are not currently taken into account.
 * @note These functions do not override the echo to stdout/stderr prints.
 *
 *
 * @param logger The pointer to the logger instance.
 * @param custom_puts Override for fputs (but your fputs must return the number of bytes written or < 0 on error.
 * @param custom_printf Override for fprintf.
 * @param custom_printfv Override for vfprintf.
 * @param custom_callback_user_data Userdata that will be passed back to each callback. May be NULL.
 *
 * @note: All 3 must function pointers be defined or all must be NULL. Only overriding some is not supported.
 *
 */
extern LOGGER_EXPORT void LOGGER_CALLCONVENTION Logger_SetCustomPrintFunctions(Logger* logger,
	int (*custom_puts)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text),
	int (*custom_printf)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...),
    int (*custom_printfv)(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp),
	void* custom_callback_user_data
);


/*
 * A global instance pointer of logger.
 * Allow for a global instance of Logger so everybody can use
 * simply by including #include "Logger.h"
 * and defining in the global scope somewhere (like before main()).
 * Don't forget to call Create to actually create the instance.
 */
/* extern Logger* g_Logger; */



#ifdef __cplusplus
}
#endif
#endif /* LOGGER_H */

