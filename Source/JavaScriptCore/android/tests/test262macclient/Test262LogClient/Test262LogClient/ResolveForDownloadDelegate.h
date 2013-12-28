//
//  ResolveForDownloadDelegate.h
//  Test262LogClient
//
//  Created by Eric Wing on 12/27/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import "ResolveDelegate.h"
@class HTTPServer;

@interface ResolveForDownloadDelegate : ResolveDelegate

@property(nonatomic, strong) HTTPServer* httpServer;

@end
