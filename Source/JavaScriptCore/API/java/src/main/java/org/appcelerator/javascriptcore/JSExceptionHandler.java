package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;

public interface JSExceptionHandler {
    public void handleException(JSContextRef context, JSValueRef value);
}
