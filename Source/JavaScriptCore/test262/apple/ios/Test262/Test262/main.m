//
//  main.m
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"

// If you are automatically breaking on SIGPIPE due to writing on broken sockets, see this to make Xcode not do that:
// http://stackoverflow.com/questions/10431579/permanently-configuring-lldb-in-xcode-4-3-2-not-to-stop-on-signals
int main(int argc, char * argv[])
{
	@autoreleasepool {
	    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
	}
}
