//
//  LogStreamWindowController.m
//  Test262LogClient
//
//  Created by Eric Wing on 12/28/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "LogStreamWindowController.h"

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


    if (scroll) // Scroll to end of the textview contents
	{
        [[self logStreamTextView] scrollRangeToVisible: NSMakeRange([[[self logStreamTextView] string] length], 0)];
	}
}

@end
