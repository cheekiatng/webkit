package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;

public class JavaScriptException extends RuntimeException {
    private JSValueRef value;
    public JavaScriptException(String message) {
        super(message);
    }
    public JavaScriptException(String message, Throwable cause) {
        super(message, cause);
    }
    public JavaScriptException(Throwable cause) {
        super(cause);
    }
    public JavaScriptException(long jsContextRef, long jsValueRef) {
        this.value = new JSValueRef(new JSContextRef(jsContextRef), jsValueRef);
    }

    public JSValueRef getValue() {
        return value;
    }
}
