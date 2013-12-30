
#import "MyHTTPConnection.h"
#import "HTTPMessage.h"
#import "HTTPDataResponse.h"
#import "DDNumber.h"
//#import "HTTPLogging.h"

#import "HTTPDynamicFileResponse.h"
#import "HTTPFileResponse.h"

//#import "AppDelegate.h"
#import "LogStreamWindowController.h"

// Log levels : off, error, warn, info, verbose
// Other flags: trace
//static const int httpLogLevel = HTTP_LOG_LEVEL_VERBOSE; // | HTTP_LOG_FLAG_TRACE;

@interface MyHTTPConnection ()
{
    NSUInteger runningByteCount;
    NSUInteger totalByteCount;
}

@property(nonatomic, copy) NSString* filePath;
@property(nonatomic, copy) NSString* fullPathAndFile;
@property(nonatomic, retain) NSFileHandle* storeFile;

@end

/**
 * All we have to do is override appropriate methods in HTTPConnection.
 **/

@implementation MyHTTPConnection

- (BOOL)supportsMethod:(NSString *)method atPath:(NSString *)path
{
//	HTTPLogTrace();
	
	// Add support for POST
	
	if ([method isEqualToString:@"POST"])
	{
		[self setFilePath:path];

//		if ([path isEqualToString:@"/upload.html"])
		{
			return YES;
		}
	}
	
	return [super supportsMethod:method atPath:path];
}

- (BOOL)expectsRequestBodyFromMethod:(NSString *)method atPath:(NSString *)path
{
//	HTTPLogTrace();
	
	// I'm learning experimentally that NSURLSession for upload tasks are setting values I can't control.
	if([method isEqualToString:@"POST"])
    {
        NSString* content_type = [request headerField:@"Content-Type"];
		if([content_type isEqualToString:@"application/octet-stream"])
        {
			return YES;
        }
    }
    return [super expectsRequestBodyFromMethod:method atPath:path];
}
/*
- (NSObject<HTTPResponse> *)httpResponseForMethod:(NSString*)method URI:(NSString*)path
{
//	HTTPLogTrace();


	if ([method isEqualToString:@"POST"])
	{
        [self setFilePath:path];
#if 0
		// this method will generate response with links to uploaded file
		NSMutableString* filesStr = [[NSMutableString alloc] init];

		for( NSString* filePath in uploadedFiles ) {
			//generate links
			[filesStr appendFormat:@"<a href=\"%@\"> %@ </a><br/>",filePath, [filePath lastPathComponent]];
		}
		NSString* templatePath = [[config documentRoot] stringByAppendingPathComponent:@"upload.html"];
		NSDictionary* replacementDict = [NSDictionary dictionaryWithObject:filesStr forKey:@"MyFiles"];
		// use dynamic file response to apply our links to response template
		return [[HTTPDynamicFileResponse alloc] initWithFilePath:templatePath forConnection:self separator:@"%" replacementDictionary:replacementDict];
#endif
	}
#if 0
	if( [method isEqualToString:@"GET"] && [path hasPrefix:@"/upload/"] ) {
		// let download the uploaded files
		return [[HTTPFileResponse alloc] initWithFilePath: [[config documentRoot] stringByAppendingString:path] forConnection:self];

	}
#endif
	return [super httpResponseForMethod:method URI:path];
}
*/

- (void)prepareForBodyWithSize:(UInt64)contentLength
{
//	HTTPLogTrace();

#if 0
	// set up mime parser
    NSString* boundary = [request headerField:@"boundary"];
    parser = [[MultipartFormDataParser alloc] initWithBoundary:boundary formEncoding:NSUTF8StringEncoding];
    parser.delegate = self;

	uploadedFiles = [[NSMutableArray alloc] init];
#else
	NSLog(@"contentLength=%llu", contentLength);
//	uploadedFiles = [[NSMutableArray alloc] init];
	NSString* filename = @"mydownload.txt";
/*
    if ( (nil == filename) || [filename isEqualToString: @""] ) {
        // it's either not a file part, or
		// an empty form sent. we won't handle it.
		return;
	}
*/
//	NSTemporaryDirectory()
	NSArray* directory_paths = NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES);
	NSString* downloads_directory = [directory_paths objectAtIndex:0];
	NSString* file_name_base = [[self filePath] stringByDeletingPathExtension];
	NSString* file_name_extension = [[self filePath] pathExtension];
	NSString* full_path_and_file = [downloads_directory stringByAppendingPathComponent:[self filePath]];

	NSUInteger collision_counter = 2;
	do
	{
		if(YES == [[NSFileManager defaultManager] fileExistsAtPath:full_path_and_file])
		{
			full_path_and_file = [downloads_directory stringByAppendingPathComponent:[NSString stringWithFormat:@"%@-%lu.%@", file_name_base, (unsigned long)collision_counter, file_name_extension]];
			collision_counter++;
		}
		else
		{
			break;
		}
	} while(1);

	if(![[NSFileManager defaultManager] createFileAtPath:full_path_and_file contents:nil attributes:nil])
	{
		[self setStoreFile:nil];
		NSLog(@"Could not create file at path: %@", full_path_and_file);

	}
	[self setStoreFile:[NSFileHandle fileHandleForWritingAtPath:full_path_and_file]];

	NSLog(@"Saving file to %@", full_path_and_file);
	[self setFullPathAndFile:full_path_and_file];

/*

    NSString* upload_dir_path = [[config documentRoot] stringByAppendingPathComponent:@"upload"];
	
	BOOL is_dir = YES;
	if (![[NSFileManager defaultManager]fileExistsAtPath:upload_dir_path isDirectory:&is_dir ])
    {
		[[NSFileManager defaultManager]createDirectoryAtPath:upload_dir_path withIntermediateDirectories:YES attributes:nil error:nil];
	}
	
    NSString* file_path = [upload_dir_path stringByAppendingPathComponent: filename];
    if( [[NSFileManager defaultManager] fileExistsAtPath:file_path] ) {
        [self setStoreFile:nil];
        NSLog(@"Ooops, file already exists");
    }
    else {
        //		HTTPLogVerbose(@"Saving file to %@", file_path);
        NSLog(@"Saving file to %@", file_path);

		if(![[NSFileManager defaultManager] createDirectoryAtPath:upload_dir_path withIntermediateDirectories:true attributes:nil error:nil]) {
//			HTTPLogError(@"Could not create directory at path: %@", file_path);
            NSLog(@"Could not create directory at path: %@", file_path);
		}
		if(![[NSFileManager defaultManager] createFileAtPath:file_path contents:nil attributes:nil]) {
//			HTTPLogError(@"Could not create file at path: %@", file_path);
            NSLog(@"Could not create directory at path: %@", file_path);
		}
        NSLog(@"setStoreFile: %@", file_path);

		[self setStoreFile:[NSFileHandle fileHandleForWritingAtPath:file_path]];
//		[uploadedFiles addObject: [NSString stringWithFormat:@"/upload/%@", filename]];
    }
*/

	//	totalByteCount = contentLength;
	// contentLength is NSIntegerMax if chunked mode is set and throws away Content-Length.
	// I want the content length for the progress meter so get the value manually.
	NSString* content_length = [request headerField:@"Content-Length"];
	totalByteCount = [content_length longLongValue];
	if(totalByteCount == 0)
	{
		totalByteCount = NSIntegerMax;
	}
	runningByteCount = 0;
	dispatch_async(dispatch_get_main_queue(),
		^{
			// FIXME: This is a hack and won't handle multiple simultaneous downloads.
//			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
//			NSProgressIndicator* progress_indicator = [app_delegate progressIndicator];
			NSProgressIndicator* progress_indicator = [[self logStreamWindowController]  progressIndicator];

			if(totalByteCount == NSIntegerMax)
			{
				[progress_indicator setIndeterminate:YES];
			}
			else
			{
				[progress_indicator setIndeterminate:NO];
				[progress_indicator setMaxValue:totalByteCount];
				[progress_indicator setMinValue:0];
			}
			[progress_indicator setDoubleValue:0];
			[progress_indicator setUsesThreadedAnimation:YES];
			[progress_indicator startAnimation:nil];
		}
    );
#endif
	
}

- (void) processBodyData:(NSData*)post_data_chunk
{
//	HTTPLogTrace();
    // append data to the parser. It will invoke callbacks to let us handle
    // parsed data.
#if 0
    [parser appendData:post_data_chunk];
#endif
	if([self storeFile] != nil)
    {
		[[self storeFile] writeData:post_data_chunk];

		runningByteCount = runningByteCount + [post_data_chunk length];
		dispatch_async(dispatch_get_main_queue(),
			^{
				// FIXME: This is a hack and won't handle multiple simultaneous downloads.
//				AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
				NSProgressIndicator* progress_indicator = [[self logStreamWindowController]  progressIndicator];
				[progress_indicator setDoubleValue:(double)runningByteCount];
			}
	    );
//        NSLog(@"processBodyData length: %lu", (unsigned long)[post_data_chunk length]);
	}
}

- (void)finishBody
{
	// Override me to perform any final operations on an upload.
	// For example, if you were saving the upload to disk this would be
	// the hook to flush any pending data to disk and maybe close the file.
	
	// as the file part is over, we close the file.
	[[self storeFile] closeFile];
	[self setStoreFile:nil];
	NSLog(@"finishBody");
	dispatch_async(dispatch_get_main_queue(),
		^{
			// FIXME: This is a hack and won't handle multiple simultaneous downloads.
//			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
//			NSProgressIndicator* progress_indicator = [app_delegate progressIndicator];
			NSProgressIndicator* progress_indicator = [[self logStreamWindowController]  progressIndicator];

			[progress_indicator stopAnimation:nil];

			// Bounce the downloads stack
			[[NSDistributedNotificationCenter defaultCenter] postNotificationName:@"com.apple.DownloadFileFinished" object:[self fullPathAndFile]];
			
			// Bounce the dock icon
//			[[NSApplication sharedApplication] requestUserAttention:NSInformationalRequest];

			// Post a notification
			NSUserNotification* user_notification = [[NSUserNotification alloc] init];
			[user_notification setTitle:@"Test262 Log Extraction Complete"];
			NSString* info_text = [NSString stringWithFormat:@"File saved to %@", [self fullPathAndFile]];
			[user_notification setInformativeText:info_text];
			[user_notification setSoundName:NSUserNotificationDefaultSoundName];
			[[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification:user_notification];

		}
	);
}



@end
