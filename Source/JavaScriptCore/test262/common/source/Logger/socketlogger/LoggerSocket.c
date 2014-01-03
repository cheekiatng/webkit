

/* socket uses void* to deal with Windows differences */
extern DECLSPEC int CALLCONVENTION LoggerInternal_ClearFileData(
	Logger* logger,
)
{
	if(NULL == logger)
	{
		return 0;
	}
	/* Need to close any open file handles */
	Logger_CloseFile(logger);

	/* File segmentation related stuff is irrelevent. */
	free(logger->baseName);
	logger->baseName = NULL;
	logger->baseNameMaxSize = 0;
	logger->segmentNumber = 0;
	logger->currentByteCount = 0;
	logger->minSegmentBytes = 0;

}


#ifdef _WIN32
#define LOGGERSOCKETWIN_INITIALBUFFERSIZE 1452
typedef struct WinSocketUserData
{
	size_t formatStringBufferSize;
	char* formatStringBuffer;
} WinSocketUserData;

static int Logger_WinSocketPuts(Logger* logger, void* userdata, const char* text)
{
	Send();
}

static int Logger_WinSocketPrintf(Logger* logger, void* userdata, const char* format, ...);
static int Logger_WinSocketPrintfv(Logger* logger, void* userdata, void* const char* format, va_list argp);


/* socket_handle uses void* to deal with Windows differences.
 * This baseline implementation is for systems that support converting
 * sockets to FILE handles (i.e. not Windows)
 */
extern DECLSPEC int CALLCONVENTION Logger_SetSocket(
	Logger* logger,
	void* socket_handle
)
{
	WinSocketUserData* win_socket_user_data;
	

	if(NULL == logger)
	{
		return 0;
	}
	/* Technically, my locks here are not correct if I want this whole function to be atomic. 
	 * But the Logger_SetCustomPrintFunctions lock, so I need to fix things.
	 */

	win_socket_user_data = (WinSocketUserData*)calloc(1, sizeof(WinSocketUserData));
	if(NULL = win_socket_user_data)
	{
		return 0;
	}
	win_socket_user_data->formatStringBuffer = (char*)calloc(LOGGERSOCKETWIN_INITIALBUFFERSIZE, sizeof(char));

	if(NULL == win_socket_user_data->formatStringBuffer)
	{
		return 0;
	}
	win_socket_user_data->formatStringBufferSize = LOGGERSOCKETWIN_INITIALBUFFERSIZE;

	Logger_SetCustomPrintFunctions(logger, Logger_WinSocketPuts, Logger_WinSocketPrintf, Logger_WinSocketPrintfv, win_socket_user_data);




	LOGGER_LOCKMUTEX(logger);
	LoggerInternal_ClearFileData();

	FILE* socket_file = fdopen((int)socket_handle, "w");
	logger->fileHandle = socket_file;


	LOGGER_UNLOCKMUTEX(logger);


	return 1;
}


#else

/* socket_handle uses void* to deal with Windows differences.
 * This baseline implementation is for systems that support converting
 * sockets to FILE handles (i.e. not Windows)
 */
extern DECLSPEC int CALLCONVENTION Logger_SetSocket(
	Logger* logger,
	void* socket_handle
)
{
	if(NULL == logger)
	{
		return 0;
	}


	LOGGER_LOCKMUTEX(logger);
	LoggerInternal_ClearFileData();

	FILE* socket_file = fdopen((int)socket_handle, "w");
	logger->fileHandle = socket_file;


	LOGGER_UNLOCKMUTEX(logger);


	return 1;
}

#endif

