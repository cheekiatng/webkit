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

// I need to root these delegate instances somewhere or ARC will release my temporary instances
@property (nonatomic, strong, readonly ) ResolveForDownloadDelegate*              resolveForDownloadDelegate;
@property (nonatomic, strong, readonly ) ResolveForLogStreamDelegate*              resolveForLogStreamDelegate;

- (LogStreamWindowController*) logStreamWindowControllerExistsForName:(NSString*)window_name;
- (void) addWindowControllerToActiveList:(LogStreamWindowController*)window_controller;
- (void) removeWindowControllerFromActiveList:(LogStreamWindowController*)window_controller;

@end
