//
//  NetworkHelperForAppDelegate.m
//  Test262
//
//  Created by Eric Wing on 1/1/14.
//  Copyright (c) 2014 JavaScriptCore. All rights reserved.
//

#import "NetworkHelperForAppDelegate.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#import "AppDelegate.h"
#import "LogWrapper.h"

#include <TargetConditionals.h>

#ifdef __APPLE__
/*
	#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
		#include <AudioToolbox/AudioToolbox.h>
	#else
		#include <OpenAL/MacOSX_OALExtensions.h>
	#endif
*/
#endif


// Watch out: Signal handlers are global
// If you are automatically breaking on SIGPIPE due to writing on broken sockets, see this to make Xcode not do that:
// http://stackoverflow.com/questions/10431579/permanently-configuring-lldb-in-xcode-4-3-2-not-to-stop-on-signals
void NetworkHelperForAppDelegate_GlobalSignalHandlerForSIGPIPE(int sig)
{
	NSLog(@"NetworkHelperForAppDelegate_GlobalSignalHandlerForSIGPIPE");
#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
	id<NetworkHelperForAppDelegate> app_delegate = (id<NetworkHelperForAppDelegate>)[[UIApplication sharedApplication] delegate];
#else
	id<NetworkHelperForAppDelegate> app_delegate = (id<NetworkHelperForAppDelegate>)[[NSApplication sharedApplication] delegate];
#endif
	LogWrapper* log_wrapper = [app_delegate logWrapper];
	Logger_Disable(log_wrapper->loggerSocket);
	// I think I need an atomic set on LoggerWrapper because there might be a theoretical race condition between freeing the Logger and setting the pointer to NULL in the wrapper.
	Logger* temp_logger = log_wrapper->loggerSocket;
	log_wrapper->loggerSocket = NULL;
	Logger_Free(temp_logger);

}

void NetworkHelperForAppDelegate_UploadFile(int accepted_socket, uint16_t http_server_port, void* user_data)
{
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	int err = getpeername(accepted_socket, (struct sockaddr*)&addr, &addr_len);
	if(err != 0)
	{
		NSLog(@"getpeername failed for NetworkHelperForAppDelegate_UploadFile");
		return;
	}

	// This is expected to be the AppDelegate, though it can be anything that persists
	// through the life of the program and responds to the appropriate selector.
	id<NetworkHelperForAppDelegate> object_providing_log_file_path = (__bridge id)user_data;
	// I made this a protocol because ARC is throwing a hissy-fit about all the runtime stuff.
	// http://stackoverflow.com/questions/6224976/how-to-get-rid-of-the-undeclared-selector-warning
	// http://stackoverflow.com/questions/7017281/performselector-may-cause-a-leak-because-its-selector-is-unknown?lq=1
//	SEL selector_for_logFileLocationString = sel_registerName("logFileLocationString");
	SEL selector_for_logFileLocationString = @selector(logFileLocationString);
	if( ! [object_providing_log_file_path respondsToSelector:selector_for_logFileLocationString])
	{
		NSLog(@"Invalid userdata object provided to NetworkHelperForAppDelegate_UploadFile");
		assert([object_providing_log_file_path respondsToSelector:selector_for_logFileLocationString]);
		return;
	}
	NSString* log_file_location_string = [object_providing_log_file_path logFileLocationString];
	NSString* filename_without_path = [log_file_location_string lastPathComponent];

	NSString* service_name = @"";
	if( ! [object_providing_log_file_path respondsToSelector:@selector(serviceName)])
	{
		NSLog(@"Invalid userdata object provided to NetworkHelperForAppDelegate_UploadFile");
		// I'll make this one optional
		// return;
	}
	else
	{
		service_name = [object_providing_log_file_path serviceName];
	}
	// This points to a statically stored char array within inet_ntoa() so that each time you call inet_ntoa() it will overwrite the last IP address you asked for.
	const char* ip_address_string = inet_ntoa(addr.sin_addr);

	NSString* url_string = [NSString stringWithFormat: @"http://%s:%d/%@-%@",
		ip_address_string,
		http_server_port,
		service_name,
		filename_without_path
	];
	url_string = [url_string stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];


	NSURL* the_url = [NSURL URLWithString:url_string];
    NSLog(@"client url: %@", url_string);

	NSMutableURLRequest *url_request = [[NSMutableURLRequest alloc] initWithURL:the_url];
	//    [url_request setHTTPMethod:@"PUT"];
    [url_request setHTTPMethod:@"POST"];
	// 3

	//    self.uploadTask = [upLoadSession uploadTaskWithRequest:request fromData:imageData];
    NSLog(@"file: %@", log_file_location_string);
    NSURL* file_url = [NSURL fileURLWithPath:log_file_location_string isDirectory:NO];
    NSLog(@"file url: %@", file_url);

	NSURLSessionConfiguration* url_session_config = [NSURLSessionConfiguration defaultSessionConfiguration];
	//        config.HTTPMaximumConnectionsPerHost = 1;
	//        [config setHTTPAdditionalHeaders:@{@"Authorization": [Dropbox apiAuthorizationHeader]}];

//	NSURLSession* upload_session = [NSURLSession sessionWithConfiguration:url_session_config delegate:self delegateQueue:nil];
	NSURLSession* upload_session = [NSURLSession sessionWithConfiguration:url_session_config delegate:nil delegateQueue:nil];



	//    self.uploadTask = [upLoadSession uploadTaskWithRequest:url_request fromFile:file_url];
    NSURLSessionUploadTask* upload_task = [upload_session uploadTaskWithRequest:url_request fromFile:file_url
		completionHandler:^(NSData *data, NSURLResponse *response, NSError *error)
		{
			NSLog(@"completed upload: %@, %@", response, error);
		}
	];
	// 4
	//        self.uploadView.hidden = NO;
	//        [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:YES];

	// 5
	[upload_task resume];

	// TODO:
	// 1) Set up the sessionWithConfiguration delegate
	// 2) Save the upload_task so we can monitor the progress
	
}

void NetworkHelperForAppDelegate_OpenLogStream(int accepted_socket, void* opaque_log_wrapper, void* user_data)
{
	LogWrapper* log_wrapper = (__bridge LogWrapper*)opaque_log_wrapper;

	// disable SIGPIPE errors (crashes) on writing to a dead socket. I need this since I'm blindly throwing stuff at Logger.
	// decided to move to platform specific code:
	// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
	// If you are automatically breaking on SIGPIPE due to writing on broken sockets, see this to make Xcode not do that:
	// http://stackoverflow.com/questions/10431579/permanently-configuring-lldb-in-xcode-4-3-2-not-to-stop-on-signals
#if 0
	int set_value = 1;
	setsockopt(accepted_socket, SOL_SOCKET, SO_NOSIGPIPE, &set_value, sizeof(set_value));
#else
    struct sigaction sa;
    sa.sa_handler = NetworkHelperForAppDelegate_GlobalSignalHandlerForSIGPIPE;
    sa.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
	{
		perror("sigaction");
		NSLog(@"can't setup signal handler");

		int set_value = 1;
		setsockopt(accepted_socket, SOL_SOCKET, SO_NOSIGPIPE, &set_value, sizeof(set_value));
    }
#endif

	// Thanks to Unix, where everything is a file, I'll just convert a socket into a FILE* with fdopen.
	// int fd = socket(AF_INET, SOCK_STREAM, 0);
	// fdopen or fdreopen?
	FILE* file_pointer_from_socket = fdopen(accepted_socket, "a");
	// Only allow one right now. Need to figure out if it is worth supporting more.
	if(NULL != log_wrapper->loggerSocket)
	{
		Logger_Free(log_wrapper->loggerSocket);
	}
	// the "" is a hack to deal with that there are no file names in sockets, but Logger currently expects files except for stdout/stderr. Logger should be extended to handle this case.
	log_wrapper->loggerSocket = Logger_CreateWithHandle(file_pointer_from_socket, "");

	Logger_EnableAutoFlush(log_wrapper->loggerSocket);

	// Test/debug message. Sending it directly to the socketLogger instead of all 3 loggers.
	Logger_LogEventNoFormat(log_wrapper->loggerSocket, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Debug", "Log Stream socket connected to Logger via socket->fdopen (this not echoed to other loggers)");

	
}

@implementation NetworkHelperForAppDelegate

@end
