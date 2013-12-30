//
//  AppDelegate.m
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "AppDelegate.h"
#import "Test262Helper.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
		^{
			// We want to disable App Nap while running this test since it might take awhile to finish.
			id test262_process_activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityUserInitiated reason:@"Running long conformance test: test262"];
			// work
			Test262Helper_RunTests();
			[[NSProcessInfo processInfo] endActivity:test262_process_activity];
		}
	);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)the_sender
{
	return YES;
}

@end
