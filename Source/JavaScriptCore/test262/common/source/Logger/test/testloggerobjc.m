
#include "LoggerObjC.h"
#include <Foundation/Foundation.h>

int main(int argc, char* argv[])
{
	NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
	Logger* logger;
	LoggerVersion compiled;
	LoggerVersion linked;
	unsigned int some_priority = 3;
	int i;

	logger = Logger_CreateWithName("MyLog.log");
	if(NULL == logger)
	{
		fprintf(stderr, "Error: Logger creation failed, quiting program.\n");
		return 0;
	}

	LOGGER_GET_COMPILED_VERSION(&compiled);
	Logger_GetLinkedVersion(&linked);
	
	/* Setting to 1 will print everything. */
	Logger_SetThresholdPriority(logger, 1);
	
	/* Will evaluate Pri=0 to what is set here. */
	Logger_SetDefaultPriority(logger, 5);

	/* Turn on echoing */
	Logger_EchoOn(logger);

	/* Set echoing to stderr */
	Logger_SetEchoToStderr(logger);

	/* Turn on segmenting to an extreme value for testing. */
	Logger_SetMinSegmentSize(logger, 1);

	Logger_SetSegmentFormatString(logger, ".%03d");


	Logger_LogEvent(logger, 0, "Initialization", "main.c", "We are compiled against Logger version %d.%d.%d", compiled.major, compiled.minor, compiled.patch);

	Logger_LogEvent(logger, 0, "Initialization", "main.c", "We are linked against Logger version %d.%d.%d", linked.major, linked.minor, linked.patch);


	for(i=0; i<101; i++)
	{
		LoggerObjC_LogEvent(logger, some_priority, "Debug", "main.c", @"Testing a message with priority=%d", some_priority);
	}

	NSString* test_string = @"Hello World";
	LoggerObjC_LogEvent(logger, some_priority, "Debug", "main.c", @"test_stiring: %@", test_string);
	Logger_LogEventNoFormat(logger, 0, "Shutdown", "main.c", "Reached end of logging test.");


	Logger_Free(logger);

	[autorelease_pool drain];

	return 0;
}

