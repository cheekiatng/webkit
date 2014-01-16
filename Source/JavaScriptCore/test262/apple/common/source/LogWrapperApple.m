//
//  LogWrapperApple.m
//  Test262
//
//  Created by Eric Wing on 1/16/14.
//  Copyright (c) 2014 JavaScriptCore. All rights reserved.
//

#import "LogWrapperApple.h"

_Bool LogWrapperApple_OpenNewFile(LogWrapper* log_wrapper)
{
	NSArray* directory_paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* documents_directory = [directory_paths firstObject];
	NSString* test_file = [documents_directory stringByAppendingPathComponent:@"test262_runlog.txt"];
	return LogWrapper_OpenNewFileWithName(log_wrapper, [test_file fileSystemRepresentation]);
}