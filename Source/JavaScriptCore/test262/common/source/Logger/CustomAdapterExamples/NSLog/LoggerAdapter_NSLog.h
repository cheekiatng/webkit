//  Created by Eric Wing on 12/30/13.
#ifndef LOGGERADAPTER_NSLOG_H
#define LOGGERADAPTER_NSLOG_H

#include "Logger.h"

// I learned from experience that NSLog is treated as special by Apple on iOS and in the iOS simulator,
// and in some cases, this is the only way to get output displayed through the console.
// I also learned the hard way that ASLog is not the same as NSLog, despite that it is implied that it is.

// WARNING: The number of bytes written return values are wrong because they don't include the NSLog stamp.
// Since NSLog stamps may contain appliation names and process id numbers (not necessarily with fixed-digits), 
// I don't know how many bytes there are. 
// So I will just return the string length.

int LoggerAdapter_CustomPrintfToNSLog(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...);
int LoggerAdapter_CustomPrintfvToNSLogv(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp);
int LoggerAdapter_CustomPutsToNSPuts(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text);

// Helper function to set up the custom function.
// This is helpful for language bindings (like Java) where dealing with C function pointers is hard.
void Logger_Cocoa_SetCustomPrintFunctionToNSLog(Logger* logger);
void Logger_Cocoa_ClearCustomPrintFunctions(Logger* logger);

#endif /* LOGGERADAPTER_NSLOG_H */

