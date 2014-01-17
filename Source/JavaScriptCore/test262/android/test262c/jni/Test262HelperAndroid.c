#include "Test262HelperAndroid.h"
#include <assert.h>
#include <string.h>
#include <sys/types.h> // need for off_t for asset_manager because it looks like r9b broke something.
#include <stdbool.h>

#include <android/asset_manager.h>
#include <android/log.h>

#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>

#include "CommonUtils.h"
#include "TimeStamp.h"
#include "LogWrapper.h"


// Note: The strategy of this file is to try to avoid anything that touches JNI. Use the _JNI file for that.
// There are build system reasons for this (related to hard-fp registers).

extern _Bool Test262_EvaluateStringScript(JSStringRef js_script_string, JSStringRef js_file_name, JSStringRef* out_error_string, JSStringRef* out_stack_string);

// This returns a string created on the heap. You are responsible for calling free() on it.
char* Test262HelperAndroid_LoadTestHarnessScriptsAndCreateString(size_t* out_buffer_size, AAssetManager* asset_manager)
{
//AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

	off_t total_file_size = 0;
	char* string_buffer;
	int bytes_read;
	char* current_buffer_ptr;

	// For minor ease and efficiency, I'll compute the file sizes first for the buffer beforehand
	// so I can use just one large buffer.
	AAsset* asset_cth = AAssetManager_open(asset_manager, "harness/cth.js", AASSET_MODE_UNKNOWN);
	if(NULL == asset_cth)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Test262Helper_LoadTestHarnessScripts", "Failed to open harness/cth.js");
	}
	off_t file_size_cth = AAsset_getLength(asset_cth);
	total_file_size = total_file_size + file_size_cth;

	AAsset* asset_sta = AAssetManager_open(asset_manager, "harness/sta.js", AASSET_MODE_UNKNOWN);
	if(NULL == asset_sta)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Test262Helper_LoadTestHarnessScripts", "Failed to open harness/sta.js");
	}
	off_t file_size_sta = AAsset_getLength(asset_sta);
	total_file_size = total_file_size + file_size_sta;

	AAsset* asset_ed = AAssetManager_open(asset_manager, "harness/ed.js", AASSET_MODE_UNKNOWN);
	if(NULL == asset_ed)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Test262Helper_LoadTestHarnessScripts", "Failed to open harness/ed.js");
	}
	off_t file_size_ed = AAsset_getLength(asset_ed);
	total_file_size = total_file_size + file_size_ed;

	AAsset* asset_test_built_in_object = AAssetManager_open(asset_manager, "harness/testBuiltInObject.js", AASSET_MODE_UNKNOWN);
	if(NULL == asset_test_built_in_object)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Test262Helper_LoadTestHarnessScripts", "Failed to open harness/testBuiltInObject.js");
	}
	off_t file_size_built_in_object = AAsset_getLength(asset_test_built_in_object);
	total_file_size = total_file_size + file_size_built_in_object;

	AAsset* asset_test_intl = AAssetManager_open(asset_manager, "harness/testIntl.js", AASSET_MODE_UNKNOWN);
	if(NULL == asset_test_intl)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Test262Helper_LoadTestHarnessScripts", "Failed to open harness/testIntl.js");
	}
	off_t file_size_intl = AAsset_getLength(asset_test_intl);
	total_file_size = total_file_size + file_size_intl;


	// For paranoia reasons, I'm going to inject a newline at the end of each file (because I was bit by not doing this before in the Java version).
	// That will add 5 bytes (for 5 files). (Yes, I could omit the final one, but keep the logic simple.)
	// We also need a null terminator, so add 1 byte.
	size_t string_buffer_size = total_file_size + 5 + 1;
	string_buffer = (char*)malloc( string_buffer_size * sizeof(char) );
	if(NULL == string_buffer)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Test262Helper_LoadTestHarnessScripts", "malloc failed");
		return NULL;
	}
	if(NULL != out_buffer_size)
	{
		*out_buffer_size = string_buffer_size;
	}

	current_buffer_ptr = &string_buffer[0];

	// read the file into the string buffer
	bytes_read = AAsset_read(asset_cth, current_buffer_ptr, file_size_cth);
	assert(bytes_read == file_size_cth);
	AAsset_close(asset_cth);	
	// increment the string pointer for the next write into the buffer
	current_buffer_ptr = current_buffer_ptr + bytes_read;
	// add a newline
	*current_buffer_ptr = '\n';
	current_buffer_ptr = current_buffer_ptr + 1;


	// read the file into the string buffer
	bytes_read = AAsset_read(asset_sta, current_buffer_ptr, file_size_sta);
	assert(bytes_read == file_size_sta);
	AAsset_close(asset_sta);	
	// increment the string pointer for the next write into the buffer
	current_buffer_ptr = current_buffer_ptr + bytes_read;
	// add a newline
	*current_buffer_ptr = '\n';
	current_buffer_ptr = current_buffer_ptr + 1;

	// read the file into the string buffer
	bytes_read = AAsset_read(asset_ed, current_buffer_ptr, file_size_ed);
	assert(bytes_read == file_size_ed);
	AAsset_close(asset_ed);	
	// increment the string pointer for the next write into the buffer
	current_buffer_ptr = current_buffer_ptr + bytes_read;
	// add a newline
	*current_buffer_ptr = '\n';
	current_buffer_ptr = current_buffer_ptr + 1;

	// read the file into the string buffer
	bytes_read = AAsset_read(asset_test_built_in_object, current_buffer_ptr, file_size_built_in_object);
	assert(bytes_read == file_size_built_in_object);
	AAsset_close(asset_test_built_in_object);	
	// increment the string pointer for the next write into the buffer
	current_buffer_ptr = current_buffer_ptr + bytes_read;
	// add a newline
	*current_buffer_ptr = '\n';
	current_buffer_ptr = current_buffer_ptr + 1;

	// read the file into the string buffer
	bytes_read = AAsset_read(asset_test_intl, current_buffer_ptr, file_size_intl);
	assert(bytes_read == file_size_intl);
	AAsset_close(asset_test_intl);	
	// increment the string file_size_intl for the next write into the buffer
	current_buffer_ptr = current_buffer_ptr + bytes_read;
	// add a newline
	*current_buffer_ptr = '\n';
	current_buffer_ptr = current_buffer_ptr + 1;

	// add the null terminator
	*current_buffer_ptr = '\0';

	return string_buffer;
}

// Returns a string literal. You do not need to free the memory.
const char* Test262Helper_DetermineStrictModeStringFromScript(const char* raw_script)
{
	char* is_found = strstr(raw_script, "@onlyStrict");
	if(is_found != NULL)
	{
		// found it
		return "\"use strict\";\nvar strict_mode = true;\n";
	}
	else
	{
		return "var strict_mode = false; \n";
	}
}

_Bool Test262Helper_DetermineIfPositiveTestFromScript(const char* raw_script)
{
	char* is_found = strstr(raw_script, "@negative");	
	if(is_found != NULL)
	{
		// found it
		return false;
	}
	else
	{
		return true;
	}
}

static LogWrapper* s_logWrapper = NULL;
// From the raw character buffer, this will parse for all the newlines, and create an array of strings for each line.
// This creates a file_list array and returns it. The number of items in the array is returned by reference via out_total_number_of_tests.
// You will need to pass both of these values to Test262Helper_FreeFileList() to clean up when you are done.
char** Test262Helper_CreateFileListFromBuffer(const char* restrict file_buffer, size_t buffer_size, size_t* restrict out_total_number_of_tests)
{
	// count the number of newlines
	size_t total_number_of_files = 0;
	const char* newline_ptr = &file_buffer[0];
	size_t i;
	for(i=0; i<buffer_size; i++)
	{
		if('\n' == file_buffer[i])
		{
			total_number_of_files = total_number_of_files + 1;
		}
	}
//	fprintf(stderr, "%lu occurences of newline\n", total_number_of_files);

	char** file_list = (char**)malloc(total_number_of_files * sizeof(char*));

	
	const char* begin_ptr = &file_buffer[0];
//	fprintf(stderr, "begin_ptr %s\n", begin_ptr);
	
	i = 0;
	while(NULL != (newline_ptr = strchr(begin_ptr, '\n')))
	{
		// Remember: This doesn't include null terminator which we need to add. 
		// Also remember: This subtraction already omits the newline which we intentionally want to remove.
		size_t string_length = newline_ptr - begin_ptr;
		file_list[i] = (char*)malloc( (string_length + 1) * sizeof(char) );
		// strlcpy copies size-1 and auto-terminates the string.
		strlcpy(file_list[i], begin_ptr, string_length+1);
//		strncpy(file_list[i], begin_ptr, string_length+1);
//		file_list[i] = '\0';
//		fprintf(stderr, "string_length:%lu, created: %s.\n", string_length, file_list[i]);
//		LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_CreateFileListFromBuffer", "string_length:%lu, for i:%d, created: %s.\n", string_length, i, file_list[i]);
		
		i++;
		// move to the next character after the newline
		begin_ptr = newline_ptr + 1;
	}
	assert(i == total_number_of_files);
//	fprintf(stderr, "created %lu strings\n", i);

/*	
	for(i=0; i<total_number_of_files; i++)
	{
	//	fprintf(stderr, "%s.\n", file_list[i]);
		LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_CreateFileListFromBuffer", "file_list[%d]: %s.", i, file_list[i]);
	}
*/
	if(NULL != out_total_number_of_tests)
	{
		*out_total_number_of_tests = total_number_of_files;
	}

	return file_list;


}

void Test262Helper_FreeFileList(char** restrict file_list, size_t total_number_of_files)
{
	size_t i;
	for(i=0; i<total_number_of_files; i++)
	{
		free(file_list[i]);
	}
	free(file_list);
}

// This creates a file_list array. The number of items in the array is returned by reference via out_total_number_of_tests.
// You will need to pass both of these values to Test262Helper_FreeFileList() to clean up when you are done.
char** Test262HelperAndroid_CreateFileList(size_t* restrict out_total_number_of_tests, AAssetManager* restrict asset_manager)
{
	/* standard C would look something like this, but Android makes us rewrite it for AAssetManager. */
	/*
	FILE* file_handle = fopen("/Users/ewing/TEMP/test262_filelist.txt", "r");

	//Steak to the end of the file to determine the file size
	fseek(file_handle, 0L, SEEK_END);
	long file_size = ftell(file_handle);
	fseek(file_handle, 0L, SEEK_SET);

	
	//Allocate enough memory (add 1 for the \0, since fread won't add it)
	char* file_buffer = (char*)malloc(file_size+1*sizeof(char));

	//Read the file 
	size_t bytes_read = fread(file_buffer, file_size, 1, file_handle);
	file_buffer[file_size] = '\0'; // Add terminating zero.

	//Print it again for debugging
//	printf("Read %s\n", file_buffer);

	// Close the file
	fclose(file_handle);
	*/

	AAsset* the_asset = AAssetManager_open(asset_manager, "test262_filelist.txt", AASSET_MODE_UNKNOWN);
	if(NULL == the_asset)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Test262HelperAndroid_CreateFileList", "test262_filelist.txt");
	}
	off_t file_size = AAsset_getLength(the_asset);
	// Allocate enough memory (add 1 for the \0, since fread won't add it)
	char* file_buffer = (char*)malloc(file_size+1*sizeof(char));
	// read the whole file into the buffer
	int bytes_read = AAsset_read(the_asset, file_buffer, file_size);
	assert(bytes_read == file_size);
	AAsset_close(the_asset);

	char** file_list = Test262Helper_CreateFileListFromBuffer(file_buffer, file_size, out_total_number_of_tests);

	free(file_buffer);

	return file_list;
}

#if 1
/*
void Test262Helper_RunTests(NSProgress* ns_progress, LogWrapper* log_wrapper,
	NSInteger (^callback_for_all_tests_starting)(NSUInteger total_number_of_tests),
	NSInteger (^callback_for_beginning_test)(NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number),
	NSInteger (^callback_for_ending_test)(NSString* test_file, NSUInteger total_number_of_tests, NSUInteger current_test_number, NSUInteger total_number_of_tests_failed, _Bool did_pass, NSString* nscf_exception_string, NSString* nscf_stack_string),
	NSInteger (^callback_for_all_tests_finished)(NSUInteger total_number_of_tests, NSUInteger number_of_tests_run, NSUInteger total_number_of_tests_failed)
)
*/
void Test262Helper_RunTests(LogWrapper* log_wrapper, AAssetManager* asset_manager)
{
s_logWrapper = log_wrapper;

	const int TIMESTAMP_LENGTH = 24;
	char current_time[TIMESTAMP_LENGTH];
	TimeStamp_GetTimeStamp(current_time, TIMESTAMP_LENGTH);
	// Being from a gaming background, I prefer tick clocks because it is easier to do math on.
	CommonUtils_InitTime();
	unsigned long start_time_in_ticks = CommonUtils_GetTicks();


	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_STANDARD, LOGWRAPPER_PRIMARY_KEYWORD, "Starting Tests", "Starting Tests at time: %s", current_time);

	size_t test_harness_script_string_buffer_size = 0;
	// remember to free this later
	char* test_harness_script_string = Test262HelperAndroid_LoadTestHarnessScriptsAndCreateString(&test_harness_script_string_buffer_size, asset_manager);


//	NSMutableString* concat_string = [[NSMutableString alloc] init];

//	NSString* suite_dir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"suite"];

//	NSString* resource_path = [[NSBundle mainBundle] resourcePath];



	size_t number_of_reallocs_for_raw_script = 0;
	size_t number_of_reallocs_for_concat_string = 0;

	char* raw_test_script = NULL;
	size_t raw_test_script_allocated_memory = 0;
	char* concat_string = NULL;
	size_t concat_string_allocated_memory = 0;
//#define TEST262_USE_MAGIC_NUMBERS 1
// If defined, this will preallocate memory, ideally in a way that doesn't need to keep realloc'ing.
#if TEST262_USE_MAGIC_NUMBERS
	// Recording the run, these were the max numbers:
	// raw_test_script_allocated_memory=59783, concat_string_allocated_memory=132776
	raw_test_script = (char*)malloc(59784);
	raw_test_script_allocated_memory = 59784;

	concat_string = (char*)malloc(132776);
	concat_string_allocated_memory = 132776;
#endif

//	NSError* the_error = nil;
//	NSString* file_path = nil;

	unsigned int number_of_failed_tests = 0;
	unsigned int current_index_count = 0;
	_Bool should_keep_running = true;

	/* I originally was going to dynamically traverse the directories, but on Android,
		there was a serious performance problem with this (took 3 hours just to get a full directory listing in an APK).
		So instead, I used a pre-created text file which contained the list of files.
	 */

	size_t total_number_of_tests = 0;
	// remember to free this later
	char** file_list = Test262HelperAndroid_CreateFileList(&total_number_of_tests, asset_manager);


		for(size_t i=0; i<total_number_of_tests; i++)
	{
	//	fprintf(stderr, "%s.\n", file_list[i]);
		LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "debug4", "file_list[%d]: %s.", i, file_list[i]);
	}


	
//		NSString* test_manifiest_file_string = [NSString stringWithContentsOfFile:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"test262_filelist.txt"] encoding:NSUTF8StringEncoding error:&the_error];
//	NSArray* file_list = [test_manifiest_file_string componentsSeparatedByString:@"\n"];


	// We might want to further sanitize the list if we support commented out lines and so forth
//	[ns_progress setTotalUnitCount:[file_list count]];

#if 0
	if(nil != callback_for_all_tests_starting)
	{
		callback_for_all_tests_starting([file_list count]);
	}
#endif
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "entering main loop");

	size_t i;
	for(i=0; i<total_number_of_tests; i++)
	{
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "loop:%d", i);
		if(!should_keep_running)
		{
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "!should_keep_running");
			break;
		}
		char* file_path = file_list[i];
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "assign file_path");
		if(NULL == file_path)
		{
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "file_path is NULL");
			current_index_count = current_index_count + 1;
			continue;			
		}
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "strlen file_path:%d", strlen(file_path));
		
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "file_path:%s", file_path);

		// just in case there is a blank line
		if(strlen(file_path) == 0)
		{
			current_index_count = current_index_count + 1;
//			[ns_progress setCompletedUnitCount:current_index_count];
	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262Helper_RunTests", "strlen==0");
			continue;
		}
		
		AAsset* file_asset = AAssetManager_open(asset_manager, file_path, AASSET_MODE_UNKNOWN);
		if(NULL == file_asset)
		{
			LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_CRITICAL, LOGWRAPPER_PRIMARY_KEYWORD, "Error", "Error loading file: %s", file_path);
			current_index_count = current_index_count + 1;
//			[ns_progress setCompletedUnitCount:current_index_count];
			continue;
		}
		off_t file_size = AAsset_getLength(file_asset);
		// file_size doesn't contain a null terminator, while raw_test_script_allocated_memory does, so adjust accordingly.
		if(file_size >= raw_test_script_allocated_memory)
		{
//	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "realloc raw_test_script", "file_size:%d", file_size);
			raw_test_script = realloc(raw_test_script, file_size+1);
			assert(raw_test_script != NULL);
			raw_test_script_allocated_memory = file_size+1;
			number_of_reallocs_for_raw_script++;
		}
		int bytes_read = AAsset_read(file_asset, raw_test_script, file_size);
//		assert(bytes_read == file_size);
		// null terminate the string
		raw_test_script[file_size] = '\0';
		AAsset_close(file_asset);	

#if 0
		if(nil != callback_for_beginning_test)
		{
			NSInteger return_status = callback_for_beginning_test(file_path, [file_list count], current_index_count+1);
			if(0 == return_status)
			{
				// 0 means user wants to cancel tests
				should_keep_running = false;
			}
		}
#endif
		_Bool is_positive_test = Test262Helper_DetermineIfPositiveTestFromScript(raw_test_script);

		const char* strict_mode_bootstrap_string = Test262Helper_DetermineStrictModeStringFromScript(raw_test_script);

		// also add 2 extra newlines and the null terminator
		size_t needed_bytes_for_full_script = strlen(strict_mode_bootstrap_string) + test_harness_script_string_buffer_size + strlen(raw_test_script) + 2 + 1; 
		if(needed_bytes_for_full_script > concat_string_allocated_memory)
		{
			concat_string = realloc(concat_string, needed_bytes_for_full_script);
			assert(concat_string != NULL);
			concat_string_allocated_memory = needed_bytes_for_full_script;
			number_of_reallocs_for_concat_string++;
		}

		// copy the first string over
		strcpy(concat_string, strict_mode_bootstrap_string);
		// then concat the remaining parts
		strcat(concat_string, "\n");
		strcat(concat_string, test_harness_script_string);
		strcat(concat_string, "\n");
		strcat(concat_string, raw_test_script);


		JSStringRef js_script_string = JSStringCreateWithUTF8CString(concat_string);
		JSStringRef js_file_name = JSStringCreateWithUTF8CString(file_path);
		JSStringRef js_exception_string = NULL;
		JSStringRef js_stack_string = NULL;

		LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_STANDARD, LOGWRAPPER_PRIMARY_KEYWORD, "Evaluating Script", "Evaluating Script: %s", file_path);


		_Bool is_success = Test262_EvaluateStringScript(js_script_string, js_file_name, &js_exception_string, &js_stack_string);



//		NSLog(@"%@ is_success: %d", file_path, is_success);



		// FIXME: This likely deserves a better check.
		if(is_positive_test != is_success)
		{
//			numberOfFailedTests = numberOfFailedTests + 1;
			number_of_failed_tests = number_of_failed_tests +1;

			char* c_exception_string = NULL;
			char* c_stack_string = NULL;

			if(NULL != js_exception_string)
			{
				// this function already includes the null terminator in the size
				size_t bytes_needed = JSStringGetMaximumUTF8CStringSize(js_exception_string);
				char* c_exception_string = (char*)malloc(bytes_needed*sizeof(char));
				JSStringGetUTF8CString(js_exception_string, c_exception_string, bytes_needed);
			}
			if(NULL != js_stack_string)
			{
				// this function already includes the null terminator in the size
				size_t bytes_needed = JSStringGetMaximumUTF8CStringSize(js_stack_string);
				char* c_stack_string = (char*)malloc(bytes_needed*sizeof(char));
				JSStringGetUTF8CString(js_stack_string, c_stack_string, bytes_needed);
			}

			//
			//                    			this.failedTests.Add(result);
			if(is_success && !is_positive_test)
			{
				//									Log.i("Test262", "Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				/*
				writeToLogFile("Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				*/

				//NSLog(@"Test failed: (script passed but negative test means it should have not have passed): %@, %@, %@\n%@", file_path, nscf_exception_string, nscf_stack_string, raw_test_script);
				LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_STANDARD, LOGWRAPPER_PRIMARY_KEYWORD, "Test failed", "Test failed: (script passed but negative test means it should have not have passed): %s, %s, %s\n%s", file_path, c_exception_string, c_stack_string, raw_test_script);

			}
			else
			{
				//									Log.i("Test262", "Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				/*
				writeToLogFile("Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
				*/
				//NSLog(@"Test failed: %@, %@, %@\n%@", file_path, nscf_exception_string, nscf_stack_string, raw_test_script);
				LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_STANDARD, LOGWRAPPER_PRIMARY_KEYWORD, "Test failed", "Test failed: %s, %s, %s\n%s", file_path, c_exception_string, c_stack_string, raw_test_script);
			}
#if 0
			if(nil != callback_for_ending_test)
			{
				NSInteger return_status = callback_for_ending_test(file_path, [file_list count], current_index_count+1, number_of_failed_tests, false, nscf_exception_string, nscf_stack_string);
				if(0 == return_status)
				{
					// 0 means user wants to cancel tests
					should_keep_running = false;
				}
			}
#endif
			free(c_stack_string);
			free(c_exception_string);
		}
		else
		{
			//								Log.i("Test262", "Test passed: " + current_file_name);
//			writeToLogFile("Test passed: " + current_file_name);
//			NSLog(@"Test passed: %@", file_path);
			LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_STANDARD, LOGWRAPPER_PRIMARY_KEYWORD, "Test passed", "Test passed: %s", file_path);
#if 0
			if(nil != callback_for_ending_test)
			{
				NSInteger return_status = callback_for_ending_test(file_path, [file_list count], current_index_count+1, number_of_failed_tests, true, nil, nil);
				if(0 == return_status)
				{
					// 0 means user wants to cancel tests
					should_keep_running = false;
				}
			}
#endif

		}


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
//		[ns_progress setCompletedUnitCount:current_index_count];
	}

	TimeStamp_GetTimeStamp(current_time, TIMESTAMP_LENGTH);
	unsigned long end_time_in_ticks = CommonUtils_GetTicks();

	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_STANDARD, LOGWRAPPER_PRIMARY_KEYWORD, "Tests completed",
		"Tests completed: %d of %d run.\n"
		"Total failed: %lu\n"
		"Total execution time: %lf seconds\n"
		"Timestamp: %s",
		current_index_count, total_number_of_tests,
		(unsigned long)number_of_failed_tests,
		// converting to double and seconds for consistency with Apple versions
		(double)(end_time_in_ticks - start_time_in_ticks)/1000.0,
		current_time
	);
#if 0
	if(nil != callback_for_all_tests_finished)
	{
		callback_for_all_tests_finished([file_list count], current_index_count+1, number_of_failed_tests);
	}
#endif
	// seems fair to free memory after the clock because GC cleans up later.
	// Also hard to say where ARC/autoreleasepool cleans up.
	free(concat_string);
	free(raw_test_script);
	free(test_harness_script_string);
	Test262Helper_FreeFileList(file_list, total_number_of_tests);

	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Debug Magic Number", "final raw_test_script_allocated_memory=%d, concat_string_allocated_memory=%d\nnumber_of_reallocs_for_raw_script=%d, number_of_reallocs_for_concat_string=%d", raw_test_script_allocated_memory, concat_string_allocated_memory, number_of_reallocs_for_raw_script, number_of_reallocs_for_concat_string);

}
#endif

