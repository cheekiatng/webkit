//https://developer.apple.com/library/mac/qa/qa1487/_index.html

@interface NSAttributedString (Hyperlink)
    +(id)hyperlinkFromString:(NSString*)inString withURL:(NSURL*)aURL;
@end
 
