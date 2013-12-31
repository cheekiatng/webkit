//
//  Test262Helper.h
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LogWrapper.h"

NSString* Test262Helper_LoadTestHarnessScripts(void);
void Test262Helper_RunTests(NSProgress* ns_progress, LogWrapper* log_wrapper);

@interface Test262Helper : NSObject

@end
