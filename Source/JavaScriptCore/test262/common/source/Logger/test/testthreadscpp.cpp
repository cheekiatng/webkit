#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 10

#include "Logger.hpp"

Logger g_Logger;

void *PrintHello(void *threadid)
{
	int i;
	for(i=0; i<1000; i++)
	{
		g_Logger.LogEvent(1, "Test", "PrintHello", "Hello World %d in thread %d", i, threadid);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

	pthread_t threads[NUM_THREADS];
	int rc, t;
	bool ret_flag;

	ret_flag = g_Logger.Init("ThreadLog.log");
	if(false == ret_flag)
	{
		fprintf(stderr, "Error: Logger creation failed, quiting program.\n");
		return 1;
	}


	/* Setting to 1 will print everything. */
	g_Logger.SetThresholdPriority(1);

	/* Will evaluate Pri=0 to what is set here. */
	g_Logger.SetDefaultPriority(5);

	/* Turn on echoing */
	g_Logger.EchoOn();

	/* Set echoing to stderr */
	g_Logger.SetEchoToStderr();

	/* Turn on segmenting to an extreme value for testing. */
	g_Logger.SetMinSegmentSize(0);

	/*
	 g_Logger.SetSegmentFormatString(logger, ".%2d");
	 g_Logger.SetSegmentFormatString(logger, ".%d");
	 */
	//g_Logger.SetSegmentFormatString(".%04d");


	if(g_Logger.IsCompiledWithLocking())
	{
		g_Logger.LogEvent(1, "Test", "Init", "Logger is compiled with Locking Enabled\n");
	}
	else
	{
		g_Logger.LogEvent(1, "Test", "Init", "Logger is NOT compiled with Locking Enabled\n");
	}


	for(t=0;t<NUM_THREADS;t++){
		printf("Creating thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	pthread_exit(NULL);

//	g_Logger.Free(g_Logger);

	return 0;
}

