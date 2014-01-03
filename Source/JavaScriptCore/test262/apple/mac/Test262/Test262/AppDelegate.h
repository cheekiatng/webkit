//
//  AppDelegate.h
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "NetworkHelperForAppDelegate.h"

@interface AppDelegate : NSObject <NSApplicationDelegate, NetworkHelperForAppDelegate>

@property (assign) IBOutlet NSWindow *window;

// for NetworkHelperForAppDelegate
@property(strong, nonatomic, readonly) NSString* logFileLocationString;
// for NetworkHelperForAppDelegate
- (NSString*) serviceName;

@end
