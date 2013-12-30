//
//  ResolveForDownloadDelegate.m
//  Test262LogClient
//
//  Created by Eric Wing on 12/27/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "ResolveForDownloadDelegate.h"
#import "HTTPServer.h"
#import "LogStreamWindowController.h"
#import "AppDelegate.h"
#import "MyHTTPConnection.h"

#include <arpa/inet.h>


@implementation ResolveForDownloadDelegate


// Sent when addresses are resolved
- (void) netServiceDidResolveAddress:(NSNetService *)net_service
{
    // Make sure [netService addresses] contains the
    // necessary connection information
	[super netServiceDidResolveAddress:net_service];

	[self connectToServerAndSendHttpServerInfomation:net_service];
}


- (void) connectToServerAndSendHttpServerInfomation:(NSNetService*)net_service
{

	dispatch_async(dispatch_get_main_queue(),
		^{
			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
			NSString* identifier_name = [net_service name];
			LogStreamWindowController* window_controller = nil;

			window_controller = [app_delegate logStreamWindowControllerExistsForName:identifier_name];
			if(nil != window_controller)
			{
				[window_controller showWindow:nil];
			}
			else
			{


				window_controller = [[LogStreamWindowController alloc] initWithWindowNibName:@"LogStreamWindowController"];
				NSWindow* the_window = [window_controller window];
				[the_window setTitle:[net_service name]];
				[window_controller setNetService:net_service];

				[app_delegate addWindowControllerToActiveList:window_controller];

				// capturing window_controller strongly in this block is likely to lead to a retain cycle
				typeof(window_controller) __weak weak_window_controller = window_controller;

				[window_controller setWindowDidCloseCompletionBlock:
					^{
						AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
						[app_delegate removeWindowControllerFromActiveList:weak_window_controller];

						}
				];

				[window_controller showWindow:nil];

			}
			

			dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
				^{
					uint16_t http_server_port = [[self httpServer] listeningPort];

					uint16_t http_server_port_networkorder = htons([[self httpServer] listeningPort]);

					// capturing window_controller strongly in this block is likely to lead to a retain cycle
					typeof(window_controller) __weak weak_window_controller = window_controller;


					[[self httpServer] setConnectionClassInstantiationCallback:
						^(HTTPConnection* http_connection_instance)
						{
					 		if([http_connection_instance isKindOfClass:[MyHTTPConnection class]])
							{
								NSLog(@"Got MyHTTPConnection instantiation callback");
								[(MyHTTPConnection*)http_connection_instance setLogStreamWindowController:weak_window_controller];
							}

						}
					];

					// dispatch_async
					// Remember to pull out shared ivar values if possible to avoid the need for syncing
					int sock_fd = Test262Helper_ConnectToServer(net_service);
					if(sock_fd == -1)
					{
						NSLog(@"Failed to connect, not sending server info");
						[self handleConnectError:net_service withErrno:errno];
						return;
					}

					send(sock_fd, &http_server_port_networkorder, sizeof(http_server_port_networkorder), 0);

					NSLog(@"sent port info: host:%d, network:%d", http_server_port, http_server_port_networkorder);
				}
			);

		}
	);

}


@end
