#include "Logger.h"
#include "LoggerPrimitives.h"
#include "lua.h"
#include "lauxlib.h"

#define LOGGERLUA_METATABLE_NAME "Logger"
#define LOGGERLUA_LIBRARY_NAME "Logger"


#define LOGGERLUA_SUPPORT_LIGHTUSERDATA 1


/* Uses luaL_checkudata to check for the correct UserData type.
 * Then digs out the pointer to the actual Logger instance and returns it.
 * Remember that luaL_checkudata throws a lua_error and doesn't return if it fails.
 */
static Logger* LoggerLua_CheckUDataLogger(lua_State* lua_state, int index)
{
	Logger** loggeruserdata = (Logger**)luaL_checkudata(lua_state, index, LOGGERLUA_METATABLE_NAME);
	if(NULL != loggeruserdata)
	{
		return *loggeruserdata;
	}
	else
	{
		return NULL;
	}
}
/* As a compile option, I am allowing for either userdata or light userdata because 
 * I want to allow users to pass in a Logger instance 
 * created on the C-side and not necessarily managed by the Lua garbage collector.
 * But this could be used as a way of allowing a malicious user to crash a program from Lua.
 * So this is why it is a compile option.
 *
 * To make this easy to toggle, all functions should use this function to retrieve the Logger instance.
 * So this will return a Logger instance at the specified Lua index.
 * If the object is light user data then the pointer is extracted and returned (if compiled in).
 * If the object is full user data, then LoggerLua_CheckUDataLogger is used. 
 * (Remember that this latter one will throw a Lua error and never returns on a failure.)
 */
static Logger* LoggerLua_ExtractLogger(lua_State* lua_state, int index)
{
	Logger* logger = NULL;
	switch(lua_type(lua_state, 1))
	{
#if LOGGERLUA_SUPPORT_LIGHTUSERDATA
		case LUA_TLIGHTUSERDATA:
		{
			logger = (Logger*)lua_touserdata(lua_state, index);
			break;
		}
#endif
		case LUA_TUSERDATA:
		{
			logger = LoggerLua_CheckUDataLogger(lua_state, index);
			break;
		}
		default:
		{
			luaL_error(lua_state, "Expected Logger instance at Lua index %d", index);
			/* luaL_error never reaches here. */
			return NULL;
		}
	}
	if(NULL == logger)
	{
		luaL_error(lua_state, "Expected Logger instance at Lua index %d", index);
		/* luaL_error never reaches here. */
		return NULL;
	}
	return logger;
}


static int LoggerLua_LogEvent(lua_State* lua_state)
{
	int n = lua_gettop(lua_state);  /* number of arguments */
	int i;

	/* logger:LogEvent(pri, "keyword", "subkeyword", "text", "moretext", "moretext") */
  	char time_stamp[LOGGER_TIME_STAMP_SIZE];
	int byte_counter = 0;
	int ret_val;
	int error_flag = 0;
	va_list echo_copy;
	Logger* logger;
	int priority;
	const char* keyword;
	const char* subkeyword;
	size_t keyword_length = 0;
	size_t subkeyword_length = 0;
	int start_text_index = 5;

	/* First parameter needs to be a logger instance. Anything less than 2 parameters definitely won't work. */
	if(n<2)
	{
		return luaL_error(lua_state, "Not enough parameters passed to LogEvent.");
		
		/*
		lua_pushinteger(lua_state, 0);
		return 1;
		*/
	}

	/* I am allowing for either userdata or light userdata because I want to allow users to pass in a Logger instance 
	 * created on the C-side and not necessarily managed by the Lua garbage collector.
	 */
	logger = LoggerLua_ExtractLogger(lua_state, 1);

	LOGGER_LOCKMUTEX(logger);

	if(!logger->loggingEnabled)
	{
		LOGGER_UNLOCKMUTEX(logger);
		lua_pushinteger(lua_state, 0);
		return 1;
	}

	/* Allow for optional priority? Then next parameter must be a string to avoid ambigutiy issues. */
	/* Should we call tonumber or just assume an integer? Assuming an integer is faster, but tonumber is more flexible. */
	/* For now, not optional because, and using Lua integer APIs for speed. */
	/* I cannot use the luaL_check* family because I need to unlock the logger mutex if it fails before returning.
	 * I basically reproduce the check implementation here.
	 */
	priority = lua_tointeger(lua_state, 2);
	if(0 == priority && !lua_isnumber(lua_state, 2))  /* avoid extra test when d is not 0 */
	{
		LOGGER_UNLOCKMUTEX(logger);
		return luaL_error(lua_state, "Second parameter must be an integer for priority value.");
	}

	if(0 == priority)
	{
		priority = logger->defaultPriority;
	}
	/* print to log only if priority is >= threshold */
	if(priority < logger->thresholdPriority)
	{
		LOGGER_UNLOCKMUTEX(logger);
		lua_pushinteger(lua_state, 0);
		return 1;
	}
	
	/* Get the current time */
	TimeStamp_GetTimeStamp(time_stamp, LOGGER_TIME_STAMP_SIZE);

	/* if segmentation is enabled */
	if(logger->minSegmentBytes != 0)
	{
		if(logger->currentByteCount > logger->minSegmentBytes)
		{
			/* SegmentFile() will just return if logger->fileHandle
			 * is not a file.
			 */
			if(!Logger_SegmentFile(logger))
			{
				/* Might still want echo
				 * Commented out for now
				 * logger->loggingEnabled = false;
				 * Pointer should be set to NULL
				 */
			}
			logger->currentByteCount = 0;
		}
	}

	/* Now fetch the keyword and subkeyword.
	 * This was deferred until after testing the priority threshold for performance to not incur any overhead when no log event is needed.
	 * I cannot use the luaL_check* family because I need to unlock the logger mutex if it fails before returning.
	 * The C implementation actually allows NULL for these strings. So I'm thinking maybe error checking isn't worth doing here.
	 */
	keyword = lua_tolstring(lua_state, 3, &keyword_length);
	subkeyword = lua_tolstring(lua_state, 4, &subkeyword_length);
	

	/* Log stuff to fileHandle */
	if(logger->fileHandle != NULL)
	{
		int use_custom_print = LOGGER_USE_CUSTOM_PRINT(logger);
		
		if(0 == use_custom_print)
		{
			ret_val = Logger_PrintHeaderToFileHandle(logger->fileHandle,
				logger->preNewLines,
				priority, time_stamp, keyword, subkeyword);
		}
		else
		{
			ret_val = Logger_PrintHeaderWithCustom(logger,
				logger->preNewLines,
				priority, time_stamp, keyword, subkeyword);
		}
		if(ret_val >= 0)
		{
			logger->currentByteCount = ret_val;
		}
		/* Try to record/preserve the first error flag because subsequent 
		 * messages might be less helpful. */
		else if(0 == error_flag)
		{
			error_flag = ret_val;
		}

		lua_getglobal(lua_state, "tostring");
		for (i=start_text_index; i<=n; i++)
		{
			const char *s;
			size_t s_length = 0;
			
			lua_pushvalue(lua_state, -1);  /* function to be called */
			lua_pushvalue(lua_state, i);   /* value to print */
			lua_call(lua_state, 1, 1);
			s = lua_tolstring(lua_state, -1, &s_length);  /* get result */
			if (s == NULL)
			{
				LOGGER_UNLOCKMUTEX(logger);
				
				return luaL_error(lua_state, LUA_QL("tostring") " must return a string to "
					LUA_QL("print"));
			}
			if (i>start_text_index)
			{
				/* Lua print puts tabs between comma separated parameters. Should logger do the same? */
				if(0 == use_custom_print)
				{
					ret_val = fputs("\t", logger->fileHandle);
				}
				else
				{
					ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, "\t");
				}
				if(ret_val >= 0)
				{
					logger->currentByteCount += 1;	
				}
				else if(0 == error_flag)
				{
					/* save the error condition */
					error_flag = ret_val;
				}
			}

			if(0 == use_custom_print)
			{
				ret_val = fputs(s, logger->fileHandle);
			}
			else
			{
				ret_val = ((LoggerOpaqueData*)logger->opaqueLoggerData)->customPuts(logger, ((LoggerOpaqueData*)logger->opaqueLoggerData)->customCallbackUserData, priority, keyword, subkeyword, s);
			}
			if(ret_val >= 0)
			{
				logger->currentByteCount += strlen(s);	
			}
			else if(0 == error_flag)
			{
				/* save the error condition */
				error_flag = ret_val;
			}

			lua_pop(lua_state, 1);  /* pop result */
		}
		lua_pop(lua_state, 1);  /* pop tostring */

		if(0 == use_custom_print)
		{
			ret_val = Logger_PrintFooterToFileHandle(logger->fileHandle,
				logger->postNewLines,
				logger->autoFlushEnabled);
		}
		else
		{
			ret_val = Logger_PrintFooterWithCustom(logger,
				logger->postNewLines,
				logger->autoFlushEnabled);
		}
		if(ret_val >= 0)
		{
			logger->currentByteCount += ret_val;	
		}
		else if(0 == error_flag)
		{
			/* save the error condition */
			error_flag = ret_val;
		}

	}


	/* Repeat again (to terminal) if echo is on. */
	/* This could have been made faster by saving the results from the previous pass into a temporary array (e.g. Lua array).
	 * But I like avoiding memory allocation/collection where necessary for other speed gains. Might be worth benchmarking some day. */
	if(logger->echoOn && (NULL != logger->echoHandle))
	{
		ret_val = Logger_PrintHeaderToFileHandle(logger->echoHandle,
			logger->preNewLines,
			priority, time_stamp, keyword, subkeyword);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

		
		lua_getglobal(lua_state, "tostring");
		for (i=start_text_index; i<=n; i++)
		{
			const char *s;
			size_t s_length = 0;
			
			lua_pushvalue(lua_state, -1);  /* function to be called */
			lua_pushvalue(lua_state, i);   /* value to print */
			lua_call(lua_state, 1, 1);
			s = lua_tolstring(lua_state, -1, &s_length);  /* get result */
			if (s == NULL)
			{
				LOGGER_UNLOCKMUTEX(logger);
				
				return luaL_error(lua_state, LUA_QL("tostring") " must return a string to "
					LUA_QL("print"));
			}
			if (i>start_text_index)
			{
				/* Lua print puts tabs between comma separated parameters. Should logger do the same? */
				ret_val = fputs("\t", logger->fileHandle);
				if(ret_val >= 0)
				{
					/* Don't change byte count for echo */
				}
				else if(0 == error_flag)
				{
					/* save the error condition */
					error_flag = ret_val;
				}
			}

			ret_val = fputs(s, logger->fileHandle);
			if(ret_val >= 0)
			{
				/* Don't change byte count for echo */
			}
			else if(0 == error_flag)
			{
				/* save the error condition */
				error_flag = ret_val;
			}

			lua_pop(lua_state, 1);  /* pop result */
		}
		lua_pop(lua_state, 1);  /* pop tostring */

		
		ret_val = Logger_PrintFooterToFileHandle(logger->echoHandle,
			logger->postNewLines,
			logger->autoFlushEnabled);
		if((ret_val < 0) && (0 == error_flag))
		{
			error_flag = ret_val;
		}

	}	
		
	LOGGER_UNLOCKMUTEX(logger);

	if(error_flag == 0)
	{
		lua_pushinteger(lua_state, byte_counter);
		return 1;
	}
	else
	{
		lua_pushinteger(lua_state, error_flag);
		return 1;
	}
}


static int LoggerLua_Create(lua_State* lua_state)
{
	Logger* logger;
	Logger** loggerudata;
	int num_arguments = lua_gettop(lua_state);
	/* TODO: Support optional table parameters to specify file names, file handles, socket, etc.? */
  
	logger = Logger_Create();

	loggerudata = (Logger**)lua_newuserdata(lua_state, sizeof(Logger*));
	//*loggerudata = Logger_Create();
	*loggerudata = logger;


	luaL_getmetatable(lua_state, LOGGERLUA_METATABLE_NAME);
	lua_setmetatable(lua_state, -2);

	return 1;
}

static int LoggerLua_CloseFile(lua_State* lua_state)
{
	Logger* logger = LoggerLua_ExtractLogger(lua_state, 1);
	Logger_CloseFile(logger);
}

static int LoggerLua_GarbageCollect(lua_State* lua_state)
{
	/* There shouldn't be any way a light user data will hit this __gc method. */
	Logger* logger = LoggerLua_CheckUDataLogger(lua_state, 1);
	Logger_Free(logger);
	return 0;
}

static const luaL_Reg s_LoggerFunctions[] =
{
	{"Create", LoggerLua_Create},
/* It's not so bad if these functions are exposed to the Logger table. 
 * Maybe I should leave them in for all cases.
 */
/* #if LOGGERLUA_SUPPORT_LIGHTUSERDATA */
	{"LogEvent", LoggerLua_LogEvent},
	{"CloseFile", LoggerLua_CloseFile},
/* #endif */
/* 	{"GetTimeStamp", LoggerLua_GetTimeStamp}, */
/*	{"LogEventWithTimeStamp", LoggerLua_LogEventWithManualTimeStamp}, */
	{NULL, NULL}
};

static const luaL_Reg s_LoggerMethods[] =
{
	{"LogEvent", LoggerLua_LogEvent},
	{"CloseFile", LoggerLua_CloseFile},
	{"__gc", LoggerLua_GarbageCollect},
	{NULL, NULL}
};

/*
** Open library
*/
int luaopen_loggerlua(lua_State* lua_state)
//LOGGER_DECLSPEC int LOGGER_CALLCONVENTION luaopen_logger(lua_State *L)
{
	luaL_newmetatable(lua_state, LOGGERLUA_METATABLE_NAME);

	/* metatable.__index = metatable */
	lua_pushvalue(lua_state, -1); /* duplicates the metatable */
	lua_setfield(lua_state, -2, "__index");
	luaL_register(lua_state, NULL, s_LoggerMethods);
	luaL_register(lua_state, LOGGERLUA_LIBRARY_NAME, s_LoggerFunctions);
	return 1;
}

