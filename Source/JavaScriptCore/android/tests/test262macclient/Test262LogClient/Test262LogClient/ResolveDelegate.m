//
//  ResolveDelegate.m
//  Test262LogClient
//
//  Created by Eric Wing on 12/27/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "ResolveDelegate.h"
#import "AppDelegate.h"
#import <Cocoa/Cocoa.h>

#include <arpa/inet.h>

int Test262Helper_ConnectToServer(NSNetService* net_service)
{
    int sock_fd = 0;
    _Bool did_connect = 0;

	//   struct addrinfo hints, *servinfo, *p;
	//    int rv;
	//    char s[INET6_ADDRSTRLEN];
    char addr[INET6_ADDRSTRLEN];

	/*
	 memset(&hints, 0, sizeof hints);
	 hints.ai_family = AF_UNSPEC;
	 hints.ai_socktype = SOCK_STREAM;

	 if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
	 fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	 return 1;
	 }
	 */

	//    NSMutableArray* ipv4_addresses = [NSMutableArray array];
    NSArray* addresses = [net_service addresses];
    NSUInteger number_of_addresses = [addresses count];

    // loop through all the results and connect to the first we can
    for(NSUInteger i = 0; i < number_of_addresses; i++)
    {
        struct sockaddr* socket_address = (struct sockaddr *)[[addresses objectAtIndex:i] bytes];

        if(socket_address && socket_address->sa_family == AF_INET)
        {
            if (inet_ntop(AF_INET, &((struct sockaddr_in *)socket_address)-> sin_addr, addr, sizeof(addr)))
            {
                uint16_t port = ntohs(((struct sockaddr_in *)socket_address)-> sin_port);
                NSLog(@"host port is %d", port);

				//               [ipv4_addresses addObject:[NSString stringWithFormat:@"%s:%d", addr, port]];

                if((sock_fd = socket(socket_address->sa_family, SOCK_STREAM, 0)) == -1)
                {
                    perror("client: socket");
 //                   NSLog(@"can't create socket");
                    continue;
                }

                if(connect(sock_fd, socket_address, INET_ADDRSTRLEN) == -1)
                {
                    close(sock_fd);
                    perror("client: connect");
 //                   NSLog(@"can't connect");
                    continue;
                }
                NSLog(@"connected on ipv4");
                did_connect = 1;
                break;
            }
        }
        else if(socket_address && socket_address->sa_family == AF_INET6)
        {
            if (inet_ntop(AF_INET6, &((struct sockaddr_in6*)socket_address)-> sin6_addr, addr, sizeof(addr)))
            {
                uint16_t port = ntohs(((struct sockaddr_in6 *)socket_address)-> sin6_port);
				NSLog(@"host port is %d", port);
                //               [ipv4_addresses addObject:[NSString stringWithFormat:@"%s:%d", addr, port]];
                //INET6_ADDRSTRLEN
                if((sock_fd = socket(socket_address->sa_family, SOCK_STREAM, 0)) == -1)
                {
                    perror("client: socket");
//                    NSLog(@"can't create socket");
                    continue;
                }

                if(connect(sock_fd, socket_address, INET6_ADDRSTRLEN) == -1)
                {
                    close(sock_fd);
                    perror("client: connect");
//                    NSLog(@"can't connect");
                    continue;
                }
                NSLog(@"connected on ipv6");
                did_connect = 1;
                break;
            }
        }
        else
        {
            continue;
        }
    }


    if(0 == did_connect)
    {
        NSLog(@"connect failed");
		sock_fd = -1;

    }

    return sock_fd;
}


@implementation ResolveDelegate

// Sent if resolution fails
- (void)netService:(NSNetService *)net_service
     didNotResolve:(NSDictionary *)error_dict
{

	    NSLog(@"An error occurred with service %@.%@.%@, error code=%@, error domain=%@",
	[net_service name], [net_service type], [net_service domain], [error_dict objectForKey:NSNetServicesErrorCode], [error_dict objectForKey:NSNetServicesErrorDomain]);
	dispatch_async(dispatch_get_main_queue(),
		^{
			// FIXME: This is a hack and won't handle multiple simultaneous downloads.
			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];


			// Handle error here
			NSAlert* alert = [[NSAlert alloc] init];
			[alert addButtonWithTitle:@"Dismiss"];
			//	[alert addButtonWithTitle:@"Cancel"];
			NSString* message_text = [NSString stringWithFormat:@"Could not connect to %@", [net_service name]];
			[alert setMessageText:message_text];
			NSString* info_text = [NSString stringWithFormat:@"Try again or try restarting the app on the device. Error code:%@, domain:%@", [error_dict objectForKey:NSNetServicesErrorCode], [error_dict objectForKey:NSNetServicesErrorDomain]];

			[alert setInformativeText:info_text];
			[alert setAlertStyle:NSInformationalAlertStyle];

			[alert beginSheetModalForWindow:[app_delegate window]
				modalDelegate:self
				didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
				contextInfo:nil
			];

		}
    );

	//   [services removeObject:netService];
}

- (void) handleConnectError:(NSNetService*)net_service withErrno:(int)error_no
{

	    NSLog(@"An error occurred with service %@.%@.%@, errno=%d",
	[net_service name], [net_service type], [net_service domain], error_no);
	dispatch_async(dispatch_get_main_queue(),
		^{
			// FIXME: This is a hack and won't handle multiple simultaneous downloads.
			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];


			// Handle error here
			NSAlert* alert = [[NSAlert alloc] init];
			[alert addButtonWithTitle:@"Dismiss"];
			//	[alert addButtonWithTitle:@"Cancel"];
			NSString* message_text = [NSString stringWithFormat:@"Could not connect to %@", [net_service name]];
			[alert setMessageText:message_text];
			NSString* info_text = [NSString stringWithFormat:@"Try again or try restarting the app on the device. errno:(%d) %s", error_no, strerror(error_no)];

			[alert setInformativeText:info_text];
			[alert setAlertStyle:NSInformationalAlertStyle];

			[alert beginSheetModalForWindow:[app_delegate window]
				modalDelegate:self
				didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
				contextInfo:nil
			];

		}
    );

}

// Verifies [netService addresses]
/*
- (BOOL)addressesComplete:(NSArray *)addresses forServiceType:(NSString *)serviceType
{
    // Perform appropriate logic to ensure that [netService addresses]
    // contains the appropriate information to connect to the service
    return YES;
}
*/



- (void)alertDidEnd:(NSAlert*)alert returnCode:(int)return_code contextInfo:(void*)context_info
{
//    [alert release];
}

@end
