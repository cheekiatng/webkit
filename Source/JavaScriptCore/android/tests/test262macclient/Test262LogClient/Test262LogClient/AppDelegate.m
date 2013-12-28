//
//  AppDelegate.m
//  Test262LogClient
//
//  Created by Eric Wing on 12/24/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "AppDelegate.h"

#import "HTTPServer.h"
#import "DDLog.h"
#import "DDTTYLogger.h"
#import "MyHTTPConnection.h"
#import "HTTPServer.h"
#import "DDLog.h"
#import "DDTTYLogger.h"

#import "ResolveDelegate.h"
#import "ResolveForDownloadDelegate.h"
#import "ResolveForLogStreamDelegate.h"

#import "LogStreamWindowController.h"

static const int ddLogLevel = LOG_LEVEL_VERBOSE;

@interface AppDelegate () <NSNetServiceBrowserDelegate, NSUserNotificationCenterDelegate>
{
    HTTPServer* httpServer;
}
@property (nonatomic, strong, readwrite) NSNetServiceBrowser *  browser;
@property (nonatomic, strong, readwrite) IBOutlet NSArrayController *   servicesArray;
@property (nonatomic, strong, readonly ) NSMutableSet *         pendingServicesToAdd;
@property (nonatomic, strong, readonly ) NSMutableSet *         pendingServicesToRemove;

@property (nonatomic, strong, readonly ) NSMutableSet *         services;
@property (nonatomic, strong, readonly ) NSArray *              sortDescriptors;
@property (nonatomic, copy,   readwrite) NSString *             longStatus;
@property (weak) IBOutlet NSTableView* tableView;
@property (weak) IBOutlet NSButton* downloadButton;

// I need to root these delegate instances somewhere or ARC will release my temporary instances
@property (nonatomic, strong, readonly ) ResolveForDownloadDelegate*              resolveForDownloadDelegate;
@property (nonatomic, strong, readonly ) ResolveForLogStreamDelegate*              resolveForLogStreamDelegate;

@property (nonatomic, strong, readonly ) NSMutableArray* listOfActiveLogStreamWindowControllers;

@end
@implementation AppDelegate

- (id)init
{
    self = [super init];
    if (self != nil) {
        
        self->_services = [[NSMutableSet alloc] init];

        self->_pendingServicesToAdd = [[NSMutableSet alloc] init];
        self->_pendingServicesToRemove = [[NSMutableSet alloc] init];
        
        self->_sortDescriptors = @[
                                   [[NSSortDescriptor alloc] initWithKey:@"name"   ascending:YES selector:@selector(localizedStandardCompare:)],
                                   [[NSSortDescriptor alloc] initWithKey:@"domain" ascending:YES selector:@selector(localizedStandardCompare:)]
                                   ];

		_resolveForDownloadDelegate = [[ResolveForDownloadDelegate alloc] init];
		_resolveForLogStreamDelegate = [[ResolveForLogStreamDelegate alloc] init];

		_listOfActiveLogStreamWindowControllers = [[NSMutableArray alloc] init];


    }
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];

    [[self tableView] setDoubleAction:@selector(tableViewDoubleClicked:)];
    
    [self startBrowsing];
    [self startHttpServer];
}

// Overrides to always send notifications, even when in the foreground.
- (BOOL) userNotificationCenter:(NSUserNotificationCenter*)center shouldPresentNotification:(NSUserNotification *)notification
{
	return YES;
}

- (void) startHttpServer
{
    // Configure our logging framework.
	// To keep things simple and fast, we're just going to log to the Xcode console.
	[DDLog addLogger:[DDTTYLogger sharedInstance]];

	// Initalize our http server
	httpServer = [[HTTPServer alloc] init];


    /* This may seem a little indirect.
    The device will publish itself via Bonjour/Zeroconf, not this client.
    But this client will run an http server so the device can upload the file to here.
    The main reasons that the device advertises and not this client are that
    - The http server dependencies are relatively big
    - Building a good GUI on the device to pick a server to send to is harder to do on a constrained screen.
    Since this client is a desktop app, size and GUI issues don't apply.
    
    Thus this client will content to the device, and send over the port that this http server is running on so the device can open a new connection to communicate back as necessary.
    */


	// Tell the server to broadcast its presence via Bonjour.
	// This allows browsers such as Safari to automatically discover our service.
//	[httpServer setType:@"_http._tcp."];

	// Normally there's no need to run our server on any specific port.
	// Technologies like Bonjour allow clients to dynamically discover the server's port at runtime.
	// However, for easy testing you may want force a certain port so you can just hit the refresh button.
//    [httpServer setPort:12345];

	// Serve files from the standard Sites folder
	NSString *docRoot = [[[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"web"] stringByDeletingLastPathComponent];
	DDLogInfo(@"Setting document root: %@", docRoot);

	[httpServer setDocumentRoot:docRoot];

	[httpServer setConnectionClass:[MyHTTPConnection class]];

	NSError *error = nil;
	if(![httpServer start:&error])
	{
		DDLogError(@"Error starting HTTP Server: %@", error);
	}
}


- (void)startBrowsing
    // Starts a browse operation for our service type.
{
    assert(self.browser == nil);
    self.browser = [[NSNetServiceBrowser alloc] init];
    [self.browser setDelegate:self];
    // Passing in "" for the domain causes us to browse in the default browse domain
    [self.browser searchForServicesOfType:@"_test262logging._tcp." inDomain:@""];
//    [self.browser searchForServicesOfType:@"_test262logging._tcp." inDomain:@""];
//    [self.browser searchForServicesOfType:@"_wwdcpic2._tcp." inDomain:@""];
//    [self.browser searchForServicesOfType:@"_http._tcp." inDomain:@""];
}

- (void)stopBrowsingWithStatus:(NSString *)status
    // Stops the browser after some sort of fatal error, displaying 
    // the status message to the user.
{
    assert(status != nil);
    
    [self.browser setDelegate:nil];
    [self.browser stop];
    self.browser = nil;
    
    [self.pendingServicesToAdd removeAllObjects];
    [self.pendingServicesToRemove removeAllObjects];

    [self willChangeValueForKey:@"services"];
    [self.services removeAllObjects];
    [self  didChangeValueForKey:@"services"];
    
    self.longStatus = status;
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing
    // An NSNetService delegate callback that's called when we discover a service. 
    // We add this service to our set of pending services to add and, if there are 
    // no more services coming, we add that set to our services set, triggering the 
    // necessary KVO notification.
{
    assert(aNetServiceBrowser == self.browser);
    #pragma unused(aNetServiceBrowser)
    
    [self.pendingServicesToAdd addObject:aNetService];
    
    if ( ! moreComing ) {
        NSSet * setToAdd;

        setToAdd = [self.pendingServicesToAdd copy];
        assert(setToAdd != nil);
        [self.pendingServicesToAdd removeAllObjects];

        [self willChangeValueForKey:@"services" withSetMutation:NSKeyValueUnionSetMutation usingObjects:setToAdd];
        [self.services unionSet:setToAdd];
        [self didChangeValueForKey:@"services" withSetMutation:NSKeyValueUnionSetMutation usingObjects:setToAdd];

		// Trigger a change notification for the hostName field which only gets set in the NSNetService object after a resolve.
        [self willChangeValueForKey:@"services.arrangedObjects.hostName" withSetMutation:NSKeyValueUnionSetMutation usingObjects:setToAdd];
        [self didChangeValueForKey:@"services.arrangedObjects.hostName" withSetMutation:NSKeyValueUnionSetMutation usingObjects:setToAdd];

		// I think we use ChangeValueForKey:withSetMutation instead of ArrayController addObject because we don't want to blindly add objects to the array in case there are duplicates.
//        [self.servicesArray addObject:aNetService];
    }
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing
    // An NSNetService delegate callback that's called when a service goes away. 
    // We add this service to our set of pending services to remove and, if there are 
    // no more services coming (well, going :-), we remove that set to our services set, 
    // triggering the necessary KVO notification.
{
    assert(aNetServiceBrowser == self.browser);
    #pragma unused(aNetServiceBrowser)

    [self.pendingServicesToRemove addObject:aNetService];
    
    if ( ! moreComing ) {
        NSSet * setToRemove;

        setToRemove = [self.pendingServicesToRemove copy];
        assert(setToRemove != nil);
        [self.pendingServicesToRemove removeAllObjects];

        [self willChangeValueForKey:@"services" withSetMutation:NSKeyValueMinusSetMutation usingObjects:setToRemove];
        [self.services minusSet:setToRemove];
        [self  didChangeValueForKey:@"services" withSetMutation:NSKeyValueMinusSetMutation usingObjects:setToRemove];
    }
}

- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)aNetServiceBrowser
    // An NSNetService delegate callback that's called when the service spontaneously 
    // stops.  This rarely happens on OS X but, regardless, we respond by shutting 
    // down our browser.
{
    assert(aNetServiceBrowser == self.browser);
    #pragma unused(aNetServiceBrowser)
    [self stopBrowsingWithStatus:@"Service browsing stopped."];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didNotSearch:(NSDictionary *)errorDict
    // An NSNetService delegate callback that's called when the browser fails 
    // completely.  We respond by shutting it down.
{
    assert(aNetServiceBrowser == self.browser);
    #pragma unused(aNetServiceBrowser)
    assert(errorDict != nil);
    #pragma unused(errorDict)
    [self stopBrowsingWithStatus:@"Service browsing failed."];
}

/*
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

}
*/
- (IBAction)tableRowClickedAction:(id)sender
// Called when user clicks a row in the services table.  If we're not already receiving,
// we kick off a receive.
{
#pragma unused(sender)
    // We test for a positive clickedRow to eliminate clicks in the column headers.
    if ( ([sender clickedRow] >= 0) && [[self.servicesArray selectedObjects] count] != 0) {
        NSNetService *  service;
        
        service = [[self.servicesArray selectedObjects] objectAtIndex:0];
        assert([service isKindOfClass:[NSNetService class]]);
        
//        [self startReceiveFromService:service];
    }
}

- (IBAction) downloadButtonClicked:(id)the_sender
{
    NSInteger selected_row = [[self tableView] selectedRow];
    [self resolveForDownloadForSelectedRow:selected_row];
}


- (IBAction) streamLogButtonClicked:(id)the_sender
{
    NSInteger selected_row = [[self tableView] selectedRow];
    [self resolveForLogStreamForSelectedRow:selected_row];
}


- (void) tableViewDoubleClicked:(id)the_sender
{
    NSInteger clicked_row = [the_sender clickedRow];
    [self resolveForLogStreamForSelectedRow:clicked_row];
}

- (void) resolveForDownloadForSelectedRow:(NSInteger)selected_row
{
//    NSInteger clicked_row = [[self tableView] clickedRow];
    // We test for a positive clickedRow to eliminate clicks in the column headers.
    if ( (selected_row >= 0) && [[self.servicesArray selectedObjects] count] != 0)
    {
        NSNetService *  service;
        
        service = [[self.servicesArray selectedObjects] objectAtIndex:0];
        assert([service isKindOfClass:[NSNetService class]]);
#if 1
        [service setDelegate:[self resolveForDownloadDelegate]];
        [service resolveWithTimeout:5.0];
#else
        NSInputStream *istream = nil;
        NSOutputStream *ostream = nil;

        [service getInputStream:&istream outputStream:&ostream];
        if (istream && ostream)
        {
            // Use the streams as you like for reading and writing.

        }
        else
        {
            NSLog(@"Failed to acquire valid streams");
        }

#endif

        //        [self startReceiveFromService:service];
//        [self startBackgroundDownloadWithNetService:service];
    }
}

- (void) resolveForLogStreamForSelectedRow:(NSInteger)selected_row
{
	//    NSInteger clicked_row = [[self tableView] clickedRow];
    // We test for a positive clickedRow to eliminate clicks in the column headers.
    if ( (selected_row >= 0) && [[self.servicesArray selectedObjects] count] != 0)
    {
        NSNetService *  service;

        service = [[self.servicesArray selectedObjects] objectAtIndex:0];
        assert([service isKindOfClass:[NSNetService class]]);

        [service setDelegate:[self resolveForLogStreamDelegate]];
        [service resolveWithTimeout:5.0];

    }
}

//////////////////


- (LogStreamWindowController*) logStreamWindowControllerExistsForName:(NSString*)window_name
{
	for(LogStreamWindowController* window_controller in _listOfActiveLogStreamWindowControllers)
	{
		NSWindow* the_window = [window_controller window];
		if([window_name isEqualToString:[the_window title]])
		{
			return window_controller;
		}
	}
	return nil;
}

- (void) addWindowControllerToActiveList:(LogStreamWindowController*)new_window_controller
{
	for(LogStreamWindowController* window_controller in _listOfActiveLogStreamWindowControllers)
	{
		if([window_controller isEqualTo:new_window_controller])
		{
			return;
		}
	}
	[_listOfActiveLogStreamWindowControllers addObject:new_window_controller];

}

- (void) removeWindowControllerFromActiveList:(LogStreamWindowController*)window_controller
{
	[_listOfActiveLogStreamWindowControllers removeObject:window_controller];
}


//////////////////

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>

static NSArray* ipv4Addresses(NSNetService* net_service)
{
    NSMutableArray* ipv4_addresses = [NSMutableArray array];
    NSArray* addresses = [net_service addresses];
    NSUInteger number_of_addresses = [addresses count];
    char addr[256];
    
    for(NSUInteger i = 0; i < number_of_addresses; i++)
    {
        struct sockaddr* socket_address = (struct sockaddr *)[[addresses objectAtIndex:i] bytes];
        
        if(socket_address && socket_address->sa_family == AF_INET)
        {
            if (inet_ntop(AF_INET, &((struct sockaddr_in *)socket_address)-> sin_addr, addr, sizeof(addr)))
            {
                uint16_t port = ntohs(((struct sockaddr_in *)socket_address)-> sin_port);
                
                [ipv4_addresses addObject:[NSString stringWithFormat:@"%s:%d", addr, port]];
            }
        }
    }
    
    return ipv4_addresses;
}

static NSURL* URLFromNetService(NSNetService* net_service, NSString* url_path)
{
    // build URL from host, port, and path
	NSString* url_string = [NSString
        stringWithFormat: @"http://%@:%ld/%@",
        [net_service hostName],
        (long)[net_service port],
        url_path
    ];
	NSURL* the_url = [NSURL URLWithString:url_string];
    return the_url;
    
//	NSURLRequest *request = [[NSURLRequest alloc] initWithURL:url];
}

@end
