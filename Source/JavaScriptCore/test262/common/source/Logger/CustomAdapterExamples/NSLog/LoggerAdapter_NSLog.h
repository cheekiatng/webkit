//  Created by Eric Wing on 12/30/13.
#ifndef LOGGERADAPTER_NSLOG_H
#define LOGGERADAPTER_NSLOG_H

// I learned from experience that NSLog is treated as special by Apple on iOS and in the iOS simulator,
// and in some cases, this is the only way to get output displayed through the console.
// I also learned the hard way that ASLog is not the same as NSLog, despite that it is implied that it is.

int LoggerAdapter_CustomPrintfToNSLog(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, ...);
int LoggerAdapter_CustomPrintfvToNSLogv(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* format, va_list argp);
int LoggerAdapter_CustomPutsToNSPuts(Logger* logger, void* userdata, unsigned int priority, const char* keyword, const char* subkeyword, const char* text);

#endif /* LOGGERADAPTER_NSLOG_H */

