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

static const int ddLogLevel = LOG_LEVEL_VERBOSE;

@interface AppDelegate () <NSNetServiceBrowserDelegate, NSStreamDelegate, NSURLSessionDataDelegate, NSNetServiceDelegate>
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

@property (strong, nonatomic) NSURLSessionDownloadTask* backgroundDownloadTask;



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
        
    }
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    
    [[self tableView] setDoubleAction:@selector(tableViewDoubleClicked:)];
    
    [self startBrowsing];
    [self startHttpServer];
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
//    [self.browser searchForServicesOfType:@"_test262logging._tcp." inDomain:@""];
//    [self.browser searchForServicesOfType:@"_wwdcpic2._tcp." inDomain:@""];
    [self.browser searchForServicesOfType:@"_http._tcp." inDomain:@""];
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
        [self  didChangeValueForKey:@"services" withSetMutation:NSKeyValueUnionSetMutation usingObjects:setToAdd];

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


- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

}
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

- (void) tableViewDoubleClicked:(id)the_sender
{
//    NSInteger clicked_row = [[self tableView] clickedRow];
    // We test for a positive clickedRow to eliminate clicks in the column headers.
    if ( ([the_sender clickedRow] >= 0) && [[self.servicesArray selectedObjects] count] != 0) {
        NSNetService *  service;
        
        service = [[self.servicesArray selectedObjects] objectAtIndex:0];
        assert([service isKindOfClass:[NSNetService class]]);
#if 1
        [service setDelegate:self];
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

// Sent when addresses are resolved
- (void)netServiceDidResolveAddress:(NSNetService *)net_service
{
    // Make sure [netService addresses] contains the
    // necessary connection information

    if ([self addressesComplete:[net_service addresses]
                 forServiceType:[net_service type]])
    {

//        [self startBackgroundDownloadWithNetService:service];

        [self connectToServerAndSendHttpServerInfomation:net_service];

    }



}

// Sent if resolution fails
- (void)netService:(NSNetService *)netService
     didNotResolve:(NSDictionary *)errorDict
{
    [self handleError:[errorDict objectForKey:NSNetServicesErrorCode] withService:netService];
 //   [services removeObject:netService];
}

// Verifies [netService addresses]
- (BOOL)addressesComplete:(NSArray *)addresses
           forServiceType:(NSString *)serviceType
{
    // Perform appropriate logic to ensure that [netService addresses]
    // contains the appropriate information to connect to the service
    return YES;
}

// Error handling code
- (void)handleError:(NSNumber *)error withService:(NSNetService *)service
{
    NSLog(@"An error occurred with service %@.%@.%@, error code = %d",
          [service name], [service type], [service domain], [error intValue]);
    // Handle error here
}



//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>

NSArray* ipv4Addresses(NSNetService* net_service)
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

NSURL* URLFromNetService(NSNetService* net_service, NSString* url_path)
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

int ConnectToServer(NSNetService* net_service)
{
    int sock_fd;
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
                    NSLog(@"can't create socket");
                    continue;
                }

                if(connect(sock_fd, socket_address, INET_ADDRSTRLEN) == -1)
                {
                    close(sock_fd);
                    perror("client: connect");
                    NSLog(@"can't connect");
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
                    NSLog(@"can't create socket");
                    continue;
                }

                if(connect(sock_fd, socket_address, INET6_ADDRSTRLEN) == -1)
                {
                    close(sock_fd);
                    perror("client: connect");
                    NSLog(@"can't connect");
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

    }

    return sock_fd;
}

- (void) connectToServerAndSendHttpServerInfomation:(NSNetService*)net_service
{
    uint16_t http_server_port = [httpServer listeningPort];
    
    uint16_t http_server_port_networkorder = htons([httpServer listeningPort]);

// dispatch_async
    // Remember to pull out shared ivar values if possible to avoid the need for syncing
    int sock_fd = ConnectToServer(net_service);
    if(sock_fd == -1)
    {
        NSLog(@"Failed to connect, not sending server info");
    }

    send(sock_fd, &http_server_port_networkorder, sizeof(http_server_port_networkorder), 0);

    NSLog(@"sent port info: host:%d, network:%d", http_server_port, http_server_port_networkorder);

}

- (void) startBackgroundDownloadWithNetService:(NSNetService*)net_service
{
    // Image CreativeCommons courtesy of flickr.com/charliematters
//    NSString *url = @"http://farm3.staticflickr.com/2831/9823890176_82b4165653_b_d.jpg";
    NSString* path_suffix = @"zebra.jpg";
    NSURL* the_url = URLFromNetService(net_service, path_suffix);
//    NSURL* the_url = URLFromNetService(net_service, @"");
    NSLog(@"server url: %@", the_url);
    NSURLRequest* url_request = [NSURLRequest requestWithURL:the_url];
    self.backgroundDownloadTask = [[self backgroundSession] downloadTaskWithRequest:url_request];
//    [self setDownloadButtonsAsEnabled:NO];
//    self.imageView.hidden = YES;
    // Start the download
    // Delay execution of my block for 10 seconds.
 //   dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"starting download");
        [[self backgroundDownloadTask] resume];
  //  });
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"starting download2");
        NSString* host = [net_service hostName];
        NSInteger port = 55555;
//        NSInteger port = [net_service port];
        NSString* url_string = [NSString
                                stringWithFormat: @"http://%@:%ld/%@",
                                host,
                                port,
                                path_suffix
                                ];
        NSURL* the_url = [NSURL URLWithString:url_string];
        
        NSLog(@"server url: %@", the_url);
        NSURLRequest* url_request = [NSURLRequest requestWithURL:the_url];
        self.backgroundDownloadTask = [[self backgroundSession] downloadTaskWithRequest:url_request];
        [[self backgroundDownloadTask] resume];
    });
    
}


- (NSURLSession *)backgroundSession
{
    static NSURLSession *backgroundSession = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSURLSessionConfiguration *config = [NSURLSessionConfiguration backgroundSessionConfiguration:@"org.webkit.javascriptcore.test262.backgrounddownloadsession"];
        backgroundSession = [NSURLSession sessionWithConfiguration:config delegate:self delegateQueue:nil];
    });
    return backgroundSession;
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didWriteData:(int64_t)bytesWritten totalBytesWritten:(int64_t)totalBytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite
{
    NSLog(@"%@", NSStringFromSelector(_cmd));
    double currentProgress = totalBytesWritten / (double)totalBytesExpectedToWrite;
    dispatch_async(dispatch_get_main_queue(), ^{
//        self.progressIndicator.hidden = NO;
//        self.progressIndicator.progress = currentProgress;
    });
}


- (void)URLSession:(NSURLSession*)url_session downloadTask:(NSURLSessionDownloadTask *)download_task didFinishDownloadingToURL:(NSURL*)download_location
{
    NSLog(@"%@", NSStringFromSelector(_cmd));
   // We've successfully finished the download. Let's save the file
    NSFileManager* file_manager = [NSFileManager defaultManager];
    
    NSArray* url_array = [file_manager URLsForDirectory:NSDownloadsDirectory inDomains:NSUserDomainMask];
    NSURL *download_directory = url_array[0];
    
    NSURL* destination_path = [download_directory URLByAppendingPathComponent:[download_location lastPathComponent]];
    NSError* the_error;
    
    // Make sure we overwrite anything that's already there
    [file_manager removeItemAtURL:destination_path error:NULL];
    BOOL is_success = [file_manager copyItemAtURL:download_location toURL:destination_path error:&the_error];
    
    if(is_success)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            /*
            UIImage *image = [UIImage imageWithContentsOfFile:[destinationPath path]];
            self.imageView.image = image;
            self.imageView.contentMode = UIViewContentModeScaleAspectFill;
            self.imageView.hidden = NO;
             */
        });
    }
    else
    {
        NSLog(@"Couldn't copy the downloaded file");
    }
    
    if([self backgroundSession] == url_session)
    {
        [self setBackgroundDownloadTask:nil];
        
        // Get hold of the app delegate
        /*
        SCAppDelegate *appDelegate = (SCAppDelegate *)[[UIApplication sharedApplication] delegate];
        if(appDelegate.backgroundURLSessionCompletionHandler) {
            // Need to copy the completion handler
            void (^handler)() = appDelegate.backgroundURLSessionCompletionHandler;
            appDelegate.backgroundURLSessionCompletionHandler = nil;
            handler();
        }
         */
    }
    
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error
{
    NSLog(@"%@", NSStringFromSelector(_cmd));
    NSLog(@"error %@", [error localizedDescription]);
    dispatch_async(dispatch_get_main_queue(), ^{
//        self.progressIndicator.hidden = YES;
    //[self setDownloadButtonsAsEnabled:YES];
    });
}



@end
