//
//  Test262Helper.m
//  Test262
//
//  Created by Eric Wing on 12/29/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "Test262Helper.h"
#import <JavaScriptCore/JavaScriptCore.h>


extern _Bool Test262_EvaluateStringScript(JSStringRef js_script_string, JSStringRef js_file_name, JSStringRef* out_error_string, JSStringRef* out_stack_string);


NSString* Test262Helper_LoadTestHarnessScripts()
{
	NSMutableString* concat_string = [[NSMutableString alloc] init];
	NSString* harness_dir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"harness"];

	NSString* temp_string = nil;
	NSError* the_error = nil;
	NSString* file_path = nil;

	file_path = [harness_dir stringByAppendingPathComponent:@"cth.js"];
	temp_string = [NSString stringWithContentsOfFile:file_path encoding:NSUTF8StringEncoding error:&the_error];
	if((nil == temp_string) || (the_error != nil))
	{
		NSLog(@"Error loading %@", file_path);
		if(the_error != nil)
		{
			NSLog(@"NSError is: %@", [the_error localizedDescription]);
			the_error = nil;
		}
	}
	else
	{
		[concat_string appendString:temp_string];
		[concat_string appendString:@"\n"];
	}

	file_path = [harness_dir stringByAppendingPathComponent:@"sta.js"];
	temp_string = [NSString stringWithContentsOfFile:file_path encoding:NSUTF8StringEncoding error:&the_error];
	if((nil == temp_string) || (the_error != nil))
	{
		NSLog(@"Error loading %@", file_path);
		if(the_error != nil)
		{
			NSLog(@"NSError is: %@", [the_error localizedDescription]);
			the_error = nil;
		}
	}
	else
	{
		[concat_string appendString:temp_string];
		[concat_string appendString:@"\n"];
	}

	file_path = [harness_dir stringByAppendingPathComponent:@"ed.js"];
	temp_string = [NSString stringWithContentsOfFile:file_path encoding:NSUTF8StringEncoding error:&the_error];
	if((nil == temp_string) || (the_error != nil))
	{
		NSLog(@"Error loading %@", file_path);
		if(the_error != nil)
		{
			NSLog(@"NSError is: %@", [the_error localizedDescription]);
			the_error = nil;
		}
	}
	else
	{
		[concat_string appendString:temp_string];
		[concat_string appendString:@"\n"];
	}

	file_path = [harness_dir stringByAppendingPathComponent:@"testBuiltInObject.js"];
	temp_string = [NSString stringWithContentsOfFile:file_path encoding:NSUTF8StringEncoding error:&the_error];
	if((nil == temp_string) || (the_error != nil))
	{
		NSLog(@"Error loading %@", file_path);
		if(the_error != nil)
		{
			NSLog(@"NSError is: %@", [the_error localizedDescription]);
			the_error = nil;
		}
	}
	else
	{
		[concat_string appendString:temp_string];
		[concat_string appendString:@"\n"];
	}

	file_path = [harness_dir stringByAppendingPathComponent:@"testIntl.js"];
	temp_string = [NSString stringWithContentsOfFile:file_path encoding:NSUTF8StringEncoding error:&the_error];
	if((nil == temp_string) || (the_error != nil))
	{
		NSLog(@"Error loading %@", file_path);
		if(the_error != nil)
		{
			NSLog(@"NSError is: %@", [the_error localizedDescription]);
			the_error = nil;
		}
	}
	else
	{
		[concat_string appendString:temp_string];
		[concat_string appendString:@"\n"];
	}

	return [concat_string copy];
}


NSString* Test262Helper_DetermineStrictModeStringFromScript(NSString* raw_script)
{
	NSRange is_in_range = [raw_script rangeOfString:@"@onlyStrict" options:0];
	if(NSNotFound != is_in_range.location)
	{
		// found it
		return @"\"use strict\";\nvar strict_mode = true;\n";
	}
	else
	{
		return @"var strict_mode = false; \n";
	}
}

_Bool Test262Helper_DetermineIfPositiveTestFromScript(NSString* raw_script)
{
	NSRange is_in_range = [raw_script rangeOfString:@"@negative" options:0];
	if(NSNotFound != is_in_range.location)
	{
		// found it
		return false;
	}
	else
	{
		return true;
	}
}


void Test262Helper_RunTests(NSProgress* ns_progress)
{
	NSString* test_harness_script_string = Test262Helper_LoadTestHarnessScripts();
	NSMutableString* concat_string = [[NSMutableString alloc] init];

//	NSString* suite_dir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"suite"];

	NSString* resource_path = [[NSBundle mainBundle] resourcePath];

	NSString* raw_test_script = nil;
	NSError* the_error = nil;
//	NSString* file_path = nil;

	NSUInteger number_of_failed_tests = 0;
	NSUInteger current_index_count = 0;

	/* I originally was going to dynamically traverse the directories, but on Android,
		there was a serious performance problem with this (took 3 hours just to get a full directory listing in an APK).
		So instead, I used a pre-created text file which contained the list of files.
		While we don't have the performance problems of Android, I discovered I liked having the text file because
		I could modify the text file to run specific tests for debugging.
		So I've decided to use the text file here too.
	 */
	NSString* test_manifiest_file_string = [NSString stringWithContentsOfFile:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"test262_filelist.txt"] encoding:NSUTF8StringEncoding error:&the_error];
	NSArray* file_list = [test_manifiest_file_string componentsSeparatedByString:@"\n"];

	// componentsSeparatedByString is leaving me a blank line at the end
	// I want to get rid of it
	if([[file_list lastObject] length] == 0)
	{
		NSRange sub_range;
		sub_range.location = 0;
		sub_range.length = [file_list count] - 1;
		file_list = [file_list subarrayWithRange:sub_range];
	}

	// We might want to further sanitize the list if we support commented out lines and so forth
	[ns_progress setTotalUnitCount:[file_list count]];

	for(NSString* a_file in file_list)
	{
		// just in case there is a blank line
		if([a_file length] == 0)
		{
			current_index_count = current_index_count + 1;
			[ns_progress setCompletedUnitCount:current_index_count];
			continue;
		}
		NSString* file_path = [resource_path stringByAppendingPathComponent:a_file];
		raw_test_script = [NSString stringWithContentsOfFile:file_path encoding:NSUTF8StringEncoding error:&the_error];
		if((nil == raw_test_script) || (the_error != nil))
		{
			NSLog(@"Error loading %@", file_path);
			if(the_error != nil)
			{
				NSLog(@"NSError is: %@", [the_error localizedDescription]);
				the_error = nil;
			}
			current_index_count = current_index_count + 1;
			[ns_progress setCompletedUnitCount:current_index_count];
			continue;
		}
		_Bool is_positive_test = Test262Helper_DetermineIfPositiveTestFromScript(raw_test_script);


		// clear string for next loop
		[concat_string setString:@""];

		[concat_string appendString:Test262Helper_DetermineStrictModeStringFromScript(raw_test_script)];
		[concat_string appendString:@"\n"];
		[concat_string appendString:test_harness_script_string];
		[concat_string appendString:@"\n"];
		[concat_string appendString:raw_test_script];


		JSStringRef js_script_string = JSStringCreateWithCFString((__bridge CFStringRef)concat_string);
		JSStringRef js_file_name = JSStringCreateWithCFString((__bridge CFStringRef)file_path);
		JSStringRef js_exception_string = NULL;
		JSStringRef js_stack_string = NULL;

		_Bool is_success = Test262_EvaluateStringScript(js_script_string, js_file_name, &js_exception_string, &js_stack_string);



//		NSLog(@"%@ is_success: %d", file_path, is_success);



	#if 1
		// FIXME: This likely deserves a better check.
		if(is_positive_test != is_success)
		{
//			numberOfFailedTests = numberOfFailedTests + 1;
			number_of_failed_tests = number_of_failed_tests +1;

			NSString* nscf_exception_string = nil;
			NSString* nscf_stack_string = nil;
			if(NULL != js_exception_string)
			{
				nscf_exception_string = (__bridge_transfer NSString*)JSStringCopyCFString(NULL, js_exception_string);
			}
			if(NULL != js_stack_string)
			{
				nscf_stack_string = (__bridge_transfer NSString*)JSStringCopyCFString(NULL, js_stack_string);
			}

			//
			//                    			this.failedTests.Add(result);
			if(is_success && !is_positive_test)
			{
				//									Log.i("Test262", "Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				/*
				writeToLogFile("Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				*/

				NSLog(@"Test failed: (script passed but negative test means it should have not have passed): %@, %@, %@\n%@", file_path, nscf_exception_string, nscf_stack_string, raw_test_script);

			}
			else
			{
				//									Log.i("Test262", "Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				/*
				writeToLogFile("Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				*/
				NSLog(@"Test failed: %@, %@, %@\n%@", file_path, nscf_exception_string, nscf_stack_string, raw_test_script);

			}


		}
		else
		{
			//								Log.i("Test262", "Test passed: " + current_file_name);
//			writeToLogFile("Test passed: " + current_file_name);
//			NSLog(@"Test passed: %@", file_path);
		}

	#endif

		if(NULL != js_stack_string)
		{
			JSStringRelease(js_stack_string);
		}
		if(NULL != js_exception_string)
		{
			JSStringRelease(js_exception_string);
		}
		JSStringRelease(js_file_name);
		JSStringRelease(js_script_string);
		

		current_index_count = current_index_count + 1;
		[ns_progress setCompletedUnitCount:current_index_count];
	}

	NSLog(@"number_of_failed_tests=%lu", (unsigned long)number_of_failed_tests);

}

@implementation Test262Helper


@end
