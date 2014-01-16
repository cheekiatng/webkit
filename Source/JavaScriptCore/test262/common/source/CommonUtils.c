#if defined(__APPLE__)
	#include <QuartzCore/QuartzCore.h>
	#include <unistd.h>
	static CFTimeInterval s_ticksBaseTime = 0.0;
	
#elif defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winbase.h>
		LARGE_INTEGER s_hiResTicksPerSecond;
		double s_hiResSecondsPerTick;
		LARGE_INTEGER s_ticksBaseTime;
#else
	#include <unistd.h>
	#include <time.h>
	static struct timespec s_ticksBaseTime;
#endif

void CommonUtils_InitTime()
{
	#if defined(__APPLE__)
		s_ticksBaseTime = CACurrentMediaTime();
	
	#elif defined(_WIN32)
		LARGE_INTEGER hi_res_ticks_per_second;
		if(TRUE == QueryPerformanceFrequency(&hi_res_ticks_per_second))
		{
			QueryPerformanceCounter(&s_ticksBaseTime);
			s_hiResSecondsPerTick = 1.0 / hi_res_ticks_per_second.QuadPart;
		}
		else
		{
			ALmixer_SetError("Windows error: High resolution clock failed.");
			fprintf(stderr, "Windows error: High resolution clock failed. Audio will not work correctly.\n");
		}
	#else
		/* clock_gettime is POSIX.1-2001 */
		clock_gettime(CLOCK_MONOTONIC, &s_ticksBaseTime);
	#endif

}

unsigned long CommonUtils_GetTicks()
{
	#if defined(__APPLE__)
		return (unsigned long)((CACurrentMediaTime()-s_ticksBaseTime)*1000.0);
	#elif defined(_WIN32)
		LARGE_INTEGER current_time;
		QueryPerformanceCounter(&current_time);
		return (unsigned long)((current_time.QuadPart - s_ticksBaseTime.QuadPart) * 1000 * s_hiResSecondsPerTick);
	#else /* assuming POSIX */
		/* clock_gettime is POSIX.1-2001 */
		struct timespec current_time;
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		return (unsigned long)((current_time.tv_sec - s_ticksBaseTime.tv_sec)*1000.0 + (current_time.tv_nsec - s_ticksBaseTime.tv_nsec) / 1000000);
	#endif
}

