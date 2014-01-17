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

void Test262Helper_RunTests(LogWrapper* log_wrapper, AAssetManager* asset_manager);

#endif /* TEST262HELPERANDROID_H */
