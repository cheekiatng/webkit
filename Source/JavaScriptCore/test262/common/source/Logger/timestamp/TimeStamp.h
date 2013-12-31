/*
	TimeStamp
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


#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function initializes the TimeStamp library.
 * This is currently only needed by the SDL backend.
 * New implementations might want an initialization phase too.
 * For the SDL backend, SDL_Init(Timer) must have been called 
 * before calling this function.
 * The SDL backend was written poorly. The problem is that the ticks
 * and the real time are not in sync, so it is possible for the milliseconds
 * to wrap before the real time is carried. (More notes are in the 
 * implementation file about this.) The SDL sync may delay the program
 * for 1 second with this call due to the sync approximation.
 * @return Returns 1 if using native milliseconds, 0 if faking it.
 * @see TimeStamp_Quit
 */
extern int TimeStamp_Init(void);

/**
 * This function shuts down the TimeStamp library.
 * This is currently unused, but future or alternative 
 * implementations might want this.
 * Note that currently, the SDL backend will not call SDL_Quit
 * on your behalf and you must do this manually.
 */
extern void TimeStamp_Quit(void);

/**
 * This function gets the current time and copies the time (in string format)
 * into the provided buffer.
 * This is now the preferred method of retrieving a timestamp as there 
 * is no static memory involved.
 * You must supply a preallocated string that the time stamp will
 * be copied into. You also must supply the buffer size of the string
 * which will be used to help prevent buffer overflows.
 *
 * My particular implementation of the library always needs a buffer 
 * that is size 24 (including the '\\0' character). However, if you 
 * substitute my implementation, this buffer size should give you 
 * some flexibility without needing to break compatibility.
 *
 * My particular implementatin returns a time stamp in this format:
 * YYYY-MM-DD_HH:MM:SS.mmm
 * 
 * @code
 * char current_time[24];
 * TimeStamp_GetTimeStamp(current_time, 24);
 * printf("Current Time is: %s", current_time);
 * @endcode
 * 
 * @param time_stamp A pointer to a string that you want the time stamp 
 * copied into. It must have enough memory or it will be truncated.
 *
 * @param buffer_size The size of the string buffer you pass in. This
 * should include the '\\0' character.
 *
 */
extern void TimeStamp_GetTimeStamp(char* time_stamp, int buffer_size);

/**
 * This is a version of GetTimeStamp that returns a pointer to 
 * the string as the return argument. This version probably shouldn't 
 * be used. It is not reentrant because it uses a static member to hold 
 * the data. But as a convenience, it remains available. (Sometimes you 
 * just want to print stuff without a fuss.)
 * @return Returns a pointer to the time string holding the current time.
 * The memory is held by a static array provided by this library.
 * Do not delete this memory.
 */
extern const char* TimeStamp_GetStaticTimeStamp(void);


#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* TIMESTAMP_H */

