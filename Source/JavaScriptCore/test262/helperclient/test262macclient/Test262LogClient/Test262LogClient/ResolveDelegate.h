//
//  ResolveDelegate.h
//  Test262LogClient
//
//  Created by Eric Wing on 12/27/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <Foundation/Foundation.h>

int Test262Helper_ConnectToServer(NSNetService* net_service);


@interface ResolveDelegate : NSObject <NSNetServiceDelegate>

- (void) handleConnectError:(NSNetService*)net_service withErrno:(int)error_no;

@end
