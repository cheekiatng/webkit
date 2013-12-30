//
//  LogStreamWindowController.h
//  Test262LogClient
//
//  Created by Eric Wing on 12/28/13.
//  Copyright (c) 2013 JavaScriptCore. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface LogStreamWindowController : NSWindowController
@property (weak) IBOutlet NSProgressIndicator* progressIndicator;
@property (weak) NSNetService* netService;

- (void) setWindowDidCloseCompletionBlock:(void (^)(void))the_block;

//- (void) postLogEvent:(void*)recv_buffer length:(NSUInteger)num_bytes;
- (void) postLogEvent:(NSString*)log_message;


@end
