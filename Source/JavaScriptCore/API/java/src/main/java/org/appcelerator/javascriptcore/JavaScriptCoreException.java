package com.appcelerator.javascriptcore;

public class JavaScriptCoreException extends RuntimeException {
    public JavaScriptCoreException(String message) {
        super(message);
    }   
    public JavaScriptCoreException(String message, Throwable cause) {
        super(message, cause);
    }
    public JavaScriptCoreException(Throwable cause) {
        super(cause);
    }
}