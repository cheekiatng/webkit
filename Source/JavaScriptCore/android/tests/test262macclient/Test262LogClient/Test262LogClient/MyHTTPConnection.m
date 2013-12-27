
#import "MyHTTPConnection.h"
#import "HTTPMessage.h"
#import "HTTPDataResponse.h"
#import "DDNumber.h"
//#import "HTTPLogging.h"

#import "HTTPDynamicFileResponse.h"
#import "HTTPFileResponse.h"

// Log levels : off, error, warn, info, verbose
// Other flags: trace
//static const int httpLogLevel = HTTP_LOG_LEVEL_VERBOSE; // | HTTP_LOG_FLAG_TRACE;

@interface MyHTTPConnection ()


@property(nonatomic, copy) NSString* filePath;
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
}



@end
