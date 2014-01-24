#ifndef TEST262HELPERANDROID_H
#define TEST262HELPERANDROID_H

#include <stddef.h>
#include <sys/types.h> // need for off_t for asset_manager because it looks like r9b broke something.
#include <android/asset_manager.h>
#include "LogWrapper.h"

char* Test262HelperAndroid_LoadTestHarnessScriptsAndCreateString(size_t* out_buffer_size, AAssetManager* asset_manager);
const char* Test262Helper_DetermineStrictModeStringFromScript(const char* raw_script);
_Bool Test262Helper_DetermineIfPositiveTestFromScript(const char* raw_script);
char** Test262Helper_CreateFileListFromBuffer(const char* restrict file_buffer, size_t buffer_size, size_t* restrict out_total_number_of_tests);
void Test262Helper_FreeFileList(char** restrict file_list, size_t total_number_of_files);
char** Test262HelperAndroid_CreateFileList(size_t* restrict out_total_number_of_tests, AAssetManager* restrict asset_manager);


//void Test262Helper_RunTests(LogWrapper* log_wrapper, AAssetManager* asset_manager);
void Test262Helper_RunTests(LogWrapper* log_wrapper, AAssetManager* asset_manager,
	int32_t (*callback_for_all_tests_starting)(uint32_t total_number_of_tests, void* user_data),
	int32_t (*callback_for_beginning_test)(const char* test_file, uint32_t total_number_of_tests, uint32_t current_test_number, void* user_data),
	int32_t (*callback_for_ending_test)(const char* test_file, uint32_t total_number_of_tests, uint32_t current_test_number, uint32_t total_number_of_tests_failed, _Bool did_pass, const char* exception_string, const char* stack_string, void* user_data),
	int32_t (*callback_for_all_tests_finished)(uint32_t total_number_of_tests, uint32_t number_of_tests_run, uint32_t total_number_of_tests_failed, double diff_time_in_double_seconds, void* user_data),
	void* user_data
);

#endif /* TEST262HELPERANDROID_H */
