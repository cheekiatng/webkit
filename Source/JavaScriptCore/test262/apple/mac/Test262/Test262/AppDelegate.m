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

@interface AppDelegate ()
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

	callbackForAllTestsStarting = ^(NSUInteger total_number_of_tests)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{

			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return (NSInteger)1;
		}
		else
		{
			return (NSInteger)0;
		}
	};

	callbackForBeginningTest = ^(NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{
				[weak_self setRunningTestNumberStatusString:[NSString stringWithFormat:@"Running test %lu of %lu", (unsigned long)current_test_number, (unsigned long)total_number_of_tests]];
				[weak_self setCurrentTestNameStatusString:test_file];
			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return (NSInteger)1;
		}
		else
		{
			return (NSInteger)0;
		}
	};

	callbackForEndingTest = ^(NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number, NSUInteger total_number_of_tests_failed, _Bool did_pass, NSString* nscf_exception_string, NSString* nscf_stack_string)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{
				[weak_self setTotalFailedStatusString:[NSString stringWithFormat:@"Total failed: %lu", (unsigned long)total_number_of_tests_failed]];
			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return (NSInteger)1;
		}
		else
		{
			return (NSInteger)0;
		}
	};

	callbackForAllTestsFinished = ^(NSUInteger total_number_of_tests, NSUInteger number_of_tests_run, NSUInteger total_number_of_tests_failed)
	{
		dispatch_async(dispatch_get_main_queue(),
			^{

			}
		);
		if(true == [weak_self javaScriptThreadShouldContinueRunning])
		{
			return (NSInteger)1;
		}
		else
		{
			return (NSInteger)0;
		}
	};
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self initBlockIvarsForTestGui];

//	LogWrapper_LogEvent([self logWrapper], LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "applicationDidFinishLaunching:", @"Entered applicationDidFinishLaunching");
//	[self runTests];
}

- (IBAction) clickedRunTests:(id)the_sender
{
	[self runTests];
}

- (IBAction)clickedAbortTests:(id)the_sender
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
			[self setJavaScriptThreadRunning:false];

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


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)the_sender
{
	return YES;
}

@end
