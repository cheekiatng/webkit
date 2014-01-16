//
//  AppDelegate.h
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "LogWrapper.h"
#import "NetworkHelperForAppDelegate.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate, NetworkHelperForAppDelegate>

@property (strong, nonatomic) UIWindow *window;
@property(assign, nonatomic, readonly) LogWrapper* logWrapper;

// for NetworkHelperForAppDelegate
@property(strong, nonatomic, readonly) NSString* logFileLocationString;
// for NetworkHelperForAppDelegate
- (NSString*) serviceName;

@end
