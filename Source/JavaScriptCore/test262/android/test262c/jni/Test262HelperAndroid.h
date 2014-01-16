#ifndef TEST262HELPERANDROID_H
#define TEST262HELPERANDROID_H

#include <stddef.h>
#include <sys/types.h> // need for off_t for asset_manager because it looks like r9b broke something.
#include <android/asset_manager.h>

char* Test262HelperAndroid_CreateTestHarnessScripts(size_t* out_buffer_size, AAssetManager* asset_manager);
const char* Test262Helper_DetermineStrictModeStringFromScript(const char* raw_script);
_Bool Test262Helper_DetermineIfPositiveTestFromScript(const char* raw_script);

#endif /* TEST262HELPERANDROID_H */
