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



#ifndef LOGGER_HPP
#define LOGGER_HPP

// For FILE* 
#include <cstdio>
#include <cstdarg> // for va_list
#include <string>

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */

/* Note: For Doxygen to produce clean output, you should set the 
 * PREDEFINED option to remove DECLSPEC, CALLCONVENTION, and
 * the DOXYGEN_SHOULD_IGNORE_THIS blocks.
 * PREDEFINED = DOXYGEN_SHOULD_IGNORE_THIS=1 DECLSPEC= CALLCONVENTION=
 */

/** Windows needs to know explicitly which functions to export in a DLL. */
#ifdef WIN32
	#define DECLSPEC __declspec(dllexport)
#else
	#define DECLSPEC
#endif

/* For Windows, by default, use the C calling convention */
#ifndef CALLCONVENTION
	#ifdef WIN32
		#define CALLCONVENTION __cdecl
	#else
		#define CALLCONVENTION
	#endif
#endif /* CALLCONVENTION */

/**
 * This is the end marker of a log entry.
 * Customize this to change your end token.
 */
#define LOGGER_LOGENDTOKEN "\n^@END:$\n"


/* Version number is set here.
 * Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
 */
#define LOGGER_MAJOR_VERSION		0
#define LOGGER_MINOR_VERSION		2
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
struct LoggerVersion
{
	int major; /**< major revision. */
	int minor; /**< minor revision. */
	int patch; /**< patch revision. */
};

/**
 * @file
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
 * Logger logger;
 * logger.Init("myoutput.log");
 * logger.LogEvent(5, "Error", "SDL_image::SDL_glLoadImage",
 * 		"Error found: %s", SDL_GetError());
 * logger.Close(); // Not necessary to explicitly call, called by destructor
 *
 * @endcode
 *
 * Sample Output:
 * @code
 * 2002-10-29_19:59:32.321    Error:    SDL_image::SDL_glLoadImage():    PRI=5
 * Error found: Error in SDL_GLhelper. Unsupported bit depth(=2)
 * ^@END:$
 * @endcode
 * 
 * @note This is a C++ adaptation of a version of my logging system written in 
 * Perl. It was then ported again to C and added basic thread safety. Those 
 * changes have been backported to this library along with changes 
 * to conform to our new coding and documentation guidelines.
 * This version continues to use standard C streams
 * instead of C++ streams because of performance and portability 
 * reasons as well as laziness issues. I'm actually unsure what a 
 * C++ stream interface would like like given that the different 
 * types of parameters (keywords, text) would effectively require
 * the >> operator to be aware of a state machine. 
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
/**
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
 * Logger logger;
 * logger.Init("myoutput.log");
 * logger.LogEvent(5, "Error", "SDL_image::SDL_glLoadImage",
 * 		"Error found: %s", SDL_GetError());
 * logger.Close(); // Not necessary to explicitly call, called by destructor
 *
 * @endcode
 *
 * Sample Output:
 * @code
 * 2002-10-29_19:59:32.321    Error:    SDL_image::SDL_glLoadImage():    PRI=5
 * Error found: Error in SDL_GLhelper. Unsupported bit depth(=2)
 * ^@END:$
 * @endcode
 * 
 * @note This is a C++ adaptation of a version of my logging system written in 
 * Perl. It was then ported again to C and added basic thread safety. Those 
 * changes have been backported to this library along with changes 
 * to conform to our new coding and documentation guidelines.
 * This version continues to use standard C streams
 * instead of C++ streams because of performance and portability 
 * reasons as well as laziness issues. I'm actually unsure what a 
 * C++ stream interface would like like given that the different 
 * types of parameters (keywords, text) would effectively require
 * the >> operator to be aware of a state machine. 
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
class DECLSPEC Logger
{
public:
	
	/**
	 * The default constructor for Logger. This creates 
	 * a new instance of Logger and calls Init.
	 * @see Init()
	 */
	Logger();

	/**
	 * This will initialize a logger instance. All the logger values 
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
	 * If an Init is called multiple times on an instance of logger,
	 * the system will attempt to close the file handle associated 
	 * with the logger and then try to reset everything. 
	 *
	 * If you want to log to an actual file, you may be better off
	 * directly calling the other Init functions.
	 * 
	 * @return Returns true on successful initialization, false on failure.
	 *
	 * @see Init(const std::string&), Init(FILE*,const std::string&) 
	 */
	bool Init();

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
	 * Do not pass in an empty string. (Use the default constructor instead.)
	 * The logger instance will get make its own copy of the string so you are 
	 * free to do what you will with your string.
	 *
	 * @return Returns true on successful initialization, false on failure.
	 *
	 * @see Init(), Init(FILE*,const std::string&) 
	 */
	bool Init(const std::string& base_name);

	/**
	 * This creates a new logger instance connected to a preopened file handle.
	 * This creates a new logger instance connected to a preopened file handle.
	 * In general, you should not use the file handle for anything else after
	 * you pass it to Logger. Logger will attempt to manage the file handle
	 * and will close the file when segmentation or destruction occurs.
	 * You can also use this function as a way to set the file handle to
	 * stderr (or stdout).
	 *
	 * @param file_handle A preopened file handle you want logger to log to.
	 * This may also be stderr or stdout.
	 *
	 * @param base_name The file name associated with the file handle you supply.
	 * This information is needed by logger so it can determine what to name
	 * the segmented parts if segmentation occurs. Passing a base_name like
	 * MyLog.log would result in the system in creating a MyLog.log.0001 if
	 * segmentation occured. 
	 * Do not pass in an empty string unless you are passing in stdout/stderr. 
	 * (Use Init() instead.)
	 * If you must use this function and if you don't have a file 
	 * name for some reason, just put in a fake name and make sure segmentation 
	 * is off.
	 *
	 * The logger instance will get make its own copy of the string so you are 
	 * free to do what you will with your string.
	 * 
	 * @return Returns true on successful initialization, false on failure.
	 *
	 * @see Init(), Init(const std::string&)
	 */
	bool Init(FILE* file_handle, const std::string& base_name);

	/**
	 * Destructor for Logger.
	 * This properly frees the memory of a logger instance.
	 * This will call Close() which closes the open file handles 
	 * (if any).
	 *
	 * @see Close
	 */
	~Logger();

	/**
	 * This will close any open log files that the logger instance 
	 * may have open. This function is intended to be used to shutdown
	 * a logger instance. In most cases, you won't ever have to call this
	 * function because the destructor calls this automatically. But
	 * there maybe a case that you want to close the active log file 
	 * because you may not be able to wait for the destructor to be 
	 * called. So this function is provided for those cases.
	 * @see ~Logger
	 */
	void Close();

	/**
	 * This inline method fills in a LoggerVersion structure with 
	 * the version of the library you compiled against. 
	 * This is determined by what header the
	 * compiler uses. Note that if you dynamically linked the library, you might
	 * have a slightly newer or older version at runtime. That version can be
	 * determined with GetLinkedVersion() which is defined in the
	 * implementation file and not here as an inline.
	 *
	 * @return Returns a structure containing the version information.
	 * @see LoggerVersion, GetLinkedVersion
	 */
	inline static LoggerVersion GetCompiledVersion()
	{
		LoggerVersion ver;
		ver.major = LOGGER_MAJOR_VERSION;
		ver.minor = LOGGER_MINOR_VERSION;
		ver.patch = LOGGER_PATCH_VERSION;
		return ver;
	}

	/**
	 * Gets the library version of Logger you are using.
	 * This gets the version of Logger that is linked against your program.
	 * If you are using a shared library (DLL) version of Logger, then it is
	 * possible that it will be different than the version you compiled against.
	 * To find the version you compiled against, use GetCompiledVersion.
	 *
	 * @code
	 * LoggerVersion compiled;
	 * LoggerVersion linked;
	 *
	 * compiled = Logger::GetCompiledVersion();
	 * linked = Logger::GetLinkedVersion();
	 * printf("We compiled against Logger version %d.%d.%d ...\n",
	 *           compiled.major, compiled.minor, compiled.patch);
	 * printf("But we linked against Logger version %d.%d.%d.\n",
	 *           linked.major, linked.minor, linked.patch);
	 * @endcode
	 *
	 * @see LoggerVersion, GetCompiledVersion
	 */
	static LoggerVersion GetLinkedVersion();

	/**
	 * Returns true if the library version of logger you are using was 
	 * compiled with locking enabled.
	 * If the library is compiled with locking enabled, then most functions 
	 * will lock the logger instances while in use and lock the file handles
	 * when appriopriate to provide some basic thread safety. This 
	 * currently does not lock files for multiple processes.
	 * @return Returns true if compiled with locking enabled. False if not.
	 */
	static bool IsCompiledWithLocking();
		
	/**
	 * This is the function that actually does the logging.
	 * This function writes the actual log entries. The 
	 * function usage is similar to fprintf (it actually calls
	 * fprintf), but you have a few extra arguments to pass.
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
	 * @see SetThresholdPriority, SetDefaultPriority,
	 * LogEventv, LogEventNoFormat
	 */
	int LogEvent(unsigned int priority, 
		const char* keyword, const char* subkeyword,
		const char* text, ...);

	/**
	 * This is the va_list version of Logger_LogEvent.
	 * This is the va_list version of Logger_LogEvent in the case that you need it.
	 * The same rules apply to this as with Logger_LogEvent.
	 *
	 * @see LogEvent
	 */
	int LogEventv(unsigned int priority, 
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
	 * @see LogEvent
	 */
	int LogEventNoFormat(unsigned int priority, 
		const char* keyword, const char* subkeyword,
		const char* text);

	/**
	 * Enables logging output.
	 * This will enable logging output so events are reported. 
	 * By default this is on.
	 */
	void Enable();

	/**
	 * Disables logging output.
	 * This will disable logging output so events not are reported. 
	 */
	void Disable();

	/**
	 * Returns true if logger is enabled.
	 * Returns true or false if logger is enabled or disabled.
	 * 
	 * @return Returns true if enabled, false if disabled.
	 * @see Enable, Disable
	 */
	bool IsEnabled() const;

	/**
	 * Flushes the file stream buffer.
	 * This will call fflush on the current stream buffers.
	 */
	int Flush();

	/**
	 * Enables flushing of the buffers after every LogEvent.
	 * This will call fflush() on the file handle after every LogEvent.
	 * This will make sure that data is recorded immediately, with 
	 * all the usual side effects associated with calling fflush().
	 * By default, autoflush is disabled.
	 */
	void EnableAutoFlush();

	/**
	 * Disables flushing of the buffers after every LogEvent.
	 * This will allow the system to decide when a stream gets written.
	 * This will generally give better performance, but if there is a system
	 * crash, the last events to be recorded may not get written.
	 * By default, autoflush is disabled.
	 */
	void DisableAutoFlush();

	/**
	 * Returns true if autoflush is enabled.
	 * Returns true or false if autoflush is enabled or disabled.
	 * 
	 * @return Returns true if enabled, false if disabled.
	 * @see EnableAutoFlush, DisableAutoFlush
	 */
	bool IsAutoFlushEnabled() const;

	/**
	 * Sets the threshold priority.
	 * This will set the threshold priority which all priority values 
	 * are compared with to determine what gets printed.
	 * Logger only prints entries with
	 * Priority >= Threshold Priority
	 * The default threshold = 1
	 *
	 * @param thresh_val The threshold value you desire. 
	 * If you set it to 0, it will reset the threshold to the default value.
	 */
	void SetThresholdPriority(unsigned int thresh_val);

	/**
	 * Gets the currently set threshold value.
	 * This returns the current threshold value.
	 *
	 * @return Returns the current threshold priority.
	 * @see SetThresholdPriority
	 */
	unsigned int GetThresholdPriority() const;

	/**
	 * Sets the level that Priority=0 gets mapped to.
	 * This will treat any entries in LogEvent called with Priority 0,
	 * as whatever you enter for this value (Default = 5)
	 * So by default, if you enter 0 as the priority to LogEvent, the 
	 * message will be treated as if you had entered priority=5.
	 *
	 * @param pri_level The priority level you want 0-values to be mapped to.
	 * If you set it to 0, it will reset the priorty to the default value.
	 */
	void SetDefaultPriority(unsigned int pri_level);

	/**
	 * Gets the currently set default priority.
	 * This returns the current default priority.
	 *
	 * @return Returns the current default priority.
	 * @see SetDefaultPriority
	 */
	unsigned int GetDefaultPriority() const;

	/**
	 * Set the target size for how large a file may be before segmenting.
	 * Logger will close the current file and create a new one after the 
	 * number of bytes written for the file exceeds this number of bytes.
	 * This is a soft limit. A log entry will not be divided between logs.
	 * So any segmentation will occur after a log entry is complete.
	 * 
	 * @param num_bytes The number of bytes a file needs before segmenting.
	 * The number 0 will disable the segmentation feature.
	 * 0 is the default.
	 */
	void SetMinSegmentSize(unsigned int num_bytes);

	/**
	 * Gets the minimum segment size.
	 * This returns the minimum size (in bytes) that is needed 
	 * before a log will be segmented.
	 *
	 * @return Returns the minimum size (in bytes) that is needed 
	 * before a log will be segmented.
	 * @see SetMinSegmentSize
	 */
	unsigned int GetMinSegmentSize() const;

	/**
	 * Enables echoing (to terminal).
	 * This will enable echoing to the echo handle. Typically when you 
	 * use this feature, you will want the file handle to be reporting 
	 * to a file, and you would echo to stdout or stderr.
	 * By default, echo is off.
	 */
	void EchoOn();

	/**
	 * Disables echoing.
	 * This will disable echoing to the echo handle. 
	 * By default, echo is off.
	 */
	void EchoOff();

	/**
	 * Returns true if Echo On is enabled.
	 * Returns true if echo is on or false if echo is off.
	 * 
	 * @return Returns true if enabled, false if disabled.
	 * @see EchoOn, EchoOff
	 */
	bool IsEchoOn() const;

	/**
	 * If echo is on, this will echo the output
	 * to be written to stdout. If the file handle is 
	 * also set to stdout, you will see the message appear
	 * twice in the stdout stream.
	 * This is the default echo handle.
	 *
	 * @see EchoOn, EchoOff, SetEchoToStderr
	 */
	void SetEchoToStdout();

	/**
	 * If echo is on, this will echo the output
	 * to be written to stderr. If the file handle is 
	 * also set to stderr, you will see the message appear
	 * twice in the stdout stream.
	 *
	 * @see EchoOn, EchoOff, SetEchoToStdout
	 */
	void SetEchoToStderr();

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
	 * @param pre_lines The number of newlines to print before each log entry.
	 */
	void SetPreLines(unsigned int pre_lines);

	/**
	 * This sets the number of blank lines succeeding a log entry.
	 * This is the number of newlines that will be printed
	 * right after a log entry.
	 * The default is 1.
	 * @param post_lines The number of newlines to print after each log entry.
	 */
	void SetPostLines(unsigned int post_lines);

	/**
	 * Sets the number of blank lines preceding a log entry and after a log 
	 * entry. This is a convenience function that allows you to set the number 
	 * of pre_lines and post_lines in the same call.
	 *
	 * @param pre_lines The number of newlines to print before each log entry.
	 * @param post_lines The number of newlines to print after each log entry.
	 * @see SetPreLines, SetPostLines
	 */
	void SetNewLines(unsigned int pre_lines, unsigned int post_lines);

	/**
	 * Gets the number of prelines.
	 * This returns the currently set number of prelines.
	 *
	 * @return Returns the currently set number of prelines.
	 * @see SetPreLines, SetNewLines
	 */
	unsigned int GetPreLines() const;

	/**
	 * Gets the number of postlines.
	 * This returns the currently set number of postlines.
	 *
	 * @return Returns the currently set number of postlines.
	 * @see SetPostLines, SetNewLines
	 */
	unsigned int GetPostLines() const;

	/**
	 * Returns true if logger will print with the specified priority.
	 * This is a convenience function that will tell you if the priorty 
	 * you submit will pass the threshold test. If the test passes, it means
	 * a log event with that priorty would be printed.
	 * 
	 * @param which_priority The priorty level you want to know if it will 
	 * print.
	 * @return Returns true if the priority you specified would pass the 
	 * threshold test and be logged. Returns false if it would not be printed.
	 * @see SetThresholdPriority, GetThresholdPriority
	 */
	bool WillLog(unsigned int which_priority) const;

	/*
	 * Sets the format string that is used to create the segment file 
	 * extensions.
	 * When segmentation occurs, a new file must be created. The scheme is 
	 * to take the base file name and append a numeric value represening the 
	 * current segment number (starting with 0). 
	 * 
	 * The default format is ".%04d".
	 * 
	 * This will produce files like 
	 * Mylog.log, Mylog.log.0000, Mylog.log.0001, etc.
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
	 * This function will make it's own internal copy of the string you
	 * pass in, so you may do what you want with your string after this call.
	 *
	 * @param format_string The format string 
	 * (the kind used in the printf family).
	 * NULL will reset the logger instance to use the default format.
	 *
	 */
	//void SetSegmentFormatString(const std::string& format_string);

	/**
	 * Sets the minimum with for the file extension segment.
	 * When a log segments, an extension with the part number is appended
	 * to the file name. This function will let you pad that extension
	 * with extra zeros to help you create similar lengthed extensions.
	 * For example, with no padding, you would get something like 
	 * .1, .2 ... .9, .10, ... .99, .100, ... .999, .1000,  
	 * If you enforced a minimum width of 3, you would get:
	 * .001, .002, ... .010, ... .099, ... .999, .1000
	 *
	 * Currently the default min width is 3.
	 * 
	 * @param width The minumum number of characters for the extension.
	 * If the value is 0, then the default value will be set (which
	 * is currently 3).
	 */
	void SetSegmentMinWidth(unsigned int width);

	/**
	 * Returns the current value set for the minumum segment width.
	 * @see SetSegmentWidth
	 * @return Returns the current segment minimum width.
	 */
	unsigned int GetSegmentMinWidth() const;

private:

	bool OpenFile(const std::string& filename);
	bool CloseFile();
	bool SegmentFile();

	int PrintHeaderToFileHandle(FILE* file_handle, 
		unsigned int pre_new_lines,
		unsigned int priority,
		const char* time_stamp, 
		const char* keyword, 
		const char* subkeyword);

	int PrintFooterToFileHandle(FILE* file_handle, 
		unsigned int post_new_lines,
		bool auto_flush_enabled);

	
	std::string baseName; /**< Base file name of logging file. */
//	size_t baseNameMaxSize; /**< Max size of baseName with \\0. */
	
	FILE* fileHandle; /**< File handle for log file. */
	FILE* echoHandle; /**< File handle for echo. */

	unsigned int thresholdPriority; /**< Min value events need to be printed. */
	unsigned int defaultPriority; /**< The value Pri=0 is interpreted as. */
	
	bool echoOn; /**< Flag for echoing messages. */
	bool loggingEnabled; /**< Flag for logging events. */
	bool autoFlushEnabled; /**< Flag for autoflush. */

	unsigned int preNewLines; /**< Number of newlines before an entry. */
	unsigned int postNewLines; /**< Number of newlines after an entry. */

	unsigned int segmentNumber; /**< Segment number of current log file. */
	unsigned int currentByteCount; /**< Current byte count for this segment. */
	unsigned int minSegmentBytes; /**< Size needed before segmenting. */
	
//	std::string segmentFormatString; /**< Format str for ext like ".%04d". */
//	size_t segmentFormatStringMaxSize; /**< Max size of the fmtstr with \\0. */	

	unsigned int segmentMinWidth; /**< Min width for ext like .1 or .01 or .001. */
	
#ifdef LOGGER_ENABLE_LOCKING
	/**
	 * Mutex used for locking if compiled for it.
	 * It is a void* so the header files can be hidden.
	 */
	mutable void* loggerMutex;
#endif
};

/*
 * A global instance pointer of logger.
 * Allow for a global instance of Logger so everybody can use
 * simply by including #include "Logger.hpp"
 * and defining in the global scope somewhere (like before main()).
 * Don't forget to call Create to actually create the instance.
 */
/* extern Logger g_Logger; */


#endif // LOGGER_HPP

