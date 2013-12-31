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


@interface AppDelegate ()
@property (weak) IBOutlet NSProgressIndicator* progressIndicator;
@property(strong, nonatomic) NSProgress* test262Progress;
@property(strong, nonatomic) LogWrapper* logWrapper;
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

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{

//	LogWrapper_LogEvent([self logWrapper], LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "applicationDidFinishLaunching:", @"Entered applicationDidFinishLaunching");


	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
		^{
			LogWrapper* log_wrapper = [self logWrapper];
			[log_wrapper openNewFile];

			// We want to disable App Nap while running this test since it might take awhile to finish.
			id test262_process_activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityUserInitiated reason:@"Running long conformance test: test262"];

			NSProgress* test262_progress = [[NSProgress alloc] initWithParent:nil userInfo:nil];
			// I'm using Cocoa Bindings to tie the NSProgress fractionCompleted to the progressIndicator.
			// I'm a little surprised this isn't crashing due to being in a background thread.
			// If this crashes, I'll add the layer of indirection later.
			[self setTest262Progress:test262_progress];

			[[self progressIndicator] startAnimation:nil];
			Test262Helper_RunTests(test262_progress, log_wrapper);
			[[self progressIndicator] stopAnimation:nil];

			[[NSProcessInfo processInfo] endActivity:test262_process_activity];

			[log_wrapper flush];
			[log_wrapper closeFile];
		}
	);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)the_sender
{
	return YES;
}

@end
