/*
	LoggerObjC
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

#ifndef LOGGEROBJC_H
#define LOGGEROBJC_H
/**
 * @mainpage LoggerObjC
 * 
 * This is an Objective-C extension to the C-based Logger.
 * It basically provides extra functions to be used in Objective-C
 * by providing a few new API calls. (Think of this as a category, 
 * except no Obj-C classes are in play.)
 * This depends directly on the C version of Logger.
 * Logger.h is included by this header. You are expected to
 * initialize Logger as usual and you may call all Logger API
 * functions as normal. But when using %@ in format strings,
 * you should use the functions provided in this API extension.
 * Ultimately, they trickle-down into the Logger C-API.
 *
 * The original implementation was not optimal and used macros to create a C-string 
 * and pass to public Logger API functions. Ideally, the NSString allocation should
 * be deferred until the last point where it is determined that it is really needed.
 * But since the Logger implementation hides aways private functions as static,
 * it is not easy to get at them.
 * A more optimal implementation is available, but requires some extra hoops to compile.
 * The implementation pretends it is part of the Logger.c file.
 * This means when building, the files should be concatenated or something.
 * 
 * Currently the CMake build system does not build this.
 * To build, you will probaby want to concatenate the two files, e.g.
 * cat Logger.c LoggerObjC.m > LoggerObjCMerged.m
 * Then compile the LoggerObjCMerged.m as if it were a full implementation.
 * Remember you will still need both header files.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "Logger.h"

#ifdef __OBJC__
@class NSString;
/**
 * This is Logger_LogEvent except that it allows for Objective-C %@,
 * and any other Obj-C-isms that I may not be aware of.
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
 * only constraint is that you should never have an END: on it's own 
 * line as this is the marker that Logger uses to denote the end of a 
 * log event. (Parsing tools and people could get confused.)
 *
 * @param ... This is the variable argument list used to accomodate 
 * printf style format strings.
 *
 * @return Returns the number of bytes written to the log. This conforms 
 * with fprintf.
 *
 * @note Assumes the existence of an autorelease pool.
 *
 * @see Logger_LogEvent
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION LoggerObjC_LogEvent(
	Logger* logger,
	unsigned int priority,
	const char* keyword, const char* subkeyword,
	NSString* text, ...);

/* Legacy macro
#define LoggerObjC_LogEvent(logger, priority, keyword, subkeyword, text, ...) \
{ \
	NSString* the_string = [[NSString alloc] initWithFormat:text, __VA_ARGS__]; \
  	int bytes_written = Logger_LogEventNoFormat(logger, priority, keyword, subkeyword, [the_string UTF8String]); \
	[the_string release]; \
	return bytes_written; \
} 
*/

/**
 * This is the va_list version of LoggerObjC_LogEvent.
 * The same rules apply to this as with LoggerObjC_LogEvent.
 *
 * @see Logger_LogEventv, LoggerObjC_LogEvent
 */
extern LOGGER_EXPORT int LOGGER_CALLCONVENTION LoggerObjC_LogEventv(
	Logger* logger, 
	unsigned int priority,
	const char* keyword, const char* subkeyword,
	NSString* text, va_list argp);

/* Legacy Marco 
#define LoggerObjC_LogEventv(logger, priority, keyword, subkeyword, text, argp) \
{ \
	NSString* the_string = [[NSString alloc] initWithFormat:text arguments:argp]; \
  	Logger_LogEventNoFormat(logger, priority, keyword, subkeyword, [the_string UTF8String]); \
	[the_string release]; \
	return bytes_written; \
} 
*/

#endif /* __OBJC__ */


#ifdef __cplusplus
}
#endif

#endif

