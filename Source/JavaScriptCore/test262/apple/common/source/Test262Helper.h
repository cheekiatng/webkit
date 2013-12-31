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

/**
 * Runs all tests for Test262.
 * This runs the entire set of tests for Test262.
 * This is intended to be portable between Mac and iOS so there is no GUI inside.
 * To allow GUI presentation, blocks are provided so you can use closures to connect 
 * in the GUI without invading the implementation. But to keep it general, these callback on the same thread,
 * so remember to redirect back to the main thread if you do GUI stuff.
 * Note that all the callbacks return an NSInteger. This was intended to allow for a way for the
 * native platform to signal back into common code. The main use case is to denote that
 * the application wants to abort running the tests.
 * This was written as a function instead of a method to help limit mutation/side-effects 
 * and make future maintanence easier.
 * @param ns_progress The NSProgress will be updated as the tests progress.
 * @param log_wrapper This is the log object that is responsible for writing output to logs.
 * @param callbackForAllTestsStarting This is called back at the very start of the tests.
 * @param callbackForAllTestsStarting This is called back at the start of each new test.
 * @param callbackForAllTestsStarting This is called back at the end of running each test.
 * @param callbackForAllTestsStarting This is called back after all tests have finished.
 */
void Test262Helper_RunTests(NSProgress* ns_progress, LogWrapper* log_wrapper,
	NSInteger (^callback_for_all_tests_starting)(NSUInteger total_number_of_tests),
	NSInteger (^callback_for_beginning_test)(NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number),
	NSInteger (^callback_for_ending_test)(NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number, NSUInteger total_number_of_tests_failed, _Bool did_pass, NSString* nscf_exception_string, NSString* nscf_stack_string),
	NSInteger (^callback_for_all_tests_finished)(NSUInteger total_number_of_tests, NSUInteger number_of_tests_run, NSUInteger total_number_of_tests_failed)
);

@interface Test262Helper : NSObject

@end
