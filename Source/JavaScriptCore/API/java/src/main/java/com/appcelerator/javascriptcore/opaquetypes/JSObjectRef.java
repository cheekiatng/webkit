package com.appcelerator.javascriptcore.opaquetypes;

public class JSObjectRef extends JSValueRef {
    public JSObjectRef(long pointer) {
        super(pointer);
    }
    public JSObjectRef(JSContextRef context, long pointer) {
        super(context, pointer);
    }
}
