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

	// Disable word-wrapping for performance reasons (when the log gets large)
	[[[self logStreamTextView] enclosingScrollView] setHasHorizontalScroller:TRUE]; /* (1) */
	[[self logStreamTextView] setHorizontallyResizable:TRUE]; /* (2) */
	NSSize layout_size = [[self logStreamTextView] maxSize]; /* (3) */
	layout_size.width = layout_size.height;
	[[self logStreamTextView] setMaxSize:layout_size];
	[[[self logStreamTextView] textContainer] setWidthTracksTextView:FALSE]; /* (4) */
	[[[self logStreamTextView] textContainer] setContainerSize:layout_size]; /* (5) */
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
	if((log_message) != nil && ([log_message length] > 0))
	{
		// Smart Scrolling http://stackoverflow.com/questions/15546808/scrolling-nstextview-to-bottom
		CGFloat visible_max_y = NSMaxY([[self logStreamTextView] visibleRect]);
		CGFloat bounds_max_y = NSMaxY([[self logStreamTextView] bounds]);

		BOOL should_scroll = NO;
		CGFloat TOLERENCE_MARGIN = bounds_max_y * .03;
		if(bounds_max_y < 10000)
		{
			// hack for small numbers to get scrolling started
			should_scroll = YES;
		}
		if(visible_max_y >= (bounds_max_y - TOLERENCE_MARGIN))
		{
			should_scroll = YES;
		}



		@try
		{
			[[[[self logStreamTextView] textStorage] mutableString] appendString:log_message];
		}
		@catch(NSException* exception)
		{
			NSLog(@"Exception thrown in Cocoa from [[[[self logStreamTextView] textStorage] mutableString] appendString:log_message]\n%@", [exception callStackSymbols]);
			NSLog(@"textview length is %lu", (unsigned long)[[[[self logStreamTextView] textStorage] mutableString] length]);
			NSLog(@"log_message length is: %lu", (unsigned long)[log_message length]);
			NSLog(@"log_message is: %@", log_message);


			return;
		}

		if(should_scroll) // Scroll to end of the textview contents
		{
			// The bounds actually changed and should be lower due to the appended string, but using scrollRangeToVisible is too slow for large data sets and trying to get bounds again doesn't work as it still provides the old value.
//			bounds_max_y = NSMaxY([[self logStreamTextView] bounds]);

			// for large numbers, use the faster, but less accurate scrollPoint
			if(bounds_max_y > 100000)
			{
				// it seems to be okay to go over the max amount
				[[self logStreamTextView] scrollPoint:NSMakePoint(0.0, bounds_max_y + 2.0*TOLERENCE_MARGIN)];
//			[[self logStreamTextView] scrollToPoint:NSMakeRange([[[self logStreamTextView] string] length], 0)];
			}
			else
			{
				// more accurate, but will bring the app to a halt around 50,000 lines
				[[self logStreamTextView] scrollRangeToVisible:NSMakeRange([[[self logStreamTextView] string] length], 0)];
			}
		}

	}
	else
	{
		if(nil == log_message)
		{
			NSLog(@"postLogEvent got nil");
		}
		else
		{
			NSLog(@"postLogEvent got 0 length message");
		}
	}
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
