//
//  NetworkHelperForAppDelegate.h
//  Test262
//
//  Created by Eric Wing on 1/1/14.
//  Copyright (c) 2014 JavaScriptCore. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "LogWrapper.h"

void NetworkHelperForAppDelegate_UploadFile(int accepted_socket, uint16_t http_server_port, void* user_data);

void NetworkHelperForAppDelegate_OpenLogStream(int accepted_socket, void* log_wrapper, void* user_data);

@protocol NetworkHelperForAppDelegate <NSObject>
- (NSString*) logFileLocationString;
- (LogWrapper*) logWrapper;
@optional
- (NSString*) serviceName;
@end

@interface NetworkHelperForAppDelegate : NSObject

@end
