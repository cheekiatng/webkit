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
		return;
	}
	NSString* log_file_location_string = [object_providing_log_file_path logFileLocationString];
	NSString* filename_without_path = [log_file_location_string lastPathComponent];

	// This points to a statically stored char array within inet_ntoa() so that each time you call inet_ntoa() it will overwrite the last IP address you asked for.
	const char* ip_address_string = inet_ntoa(addr.sin_addr);

	NSString* url_string = [NSString stringWithFormat: @"http://%s:%d/%@",
		ip_address_string,
		http_server_port,
		filename_without_path
	];

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


@implementation NetworkHelperForAppDelegate

@end
