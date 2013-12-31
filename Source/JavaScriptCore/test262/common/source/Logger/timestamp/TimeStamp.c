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
#include "TimeStamp.h"


#include <stdio.h> /* For snprintf */
#include <time.h>

#if defined (_BSD_SOURCE) || defined (_SYSV_SOURCE) \
	|| defined (__FreeBSD__) || defined (__MACH__) \
	|| defined (__OpenBSD__) || defined (__NetBSD__) \
	|| defined (__bsdi__) || defined (__svr4__) \
	|| defined (bsdi) || defined (__SVR4) \
	|| defined (cygwin)
	#include <sys/time.h>
#elif defined (_WIN32)
	#include <windows.h>
	#include <sys/timeb.h> 
#else
/* SDL is used for cross-platform compatibility for millisecond precision. */
	#include "SDL_types.h"
	#include "SDL_timer.h"
	static Uint32 sync_ticks;
#endif
	
/* Visual Studio doesn't define snprintf but _snprintf */
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/* This function must be run after SDL_Init(Timer) if using SDL for time
 * Returns 1 if using native milliseconds, 0 if SDL.
 */
int TimeStamp_Init()
{
	/* This is function is designed pretty badly, but we need to 
	 * sync the SDL_GetTicks with the actual time or the 
	 * seconds will not wrap at the 
	 * same time the milliseconds wrap.
	 * Operating on the basis of it being too painful to write a good 
	 * cross platform time sync'er (not forgetting logistics like 
	 * building e.g. autoconf), and if I were, I could abandon SDL,
	 * I'm going to do a very painful grab seconds until 
	 * I find the transition point. This presumes the code
	 * is fast enough for this to work. There will most likely still 
	 * be inaccuracies in this method too, but it will reduce them.
	 * Note: The SDL sync may delay the program 1 second
	 */
#if defined (_BSD_SOURCE) || defined (_SYSV_SOURCE) \
	|| defined (__FreeBSD__) || defined (__MACH__) \
	|| defined (__OpenBSD__) || defined (__NetBSD__) \
	|| defined (__bsdi__) || defined (__svr4__) \
	|| defined (bsdi) || defined (__SVR4) 
/*	fprintf(stderr, "Timestamp using gettimeofday\n"); */
	/* not needed for the moment */
	return 1;
#elif defined (_WIN32)
	/* not needed for the moment */
/*	fprintf(stderr, "Timestamp using windows backend\n"); */
	return 1;
#else
	/* The brute force, imprecise way */
/*	fprintf(stderr, "Timestamp using bruteforce sdl\n"); */
	time_t last_time;
	time_t now_time;
	Uint32 last_ticks;
	
/*	fprintf(stderr, "Warning: No native routine for milliseconds available.\nUsing SDL to fake milliseconds (seconds and milliseconds may be out of sync)\n");
*/
	now_time = time(NULL);
	last_ticks = SDL_GetTicks();
	last_time = time(NULL);
	while(now_time <= last_time)
	{
		/* Getting a cache miss or other latency when the 
		 * seconds change. So capture the sync_ticks
		 * before the hit, and then add 1
		 */
		last_ticks = SDL_GetTicks();
		now_time = time(NULL);
	}
	sync_ticks = last_ticks+1;
	return 0;
#endif
}

/* We don't really need to do anything, but it is here in case
 * alternative or future implementations need it.
 */
void TimeStamp_Quit()
{
}
	
/*
 * YYYY-MM-DD_HH:MM:SS.mmm
 * requires that return_buf must be at least size 24.
 * Do not delete the returned string as its memory allocation is
 * controlled by this function.
 * 
 * Note this implementation is a hack for milliseconds
 * if using SDL, because the seconds and milliseconds are uncoordinated.
 * The actual milliseconds are faked since SDL only starts 
 * counting from the init of the program.
 * However, the milliseconds remain relative to each other
 * if not reflective of the actual millisecond time.
 * Also requires SDL be initialized.
 */
void TimeStamp_GetTimeStamp(char* time_stamp, int buffer_size)
{
	char time_format[21];
	
#if defined (_BSD_SOURCE) || defined (_SYSV_SOURCE) \
	|| defined (__FreeBSD__) || defined (__MACH__) \
	|| defined (__OpenBSD__) || defined (__NetBSD__) \
	|| defined (__bsdi__) || defined (__svr4__) \
	|| defined (bsdi) || defined (__SVR4) \
	|| defined (cygwin)
	struct timeval time_struct;
	unsigned short msec;
	gettimeofday( &time_struct, NULL );
	/* Need to make the cast to (const time_t*) because of FreeBSD and OSX warnings */
	strftime(time_format, 21, "%Y-%m-%d_%H:%M:%S.", localtime((const time_t*)&time_struct.tv_sec));
	msec = (unsigned short) ( ((long)(time_struct.tv_usec*.001)) % 1000);
#elif defined (_WIN32)
	struct _timeb time_struct;
	unsigned short msec;
	_ftime(&time_struct);
	strftime(time_format, 21, "%Y-%m-%d_%H:%M:%S.", localtime(&time_struct.time));
	msec = (unsigned short) (time_struct.millitm%1000);
#else
	time_t time_struct;
	Uint32 ticks;
	Uint16 msec;
	
	time(&time_struct);
	ticks = SDL_GetTicks()-sync_ticks;

	/* format time into string */
	strftime(time_format, 21, "%Y-%m-%d_%H:%M:%S.", localtime(&time_struct));

	/* get the millisecond (last 3 digits) */
	msec = (Uint16) (ticks%1000);
#endif
	
	snprintf(time_stamp, buffer_size, "%s%03d", time_format, msec);
}


/* Consider this version Deprecated.
 * This uses static memory to return the string, making it
 * hard to use in a multithreaded program.
 * 
 * YYYY-MM-DD_HH:MM:SS.mmm
 * requires that return_buf must be at least size 24.
 * Do not delete the returned string as its memory allocation is
 * controlled by this function.
 * 
 * Note this implementation is a hack for milliseconds
 * if using SDL, because the seconds and milliseconds are uncoordinated.
 * The actual milliseconds are faked since SDL only starts 
 * counting from the init of the program.
 * However, the milliseconds remain relative to each other
 * if not reflective of the actual millisecond time.
 * Also requires SDL be initialized.
 */
const char* TimeStamp_GetStaticTimeStamp()
{
	static char return_string[24];
	char time_format[21];
	
#if defined (_BSD_SOURCE) || defined (_SYSV_SOURCE) \
	|| defined (__FreeBSD__) || defined (__MACH__) \
	|| defined (__OpenBSD__) || defined (__NetBSD__) \
	|| defined (__bsdi__) || defined (__svr4__) \
	|| defined (bsdi) || defined (__SVR4) \
	|| defined (cygwin)
	struct timeval time_struct;
	unsigned short msec;
	gettimeofday( &time_struct, NULL );
	/* Need to make the cast to (const time_t*) because of FreeBSD and OSX warnings */
	strftime(time_format, 21, "%Y-%m-%d_%H:%M:%S.", localtime((const time_t*)&time_struct.tv_sec));
	msec = (unsigned short) ( ((long)(time_struct.tv_usec*.001)) % 1000);
#elif defined (_WIN32)
	struct _timeb time_struct;
	unsigned short msec;
	_ftime(&time_struct);
	strftime(time_format, 21, "%Y-%m-%d_%H:%M:%S.", localtime(&time_struct.time));
	msec = (unsigned short) (time_struct.millitm%1000);
#else
	time_t time_struct;
	Uint32 ticks;
	Uint16 msec;
	
	time(&time_struct);
	ticks = SDL_GetTicks()-sync_ticks;

	/* format time into string */
	strftime(time_format, 21, "%Y-%m-%d_%H:%M:%S.", localtime(&time_struct));

	/* get the millisecond (last 3 digits) */
	msec = (Uint16) (ticks%1000);
#endif
	
	snprintf(return_string, 24, "%s%03d", time_format, msec);
	return return_string;
}



