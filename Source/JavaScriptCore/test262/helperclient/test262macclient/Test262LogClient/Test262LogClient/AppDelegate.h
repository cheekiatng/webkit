//
//  AppDelegate.h
//  Test262LogClient
//
//  Created by Eric Wing on 12/24/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class LogStreamWindowController;
@class ResolveForDownloadDelegate;
@class ResolveForLogStreamDelegate;

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSProgressIndicator* progressIndicator;


- (LogStreamWindowController*) logStreamWindowControllerExistsForName:(NSString*)window_name;
- (void) addWindowControllerToActiveList:(LogStreamWindowController*)window_controller;
- (void) removeWindowControllerFromActiveList:(LogStreamWindowController*)window_controller;

- (void) resolveForDownloadForNetService:(NSNetService*)service;
@end
