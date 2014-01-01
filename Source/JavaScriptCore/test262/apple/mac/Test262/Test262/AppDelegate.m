//
//  AppDelegate.m
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "AppDelegate.h"
#import "Test262Helper.h"
#import "LogWrapper.h"
#import "NSAttributedString+Hyperlink.h"

#define TEST262_QUIT_BY_APPLICATION_QUIT (void*)0x01
#define TEST262_QUIT_BY_WINDOW_CLOSE (void*)0x02


@interface AppDelegate () <NSWindowDelegate>
{
	// These blocks don't change, so let's make them instance variables
	NSInteger (^callbackForAllTestsStarting)(NSUInteger);
	NSInteger (^callbackForBeginningTest)(NSString*, NSUInteger, NSUInteger);
	NSInteger (^callbackForEndingTest)(NSString*, NSUInteger, NSUInteger, NSUInteger, _Bool, NSString*, NSString*);
	NSInteger (^callbackForAllTestsFinished)(NSUInteger, NSUInteger, NSUInteger);
}
@property (weak) IBOutlet NSProgressIndicator* progressIndicator;
@property (weak) IBOutlet NSTextField* labelForLogFileLocation;

@property(strong, nonatomic) NSProgress* test262Progress;
@property(strong, nonatomic) NSString* runningTestNumberStatusString;
@property(strong, nonatomic) NSString* currentTestNameStatusString;
@property(strong, nonatomic) NSString* totalFailedStatusString;


@property(strong, nonatomic) LogWrapper* logWrapper;
@property(assign, nonatomic) _Bool javaScriptThreadRunning;
@property(assign, nonatomic) _Bool javaScriptThreadShouldContinueRunning;
@end

@implementation AppDelegate

- (instancetype) init
{
	self = [super init];
	if(nil != self)
	{
		_logWrapper = [[LogWrapper alloc] init];
	}
	return self;
}

// Do this after the nib is loaded since there is UI involved.
// These blocks don't really use closures directly.
// They simply provide means to hook GUI into the GUI agnostic test program.
// This is the delegate pattern using the language instead of the pattern.
// These blocks could have been declared locally and inline, and maybe that's better,
// but I thought I would experiment with declaring them outside to see if it
// makes the actual logic for the runTests method more clear since it avoids interweaving.
- (void) initBlockIvarsForTestGui
{
	// capturing self strongly in this block is likely to lead to a retain cycle
	// I guess I wouldn't have this problem if I didn't use ivars and declared this inline.
	// On the otherhand, these methods don't use closures that need to capture updated values
	// so maybe there is a performance benefit of not needing to create/tear these down?
	// (Though maybe not since this will involve a block copy to the heap?)
	__weak typeof(self) weak_self = self;

	callbackForAllTestsStarting = ^ NSInteger (NSUInteger total_number_of_tests)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{

			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return 1;
		}
		else
		{
			return 0;
		}
	};

	callbackForBeginningTest = ^ NSInteger (NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{
				[weak_self setRunningTestNumberStatusString:[NSString stringWithFormat:@"Running test %lu of %lu", (unsigned long)current_test_number, (unsigned long)total_number_of_tests]];
				[weak_self setCurrentTestNameStatusString:test_file];
			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return 1;
		}
		else
		{
			return 0;
		}
	};

	callbackForEndingTest = ^ NSInteger (NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number, NSUInteger total_number_of_tests_failed, _Bool did_pass, NSString* nscf_exception_string, NSString* nscf_stack_string)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{
				[weak_self setTotalFailedStatusString:[NSString stringWithFormat:@"Total failed: %lu", (unsigned long)total_number_of_tests_failed]];
			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return 1;
		}
		else
		{
			return 0;
		}
	};

	callbackForAllTestsFinished = ^ NSInteger (NSUInteger total_number_of_tests, NSUInteger number_of_tests_run, NSUInteger total_number_of_tests_failed)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{

			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return 1;
		}
		else
		{
			return 0;
		}
	};
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self initBlockIvarsForTestGui];
/*
	NSNotificationCenter* notification_center = [NSNotificationCenter defaultCenter];
//	[notification_center addObserver:self selector:@selector(appWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];
	[notification_center addObserver:self selector:@selector(windowClosing:) name:NSWindowWillCloseNotification object:nil];
*/
//	LogWrapper_LogEvent([self logWrapper], LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "applicationDidFinishLaunching:", @"Entered applicationDidFinishLaunching");
//	[self runTests];
}

- (IBAction) clickedRunTests:(id)the_sender
{
	[self runTests];
}

- (IBAction)clickedAbortTests:(id)the_sender
{
		NSAlert* alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"Continue Running Tests"];
		[alert addButtonWithTitle:@"Abort Tests"];
		NSString* message_text = [NSString stringWithFormat:@"Tests are currently running."];
		[alert setMessageText:message_text];
		NSString* info_text = [NSString stringWithFormat:@"Are you sure you want to abort tests?"];

		[alert setInformativeText:info_text];
		[alert setAlertStyle:NSInformationalAlertStyle];

		[alert beginSheetModalForWindow:[self window]
			modalDelegate:self
			didEndSelector:@selector(abortTestsAlertDidEnd:returnCode:contextInfo:)
			contextInfo:nil
		];



}

- (void) abortTestsAlertDidEnd:(NSAlert*)alert returnCode:(int)return_code contextInfo:(void*)context_info
{
	// user doesn't want to quit
	if(NSAlertFirstButtonReturn == return_code)
	{
		return;
	}
	// user wants to quit
	if(NSAlertSecondButtonReturn == return_code)
	{
		[self abortTests];
		return;
	}

	//    [alert release];
	
}


- (void) abortTests
{
	[self setJavaScriptThreadShouldContinueRunning:false];
}

- (void) runTests
{
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
		^{
			LogWrapper* log_wrapper = [self logWrapper];
			[log_wrapper openNewFile];
			// Hold on to this reference because logger closeFile will nil its reference.
			NSString* log_file_path_and_name = [log_wrapper logFilePathAndName];



			// We want to disable App Nap while running this test since it might take awhile to finish.
			id test262_process_activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityUserInitiated reason:@"Running long conformance test: test262"];

			NSProgress* test262_progress = [[NSProgress alloc] initWithParent:nil userInfo:nil];
			// I'm using Cocoa Bindings to tie the NSProgress fractionCompleted to the progressIndicator.
			// I'm a little surprised this isn't crashing due to being in a background thread.
			// If this crashes, I'll add the layer of indirection later.
			[self setTest262Progress:test262_progress];


			[self setJavaScriptThreadRunning:true];
			[self setJavaScriptThreadShouldContinueRunning:true];

			dispatch_async(dispatch_get_main_queue(),
				^{
//					[self setHyperlinkWithTextField:[self labelForLogFileLocation] withPath:log_file_path_and_name];
					[[self labelForLogFileLocation] setStringValue:@""];
					// oops. I forgot this does nothing for determinate progress indicators
//					[[self progressIndicator] startAnimation:nil];
				}
			);

			Test262Helper_RunTests(test262_progress, log_wrapper,
				callbackForAllTestsStarting,
				callbackForBeginningTest,
				callbackForEndingTest,
				callbackForAllTestsFinished
			);

			

			[[NSProcessInfo processInfo] endActivity:test262_process_activity];

			[log_wrapper flush];
			[log_wrapper closeFile];

			dispatch_async(dispatch_get_main_queue(),
				^{
					// oops. I forgot this does nothing for determinate progress indicators
//					[[self progressIndicator] stopAnimation:nil];
					// Make sure to use the log_file_path_and_name instead of log_wrapper's because it nil'ed its reference when the file closed
					[self setHyperlinkWithTextField:[self labelForLogFileLocation] withPath:log_file_path_and_name];
				}
			);

			[self setJavaScriptThreadRunning:false];

		}
	);
}

// https://developer.apple.com/library/mac/qa/qa1487/_index.html
- (void) setHyperlinkWithTextField:(NSTextField*)text_field withPath:(NSString*)full_file_path
{
    // both are needed, otherwise hyperlink won't accept mousedown
    [text_field setAllowsEditingTextAttributes:YES];
    [text_field setSelectable:YES];

//	NSString* original_string = [text_field stringValue];

    NSURL* url = [NSURL fileURLWithPath:full_file_path];

    NSMutableAttributedString* string = [[NSMutableAttributedString alloc] init];
    [string appendAttributedString:[NSAttributedString hyperlinkFromString:full_file_path withURL:url]];

    // set the attributed string to the NSTextField
    [text_field setAttributedStringValue:string];

//    [string release];
}


- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)the_sender
{
	return YES;
}

/*
- (void) windowClosing:(NSNotification*)the_notification
{
	if([the_notification object] == [self window])
	{

	}
}
*/


- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication*)the_sender
{
	// This can get complicated. Should ask the user if they really want to quit if tests are running.
	// NSTerminateLater requires pumping the event loop manually to handle dialogs.
	// For best practices, the app should allow quiting/resuming,
	// but this requires saving/restoring the current test run state, the current file handle,
	// the current file upload to server status,
	// and any socket connections. Yuck.
	if([self javaScriptThreadRunning])
	{
		NSAlert* alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"Continue Running Tests"];
		[alert addButtonWithTitle:@"Abort Tests & Quit"];
		NSString* message_text = [NSString stringWithFormat:@"Tests are currently running."];
		[alert setMessageText:message_text];
		NSString* info_text = [NSString stringWithFormat:@"Quitting now will abort running tests. Are you sure you want to abort tests & quit?"];

		[alert setInformativeText:info_text];
		[alert setAlertStyle:NSInformationalAlertStyle];

		[alert beginSheetModalForWindow:[self window]
			modalDelegate:self
			didEndSelector:@selector(abortTestsForWindowCloseAlertDidEnd:returnCode:contextInfo:)
			contextInfo:TEST262_QUIT_BY_APPLICATION_QUIT
		];


		return NSTerminateLater;
	}
	else
	{
		return NSTerminateNow;
	}
}

// NSWindowDelegate
- (BOOL) windowShouldClose:(id)the_sender
{
	// This can get complicated. Should ask the user if they really want to quit if tests are running.
	// NSTerminateLater requires pumping the event loop manually to handle dialogs.
	// For best practices, the app should allow quiting/resuming,
	// but this requires saving/restoring the current test run state, the current file handle,
	// the current file upload to server status,
	// and any socket connections. Yuck.
	if([self javaScriptThreadRunning])
	{

		NSAlert* alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"Continue Running Tests"];
		[alert addButtonWithTitle:@"Abort Tests & Quit"];
		NSString* message_text = [NSString stringWithFormat:@"Tests are currently running."];
		[alert setMessageText:message_text];
		NSString* info_text = [NSString stringWithFormat:@"Quitting now will abort running tests. Are you sure you want to abort tests & quit?"];

		[alert setInformativeText:info_text];
		[alert setAlertStyle:NSInformationalAlertStyle];

		[alert beginSheetModalForWindow:[self window]
			modalDelegate:self
			didEndSelector:@selector(abortTestsForWindowCloseAlertDidEnd:returnCode:contextInfo:)
			contextInfo:TEST262_QUIT_BY_WINDOW_CLOSE
		];



		return NO;
	}
	else
	{
		return YES;
	}
}

- (void) abortTestsForWindowCloseAlertDidEnd:(NSAlert*)alert returnCode:(int)return_code contextInfo:(void*)context_info
{
	// user doesn't want to quit
	if(NSAlertFirstButtonReturn == return_code)
	{
		[NSApp replyToApplicationShouldTerminate:NO];
		return;
	}
	// user wants to quit
	if(NSAlertSecondButtonReturn == return_code)
	{
		[self abortTests];
		// So here's a problem. We really should wait for the thread to finish.
		// That means we can't quit here and need to monitor javaScriptThreadRunning.
		// I can use KVO to watch the variable. I really should create some different objects though to prevent possible collisions with other parts of this code that might want to watch it.
		// I can use the context_info to help differentiate cases.
		[self addObserver:self
			forKeyPath:@"javaScriptThreadRunning"
			options:NSKeyValueObservingOptionNew
			context:context_info
		];

		return;
	}

	//    [alert release];

}


- (void)observeValueForKeyPath:(NSString*)key_path
	ofObject:(id)the_object
	change:(NSDictionary*)the_change
	context:(void*)the_context
{
	// Assumuption: This observer is only setup when the user has told the application to quit.
	if([key_path isEqualToString:@"javaScriptThreadRunning"])
	{
		if(TEST262_QUIT_BY_APPLICATION_QUIT == the_context)
		{
			[NSApp replyToApplicationShouldTerminate:YES];
		}
		else if(TEST262_QUIT_BY_WINDOW_CLOSE == the_context)
		{
			[NSApp terminate:nil];
		}
		return;
	}

	[super observeValueForKeyPath:key_path ofObject:the_object change:the_change context:the_context];
}

@end
