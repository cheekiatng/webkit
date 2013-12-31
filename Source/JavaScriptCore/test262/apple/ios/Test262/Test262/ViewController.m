//
//  ViewController.m
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "ViewController.h"
#import "Test262Helper.h"

@interface ViewController ()
@property (weak, nonatomic) IBOutlet UIProgressView* progressIndicator;
@property(strong, nonatomic) NSProgress* test262Progress;
@property(strong, nonatomic) LogWrapper* logWrapper;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

	_logWrapper = [[LogWrapper alloc] init];

	NSProgress* test262_progress = [[NSProgress alloc] initWithParent:nil userInfo:nil];



	// I'm being a little conservative about setting up the KVO on the main thread.
	// I don't think this really matters since the observer will fire on the thread that the change happens in.
	[self setTest262Progress:test262_progress];
	[_test262Progress addObserver:self
				forKeyPath:@"fractionCompleted"
				   options:NSKeyValueObservingOptionNew
				   context:NULL];

	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
		^{
			LogWrapper* log_wrapper = [self logWrapper];
			[log_wrapper openNewFile];


			Test262Helper_RunTests(test262_progress, log_wrapper);

			dispatch_async(dispatch_get_main_queue(),
				^{

					[_test262Progress removeObserver:self forKeyPath:@"fractionCompleted"];
				}
			);

			[log_wrapper flush];
			[log_wrapper closeFile];
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
