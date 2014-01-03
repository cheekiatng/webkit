//
//  NetworkHelper.h
//  Test262
//
//  Created by Eric Wing on 1/1/14.
//  Copyright (c) 2014 JavaScriptCore. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NetworkHelper : NSObject

@property(nonatomic, strong, readonly) NSString* serviceName;

- (void) startServer;
- (void) stopServer;

@end
