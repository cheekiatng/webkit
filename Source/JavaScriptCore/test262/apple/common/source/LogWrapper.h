//
//  Test262Helper.h
//  Test262
//
//  Created by Eric Wing on 12/30/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Logger.h"
#import "LoggerObjC.h"

#define LOGWRAPPER_PRIORITY_CRITICAL 5
#define LOGWRAPPER_PRIORITY_STANDARD 3
#define LOGWRAPPER_PRIORITY_DEBUG 1

#define LOGWRAPPER_PRIMARY_KEYWORD "Test262"


@interface LogWrapper : NSObject
{
// I made these public so I can easily access these in a public macro.
// You shouldn't use these unless you know what your doing.
// public/protected/private is really just a comment in Obj-C anyway.
@public
	Logger* loggerFile;
	Logger* loggerNative;
	Logger* loggerSocket;
}

@property(strong, nonatomic) NSString* logFilePathAndName;

//+ (instancetype) sharedInstance;
- (instancetype) init;

- (void) flush;

// This will overwrite existing files. Opens a new file with a hardcoded name in the Documents directory.
- (BOOL) openNewFile;
// This will overwrite existing files.
- (BOOL) openNewFileWithName:(NSString*)full_path_and_file;
// closes the open file, but leaves the native and socket loggers untouched
- (void) closeFile;

@end


// I use variadic macros here because I want to pass the same string message to multiple functions.
// Without macros, this gets into temporary string creation or nasty platform specific/undefined behavior of va_copy.
#define LogWrapper_LogEvent(log_wrapper, priority, keyword, subkeyword, ...) \
	do \
	{ \
		LoggerObjC_LogEvent(log_wrapper->loggerFile, priority, keyword, subkeyword, __VA_ARGS__); \
		LoggerObjC_LogEvent(log_wrapper->loggerNative, priority, keyword, subkeyword, __VA_ARGS__); \
		LoggerObjC_LogEvent(log_wrapper->loggerSocket, priority, keyword, subkeyword, __VA_ARGS__); \
	} \
	while (0)
