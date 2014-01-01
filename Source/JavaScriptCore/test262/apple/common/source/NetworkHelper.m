//
//  NetworkHelper.m
//  Test262
//
//  Created by Eric Wing on 1/1/14.
//  Copyright (c) 2014 JavaScriptCore. All rights reserved.
//

#import "NetworkHelper.h"
#include "SocketServer.h"


@interface NetworkHelper () <NSNetServiceDelegate>
@property(assign) int serverSocket;
@property(assign) uint16_t serverPort;
@property(assign) struct SocketServer_UserData* serverUserData;
@property(assign) SimpleThread* serverAcceptThread;


@property(nonatomic, strong, readwrite) NSNetService* netService;
@property(nonatomic, strong, readwrite) NSString* serviceName;
@property(assign) _Bool isServerStarted;

@end

@implementation NetworkHelper


- (instancetype) init
{
	self = [super self];
	if(nil != self)
	{
		// In principle, we should allow the user to specify their own name
//		_serviceName = [[UIDevice currentDevice] name];
		

	}
	return self;
}

- (void) startServer
{
	if([self isServerStarted])
	{
		return;
	}
	SocketServer_CreateSocketAndListen(&_serverSocket, &_serverPort);
	NSLog(@"socket: %d, port: %d", _serverSocket, _serverPort);
	_serverUserData = (struct SocketServer_UserData*)calloc(1, sizeof(struct SocketServer_UserData));
	_serverUserData->serverSocket = _serverSocket;
	// This pointer is how we will signal the server thread to stop
	_serverUserData->shouldKeepRunning = 1;
	_serverAcceptThread = SocketServer_RunAcceptLoopInThread(_serverUserData);

	// Advertise with Bonjour
//	_netService = [[NSNetService alloc] initWithDomain:@"" type:@"_test262logging._tcp." name:[self serviceName] port:_serverPort];

	// In principle, we should allow the user to specify their own name. Empty string picks the computer's default.
	_netService = [[NSNetService alloc] initWithDomain:@"" type:@"_test262logging._tcp." name:@"" port:_serverPort];
	[_netService setDelegate:self];
	[_netService publishWithOptions:0];

	[self setIsServerStarted:true];
}

- (void) stopServer
{
	if( ! [self isServerStarted])
	{
		return;
	}
	// this needs to be before [_netService stop] because it is infinitely recursing between this and its netServiceDidStop: delegate
	[self setIsServerStarted:false];

	int thread_status = 0;
	// This pointer is how we will signal the server thread to stop
	_serverUserData->shouldKeepRunning = 0;
	// I think this will disrupt the accept() blocking forcing the loop to continue
	close(_serverSocket);


	[_netService stop];

	// wait for the thread to end
	SimpleThread_WaitThread(_serverAcceptThread, &thread_status);

}


- (void) netServiceDidPublish:(NSNetService*)the_sender
    // An NSNetService delegate callback that's called when the service is successfully 
    // registered on the network.  We set our service name to the name of the service 
    // because the service might be been automatically renamed by Bonjour to avoid 
    // conflicts.
{
	NSLog(@"%@", NSStringFromSelector(_cmd));
	// Do we want to save the name in case the app is backgrounded to try to reclaim the same name later?
//    [self setServiceName:[the_sender name]];
}

- (void) netService:(NSNetService*)the_sender didNotPublish:(NSDictionary*)error_dict
    // An NSNetService delegate callback that's called when the service fails to 
    // register on the network.  We respond by shutting down our entire network 
    // service.
{
	NSLog(@"%@", NSStringFromSelector(_cmd));
	NSLog(@"error_dict %@", error_dict);
	[self stopServer];
}

- (void) netServiceDidStop:(NSNetService*)the_sender
    // An NSNetService delegate callback that's called when the service spontaneously 
    // stops.  This rarely happens on OS X but, regardless, we respond by shutting 
    // down our entire network service.
{
	NSLog(@"%@", NSStringFromSelector(_cmd));
	[self stopServer];
}

@end
