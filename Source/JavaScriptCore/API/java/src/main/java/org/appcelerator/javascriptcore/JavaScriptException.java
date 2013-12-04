package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;

public class JavaScriptException extends RuntimeException {
    public JavaScriptException(String message, JSValueRef exception) {
        super(message);
    }   
    public JavaScriptException(String message, Throwable cause, JSValueRef exception) {
        super(message, cause);
    }
    public JavaScriptException(Throwable cause, JSValueRef exception) {
        super(cause);
    }

}
