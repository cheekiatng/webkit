//
//  ViewController.m
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "ViewController.h"
#import "Test262Helper.h"

#import "SocketServer.h"
#import "NetworkHelperForAppDelegate.h"
#import "AppDelegate.h"


@interface ViewController ()
{
	// These blocks don't change, so let's make them instance variables
	NSInteger (^callbackForAllTestsStarting)(NSUInteger);
	NSInteger (^callbackForBeginningTest)(NSString*, NSUInteger, NSUInteger);
	NSInteger (^callbackForEndingTest)(NSString*, NSUInteger, NSUInteger, NSUInteger, _Bool, NSString*, NSString*);
	NSInteger (^callbackForAllTestsFinished)(NSUInteger, NSUInteger, NSUInteger);
}
@property (weak, nonatomic) IBOutlet UIProgressView* progressIndicator;
@property(strong, nonatomic) NSProgress* test262Progress;
@property(assign, nonatomic) _Bool javaScriptThreadRunning;
@property(assign, nonatomic) _Bool javaScriptThreadShouldContinueRunning;
@property (weak, nonatomic) IBOutlet UILabel* runningTestNumberStatusLabel;
@property (weak, nonatomic) IBOutlet UILabel* currentTestNameStatusLabel;
@property (weak, nonatomic) IBOutlet UILabel* totalFailedStatusLabel;
@property (weak, nonatomic) IBOutlet UIButton* abortTestsButton;
@property (weak, nonatomic) IBOutlet UIButton* runTestsButton;
@property (weak, nonatomic) IBOutlet UILabel* logFileLocationLabel;
@property (assign, nonatomic) UIBackgroundTaskIdentifier backgroundTaskIdentifier;
@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	[self setBackgroundTaskIdentifier:UIBackgroundTaskInvalid];
	[[self runningTestNumberStatusLabel] setText:nil];
	[[self currentTestNameStatusLabel] setText:nil];
	[[self totalFailedStatusLabel] setText:nil];
	[[self logFileLocationLabel] setText:nil];
	[[self progressIndicator] setProgress:0 animated:NO];
	[[self abortTestsButton] setEnabled:NO];

	[self initBlockIvarsForTestGui];
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
				[[weak_self runTestsButton] setEnabled:NO];
				[[weak_self abortTestsButton] setEnabled:YES];
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
				[[weak_self runningTestNumberStatusLabel] setText:[NSString stringWithFormat:@"Running test %lu of %lu", (unsigned long)current_test_number, (unsigned long)total_number_of_tests]];
				[[weak_self currentTestNameStatusLabel] setText:test_file];
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
				[[weak_self totalFailedStatusLabel] setText:[NSString stringWithFormat:@"Total failed: %lu", (unsigned long)total_number_of_tests_failed]];
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
				[[weak_self runTestsButton] setEnabled:YES];
				[[weak_self abortTestsButton] setEnabled:NO];
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

- (IBAction) touchedRunTests:(id)the_sender
{
	[self runTests];

}


- (IBAction) touchedAbortTests:(id)the_sender
{
    UIAlertView* alert_view = [[UIAlertView alloc]
		initWithTitle:@"Tests are currently running."
		message:@"Are you sure you want to abort tests?"
		delegate:self
		cancelButtonTitle:@"Abort Tests"
		otherButtonTitles:@"Continue Running Tests", nil
	];
	[alert_view setTag:1];
    [alert_view show];
}

- (void) alertView:(UIAlertView*)alert_view didDismissWithButtonIndex:(NSInteger)button_index
{
    if([alert_view tag] == 1)
	{
        if(0 == button_index)
		{
			[self abortTests];
			return;
        }
    }
}

- (void) abortTests
{
	[self setJavaScriptThreadShouldContinueRunning:false];
}

- (void) runTests
{
	[self setBackgroundTaskIdentifier:[[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:nil]];

	NSProgress* test262_progress = [[NSProgress alloc] initWithParent:nil userInfo:nil];



	// I'm being a little conservative about setting up the KVO on the main thread.
	// I don't think this really matters since the observer will fire on the thread that the change happens in.
	[self setTest262Progress:test262_progress];
	[_test262Progress addObserver:self
				forKeyPath:@"fractionCompleted"
				   options:NSKeyValueObservingOptionNew
				   context:NULL];

	[self setJavaScriptThreadRunning:true];
	[self setJavaScriptThreadShouldContinueRunning:true];

	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
		^{
			LogWrapper* log_wrapper = [(AppDelegate*)[[UIApplication sharedApplication] delegate] logWrapper];
			[log_wrapper openNewFile];

			NSString* log_file_path_and_name = [log_wrapper logFilePathAndName];


			Test262Helper_RunTests(test262_progress, log_wrapper,
				callbackForAllTestsStarting,
				callbackForBeginningTest,
				callbackForEndingTest,
				callbackForAllTestsFinished
			);

			[log_wrapper flush];
			[log_wrapper closeFile];

			dispatch_async(dispatch_get_main_queue(),
				^{

					[_test262Progress removeObserver:self forKeyPath:@"fractionCompleted"];
					[[self logFileLocationLabel] setText:log_file_path_and_name];
				}
			);



			[self setJavaScriptThreadRunning:false];
			[[UIApplication sharedApplication] endBackgroundTask:[self backgroundTaskIdentifier]];
		}
	);

}

- (void)observeValueForKeyPath:(NSString*)key_path
	ofObject:(id)the_object
	change:(NSDictionary*)the_change
	context:(void*)the_context
{
	if(the_object == _test262Progress)
	{
		// The test is running in a background thread, so this change fired on that thread.
		// Redirect to the main thread to change the UI.
		dispatch_async(dispatch_get_main_queue(),
			^{
				[[self progressIndicator] setProgress:[[self test262Progress] fractionCompleted] animated:YES];
			}
		);
		// Handle new fractionCompleted value
		return;
	}

	[super observeValueForKeyPath:key_path ofObject:the_object change:the_change context:the_context];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
