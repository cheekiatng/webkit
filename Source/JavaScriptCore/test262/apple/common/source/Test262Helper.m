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
	if(0 == is_in_range.location)
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
	if(0 == is_in_range.location)
	{
		// found it
		return false;
	}
	else
	{
		return true;
	}
}


void Test262Helper_RunTests()
{
	NSString* test_harness_script_string = Test262Helper_LoadTestHarnessScripts();
	NSMutableString* concat_string = [[NSMutableString alloc] init];

	NSString* suite_dir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"suite"];

	NSString* raw_test_script = nil;
	NSError* the_error = nil;
	NSString* file_path = nil;

	file_path = [suite_dir stringByAppendingPathComponent:@"ch06/6.1.js"];
	raw_test_script = [NSString stringWithContentsOfFile:file_path encoding:NSUTF8StringEncoding error:&the_error];
	if((nil == raw_test_script) || (the_error != nil))
	{
		NSLog(@"Error loading %@", file_path);
		if(the_error != nil)
		{
			NSLog(@"NSError is: %@", [the_error localizedDescription]);
			the_error = nil;
		}
	}
	_Bool is_positive_test = Test262Helper_DetermineIfPositiveTestFromScript(raw_test_script);


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

	NSLog(@"is_success: %d", is_success);



#if 0
	// FIXME: This likely deserves a better check.
	if(is_positive_test != is_success)
	{
		numberOfFailedTests = numberOfFailedTests + 1;

		//
		//                    			this.failedTests.Add(result);
		if(is_success && !is_positive_test)
		{
			//									Log.i("Test262", "Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
			writeToLogFile("Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
		}
		else
		{
			//									Log.i("Test262", "Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
			writeToLogFile("Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
		}
	}
	else
	{
		//								Log.i("Test262", "Test passed: " + current_file_name);
		writeToLogFile("Test passed: " + current_file_name);
	}

#endif

//	[concat_string setString:@""];



}

@implementation Test262Helper


@end
