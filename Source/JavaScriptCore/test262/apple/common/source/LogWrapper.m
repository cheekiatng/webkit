#include "LogWrapper.h"

#include "Logger.h"

// These three functions provide a Logger compatibility interface to Logger.
// I learned from experience that NSLog is treated as special by Apple on iOS and in the iOS simulator,
// and in some cases, this is the only way to get output displayed through the console.
// I also learned the hard way that ASLog is not the same as NSLog, despite that it is implied that it is.
static int LogWrapper_CustomPrintfToNSLog(Logger* logger, void* userdata, const char* format, ...)
{
	va_list argp;
	va_start(argp, format);
	NSString* ns_format = [[NSString alloc] initWithUTF8String:format];
	// In order to get the number of bytes to use as a return value, I must create the full string because NSLog returns void.
	NSString* the_string = [[NSString alloc] initWithFormat:ns_format arguments:argp];
	// Since I already created the full string and used up the argp, I have to pass this.
	NSLog(@"%@", the_string);
	va_end(argp);
	return (int)[the_string length];
}

static int LogWrapper_CustomPrintfToNSLogv(Logger* logger, void* userdata, const char* format, va_list argp)
{
	NSString* ns_format = [[NSString alloc] initWithUTF8String:format];
	// In order to get the number of bytes to use as a return value, I must create the full string because NSLog returns void.
	NSString* the_string = [[NSString alloc] initWithFormat:ns_format arguments:argp];
	// Since I already created the full string and used up the argp, I have to pass this.
	NSLog(@"%@", the_string);
	return (int)[the_string length];
}

static int LogWrapper_CustomPrintfToNSPuts(Logger* logger, void* userdata, const char* text)
{
	// NSLog doesn't have a puts equivalent.
	NSLog(@"%s", text);
	return (int)strlen(text);
}


@implementation LogWrapper


+ (instancetype) sharedInstance
{
    static dispatch_once_t once;
    static id shared_instance;

    dispatch_once(&once,
		^{
			shared_instance = [[self alloc] init];
		}
	);

    return shared_instance;
}

- (instancetype) init
{
	self = [super self];
	if(nil != self)
	{
/*
		NSArray* directory_paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString* documents_directory = [directory_paths firstObject];
		NSString* test_file = [documents_directory stringByAppendingPathComponent:@"test262_runlog.txt"];

		loggerFile = Logger_CreateWithName([test_file fileSystemRepresentation]);
*/
		loggerFile = NULL;
		loggerNative = Logger_Create();
		loggerSocket = Logger_Create();
		Logger_Disable(loggerSocket);

		// These are backdoor extensions I added to Logger awhile back to allow overriding the built in print functions.
		// Use these to override fprintf/fputs with NSLog.
		loggerNative->customPrintf = LogWrapper_CustomPrintfToNSLog;
		loggerNative->customPrintfv = LogWrapper_CustomPrintfToNSLogv;
		loggerNative->customPuts = LogWrapper_CustomPrintfToNSPuts;

		// Since NSLog always prints a timestamp, printing newline padding between log messages just makes more clutter, so disable newline injection.
		Logger_SetNewLines(loggerNative, 0, 0);

		// Thanks to Unix, where everything is a file, I'll just convert a socket into a FILE* with fdopen.
//		int fd = socket(AF_INET, SOCK_STREAM, 0);
//		FILE *fp = fdopen(fd, "a");

	}
	return self;
}

- (void) dealloc
{
	Logger_Free(loggerFile);
	Logger_Free(loggerNative);
	Logger_Free(loggerSocket);
}

- (void) flush
{
	Logger_Flush(loggerFile);
	Logger_Flush(loggerNative);
	Logger_Flush(loggerSocket);
}

- (BOOL) openNewFile
{
	NSArray* directory_paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* documents_directory = [directory_paths firstObject];
	NSString* test_file = [documents_directory stringByAppendingPathComponent:@"test262_runlog.txt"];
	return [self openNewFileWithName:test_file];
}

- (BOOL) openNewFileWithName:(NSString*)full_path_and_file
{
	// close the existing one if it exists
	if(NULL != loggerFile)
	{
		Logger_Free(loggerFile);
		loggerFile = NULL;
	}
	loggerFile = Logger_CreateWithName([full_path_and_file fileSystemRepresentation]);
	if(NULL != loggerFile)
	{
		[self setLogFilePathAndName:full_path_and_file];
		return YES;
	}
	else
	{
		[self setLogFilePathAndName:nil];
		return NO;
	}
}

- (void) closeFile
{
	Logger_Free(loggerFile);
	loggerFile = NULL;
	[self setLogFilePathAndName:nil];
}

@end
