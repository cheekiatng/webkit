//
//  ResolveForLogStream.m
//  Test262LogClient
//
//  Created by Eric Wing on 12/27/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "ResolveForLogStreamDelegate.h"

#include <arpa/inet.h>
#import "LogStreamWindowController.h"
#import "AppDelegate.h"

@interface ResolveForLogStreamDelegate ()

@property(assign) BOOL keepReceiving;
@property(assign) BOOL inReceivingLoop;
@property(assign) int currentSocketFD;

@end
@implementation ResolveForLogStreamDelegate

- (id) init
{
	self = [super init];
	if(nil != self)
	{
		_currentSocketFD = -1;
	}
	return self;
}
// Sent when addresses are resolved
- (void)netServiceDidResolveAddress:(NSNetService *)net_service
{
    // Make sure [netService addresses] contains the
    // necessary connection information
	[super netServiceDidResolveAddress:net_service];

	[self connectToServerAndSendStreamDirective:net_service];
}



- (void) connectToServerAndSendStreamDirective:(NSNetService*)net_service
{
	// If there was a previous socket connection, end it and start a new one.
	// This is to deal with the possibility that the connection died and the user wants to reconnect.
	// Some better smarts to detect whether still connected would be nice
	[self setKeepReceiving:NO];
	if([self currentSocketFD] >= 0)
	{
		// On Apple, shutdown doesn't seem required, but on Linux, it does.
		shutdown([self currentSocketFD], SHUT_RDWR);
		close([self currentSocketFD]);
		[self setCurrentSocketFD:-1];
	}

	dispatch_async(dispatch_get_main_queue(),
		^{
			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
			NSString* identifier_name = [net_service name];
			LogStreamWindowController* window_controller = nil;

			window_controller = [app_delegate logStreamWindowControllerExistsForName:identifier_name];
			if(nil != window_controller)
			{
//				[window_controller showWindow:nil];
//				return;
			}
			else
			{


				window_controller = [[LogStreamWindowController alloc] initWithWindowNibName:@"LogStreamWindowController"];
				NSWindow* the_window = [window_controller window];
				[the_window setTitle:[net_service name]];
				[window_controller setNetService:net_service];

				[app_delegate addWindowControllerToActiveList:window_controller];
			}
			// capturing window_controller strongly in this block is likely to lead to a retain cycle
			typeof(window_controller) __weak weak_window_controller = window_controller;

			[window_controller setWindowDidCloseCompletionBlock:
				^{
					AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
					[app_delegate removeWindowControllerFromActiveList:weak_window_controller];

					[self setKeepReceiving:NO];
					if([self currentSocketFD] >= 0)
					{
						shutdown([self currentSocketFD], SHUT_RDWR);
						close([self currentSocketFD]);
						[self setCurrentSocketFD:-1];
					}

				}
			];

			[window_controller showWindow:nil];


			// Make sure the previous loop is done.
			// Need a better sync technique
			while([self inReceivingLoop])
			{
				usleep(1000*10);
				NSLog(@"waiting for prior socket connection to close");
			}

			dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
				^{

					// dispatch_async
					// Remember to pull out shared ivar values if possible to avoid the need for syncing
					int sock_fd = Test262Helper_ConnectToServer(net_service);
					if(sock_fd == -1)
					{
						NSLog(@"Failed to connect, not sending server info");
						[self handleConnectError:net_service withErrno:errno];
						return;
					}
					[self setCurrentSocketFD:sock_fd];

					uint16_t command_directive = 0;
					// server knows to treat 0 as a special command
					send(sock_fd, &command_directive, sizeof(uint16_t), 0);
					


					const size_t MAX_RECV_BUFFER_SIZE = 65536+1;
					char* recv_buffer = malloc(MAX_RECV_BUFFER_SIZE * sizeof(char));
					[self setInReceivingLoop:YES];
					[self setKeepReceiving:YES];

					while(YES == [self keepReceiving])
					{
						ssize_t num_bytes = recv(sock_fd, recv_buffer, MAX_RECV_BUFFER_SIZE-1, 0);
						if(0 == num_bytes)
						{
							[self setKeepReceiving:NO];
							break;
						}
						else if(num_bytes < 0)
						{
							NSLog(@"error with recv, errno:(%d) %s", errno, strerror(errno));
							// Not sure to stop or continue?
							continue;
						}

	//					recv_buffer[num_bytes] = '\0';
	//					NSLog(@"recv from socket: %s", recv_buffer);
						// I need to copy the message now before the dispatch_async because the buffer could be modified before the following is run.
						NSString* log_message = [[NSString alloc] initWithBytes:recv_buffer length:num_bytes encoding:NSUTF8StringEncoding];
						if(nil == log_message)
						{
							NSLog(@"Unexpected nil string, num_bytes=%zd", num_bytes);
							recv_buffer[num_bytes] = '\0';
							NSLog(@"recv from socket: %s", recv_buffer);
							continue;
						}
						dispatch_async(dispatch_get_main_queue(),
							^{
								// I can't use this version because the buffer could change before this gets run
	//							[weak_window_controller postLogEvent:recv_buffer length:num_bytes];
								[weak_window_controller postLogEvent:log_message];
							}
						);
					}
					free(recv_buffer);
					[self setInReceivingLoop:NO];
					[self setCurrentSocketFD:-1];

				}
			);

		}
	);

}

@end
