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
	// Insert code here to initialize your application
	Test262Helper_RunTests();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)the_sender
{
	return YES;
}

@end
