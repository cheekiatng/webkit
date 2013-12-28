//
//  ResolveForDownloadDelegate.m
//  Test262LogClient
//
//  Created by Eric Wing on 12/27/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "ResolveForDownloadDelegate.h"
#import "HTTPServer.h"

#include <arpa/inet.h>


@implementation ResolveForDownloadDelegate


// Sent when addresses are resolved
- (void) netServiceDidResolveAddress:(NSNetService *)net_service
{
    // Make sure [netService addresses] contains the
    // necessary connection information
	[self connectToServerAndSendHttpServerInfomation:net_service];
}


- (void) connectToServerAndSendHttpServerInfomation:(NSNetService*)net_service
{
    uint16_t http_server_port = [[self httpServer] listeningPort];

    uint16_t http_server_port_networkorder = htons([[self httpServer] listeningPort]);

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


@end
