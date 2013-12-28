
#import "HTTPConnection.h"

@class LogStreamWindowController;

@interface MyHTTPConnection : HTTPConnection

// Used to update the GUI
@property (weak) LogStreamWindowController* logStreamWindowController;
//@property (weak) NSNetService* netService;

@end
