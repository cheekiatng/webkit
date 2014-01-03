//  Created by Eric Wing on 01/2/14.
#ifndef LOGGERADAPTER_ANDROIDLOG_H
#define LOGGERADAPTER_ANDROIDLOG_H

// Anybody who has had the misfortune of doing NDK work knows that stdout/stderr seem to go to /dev/null,
// and the claims that they can be re-routed to appear in the adb logcat are all lies.
// So the only way to see stuff in console output is call __android_log_* family of functions instead which goes to adb logcat.

int LoggerAdapter_CustomPrintfTo__android_log_print(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...);
int LoggerAdapter_CustomPrintfvTo__android_log_vprint(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp);
int LoggerAdapter_CustomPutsTo__android_log_write(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text);

#endif /* LOGGERADAPTER_ANDROIDLOG_H */

