
#include "Logger.hpp"


int main(int argc, char* argv[])
{
	Logger logger;
	LoggerVersion compiled;
	LoggerVersion linked;
	unsigned int some_priority = 3;
	int i;
	bool ret_flag;

	ret_flag = logger.Init("MyLog.log");
	if(false == ret_flag)
	{
		fprintf(stderr, "Error: Logger creation failed, quiting program.\n");
		return 0;
	}

	compiled = Logger::GetCompiledVersion();
	linked = Logger::GetLinkedVersion();

	
//	LOGGER_GET_COMPILED_VERSION(&compiled);
//	Logger_GetLinkedVersion(&linked);
	
	/* Setting to 1 will print everything. */
	logger.SetThresholdPriority(1);
	
	/* Will evaluate Pri=0 to what is set here. */
	logger.SetDefaultPriority(5);

	/* Turn on echoing */
	logger.EchoOn();

	/* Set echoing to stderr */
	logger.SetEchoToStderr();

	/* Turn on segmenting to an extreme value for testing. */
	logger.SetMinSegmentSize(1);

	/*
	logger.SetSegmentFormatString(logger, ".%2d");
	logger.SetSegmentFormatString(logger, ".%d");
	*/
//	logger.SetSegmentFormatString(".%04d");
	logger.SetSegmentMinWidth(0);

	logger.LogEvent(0, "Initialization", "main.c", "We are compiled against Logger version %d.%d.%d", compiled.major, compiled.minor, compiled.patch);

	logger.LogEvent(0, "Initialization", "main.c", "We are linked against Logger version %d.%d.%d", linked.major, linked.minor, linked.patch);


	for(i=0; i<101; i++)
	{
		logger.LogEvent(some_priority, "Debug", "main.c", "Testing a message with priority=%d", some_priority);
	}

	logger.LogEventNoFormat(0, "Shutdown", "main.c", "Reached end of logging test.");


	logger.Close();

	return 0;
}

