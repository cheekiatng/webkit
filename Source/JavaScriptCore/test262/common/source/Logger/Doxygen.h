/* This header is only to make Doxygen to behave nicely. */

/**
 * @mainpage Logger: Logging system for performance, large data, & report generation
 * Logger is a logging system for performance, large data generation,
 * and post-processing.
 *
 * Logger was originally developed for large automated testing procedures
 * against realtime hardware devices such as commercial satellites.
 * Its goals were to log various types of events in human readable formats,
 * but in an output that could easily be parsed and searched for later
 * post-processing report generation.
 *
 * During a run-for-record, large amounts of data could be generated 
 * (measured in gigabytes) so the logging facility needed to support
 * auto-segmentation of files to avoid exceeding maximum file size limits.
 *
 * Also an optional echoing feature is available for those wishing to see the
 * output logged to a file but also a terminal for monitoring.
 *
 * For additional speed and brevity, priority thresholds can be set 
 * at runtime to eliminate low priority messages from being logged.
 *
 * Log Event entries were designed to be easily parsable so custom tools
 * could easily be written to "grep" the data for specific queries and
 * specific types of report generation. The format was originally designed
 * as follows: (note tabs used in the first line to make regex simpler)
@code
TimeStamp 	Keyword:	SubKeyword: 	PRI=<num>
Log Event message. Generally free form, including multiple lines, but avoid the end marker convention.
END:
@endcode

 * TimeStamps were written in a descending format: Year-Month-Day_HH:MM:SS.mmm
 * The descending order and consistent spacing are the key attributes as it 
 * makes post-parsing very easy and also makes sorting dates a trivial operation
 * as the descending order guarantees a simple string comparison or number
 * comparison will always sort in the correct order.
 * The END: marker always starts on the beginning of a newline and ends with
 * a colon. Again, this is to make parsing faster disambigutate it from 
 * messages. Still, care should be taken to not introduce messages that 
 * will confuse post-processing tools. Logger itself doesn't really care
 * and does no enforcement for performance.
 *
 * Here is a sample timestamp:
 * @code
 * 2002-10-29_19:59:32.321    Error:    CommunicationsSystem:    PRI=5
 * CRC checksums failed.
 * END:
 * @endcode
 *
 * Logger has been rewritten many times in many languages.
 * Included in this package is my re-invention of Logger 
 * to C and C++ (with also 
 * an Objective-C macro interface that lives on top of the C version.
 * I have found more mundane uses for Logger than commercial aerospace,
 * but have still found it useful nonetheless. 
 * Some of the features like auto-segmentation
 * might be overkill, but they can be easily disabled.
 *
 * See the Files section in the Doxygen output for the specific documentation
 * for the C or/and C++ versions. They are almost identical except that the
 * C++ gets to implicitly hide the Logger object. The C++ version uses printf
 * sementics for most operations instead of iostreams. This is somewhat because
 * I never really cared to implement iostreams. It also wasn't obvious to me
 * how to bypass stream processing when messages could be suppressed for 
 * performance, nor is it obvious how to deal with localization issues where
 * parameters may need to be reordered.
 *
 * Be aware the echo feature uses C99 va_copy. There is a fallback
 * for compilers that don't support this (e.g. Visual Studio) but 
 * this may need to be dealt with for other compilers I haven't tested on.
 *
 * Similar warning for threads. I currently only support pthreads and Windows.
 * If you don't need thread safety in Logger, you can flip a compile switch
 * to disable locking for additional performance.
 *
 * Currently the C version might have a slight edge on features.
 * The C version lets you specify a format string for file segmentation.
 * This code has not been ported to the C++ version. (See comments inside
 * the C++ implementation for why.)
 * Lookup Logger.h under Files in the Doxygen documentation for the C-API.
 * Lookup Logger under data structures in the Doxygen documentation for C++.
 * 
 * Sample Usage (C API):
 * @code
 * Logger* a_logger = Logger_Create("myoutput.log");
 * Logger_LogEvent(a_logger, 5, "Error", "SDL_image::SDL_glLoadImage",
 *              "Error found: %s", SDL_GetError());
 * Logger_Free(a_logger);
 * @endcode
 *
 * Sample Usage (C++ API):
 * @code
 * Logger a_logger;
 * a_logger.Init("myoutput.log");
 * a_logger.LogEvent(5, "Error", "SDL_image::SDL_glLoadImage",
 *              "Error found: %s", SDL_GetError());
 * a_logger.Close(); // Not necessary to explicitly call, called by destructor
 * @endcode
 *
 *
 * @author Eric Wing <ewing . public @ playcontrol.net>
 *
 * License: MIT
 *
 * Home Page: http://playcontrol.net/opensource/Logger
 */
 

