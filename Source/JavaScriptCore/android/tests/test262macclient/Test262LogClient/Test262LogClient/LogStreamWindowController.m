//
//  LogStreamWindowController.m
//  Test262LogClient
//
//  Created by Eric Wing on 12/28/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "LogStreamWindowController.h"
#import "AppDelegate.h"
#import "ResolveForDownloadDelegate.h"

@interface LogStreamWindowController ()
{
	void (^windowCloseCompletionBlock)(void);
}
@property (unsafe_unretained) IBOutlet NSTextView* logStreamTextView;
@end

@implementation LogStreamWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (void) windowWillClose:(NSNotification*)the_notification
{
	NSLog(@"windowWillClose:");
	if(nil != windowCloseCompletionBlock)
	{
		windowCloseCompletionBlock();
	}
	
}

- (void) setWindowDidCloseCompletionBlock:(void (^)(void))the_block
{
//	[windowCloseCompletionBlock release];
	windowCloseCompletionBlock = [the_block copy];
}

/*
- (void) postLogEvent:(void*)recv_buffer length:(NSUInteger)num_bytes
{
	// I'm assuming the mutableString append will create new storage, so NoCopy is an optimization
	NSString* log_message = [[NSString alloc] initWithBytesNoCopy:recv_buffer length:num_bytes encoding:NSUTF8StringEncoding freeWhenDone:NO];
	[[[[self logStreamTextView] textStorage] mutableString] appendString:log_message];
}
*/

- (void) postLogEvent:(NSString*)log_message
{
	// Smart Scrolling http://stackoverflow.com/questions/15546808/scrolling-nstextview-to-bottom
    BOOL scroll = (NSMaxY([[self logStreamTextView] visibleRect]) == NSMaxY([[self logStreamTextView] bounds]));

	[[[[self logStreamTextView] textStorage] mutableString] appendString:log_message];

/*
    if (scroll) // Scroll to end of the textview contents
	{
        [[self logStreamTextView] scrollRangeToVisible: NSMakeRange([[[self logStreamTextView] string] length], 0)];
	}
	
	*/
}

- (IBAction) clearDisplayClicked:(id)the_sender
{
	[[[[self logStreamTextView] textStorage] mutableString] setString:@""];
}

- (IBAction) downloadButtonClicked:(id)the_sender
{
	NSNetService* net_service = [self netService];
	// The service may have gone away, and since we have a weak reference, it may be nil
	if(nil == net_service)
	{
		NSLog(@"This netService is no more");


		// Handle error here
		NSAlert* alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"Dismiss"];
		//	[alert addButtonWithTitle:@"Cancel"];
		NSString* message_text = [NSString stringWithFormat:@"Service %@ is no longer available", [[self window] title]];
		[alert setMessageText:message_text];
		NSString* info_text = [NSString stringWithFormat:@"Try again or try restarting the app on the device."];

		[alert setInformativeText:info_text];
		[alert setAlertStyle:NSInformationalAlertStyle];

		[alert beginSheetModalForWindow:[self window]
			modalDelegate:self
			didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
			contextInfo:nil
		];

		return;
		
	}

	AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];

	[net_service setDelegate:[app_delegate resolveForDownloadDelegate]];
	[net_service resolveWithTimeout:5.0];
	NSProgressIndicator* progress_indicator = nil;
	/*
	// Too many edge cases depending on how the download was started.
	// Better use of bindings would help here.
	progress_indicator = [self progressIndicator];
	[progress_indicator setIndeterminate:YES];
	[progress_indicator startAnimation:nil];
*/
	progress_indicator = [app_delegate progressIndicator];
	[progress_indicator setIndeterminate:YES];
	[progress_indicator startAnimation:nil];
}

- (void)alertDidEnd:(NSAlert*)alert returnCode:(int)return_code contextInfo:(void*)context_info
{
	//    [alert release];
}


@end
