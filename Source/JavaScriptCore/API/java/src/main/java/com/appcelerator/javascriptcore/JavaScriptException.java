package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;

public class JavaScriptException extends RuntimeException {
    public JavaScriptException(String message) {
        super(message);
    }
    public JavaScriptException(String message, Throwable cause) {
        super(message, cause);
    }
    public JavaScriptException(Throwable cause) {
        super(cause);
    }
}
