package com.appcelerator.javascriptcore.opaquetypes;

public class JSObjectRef extends JSValueRef {
    public JSObjectRef(long pointer) {
        super(null, pointer);
    }
    public JSObjectRef(JSContextRef context, long pointer) {
        super(context, pointer);
    }
    public Object getPrivateObject() {
        return jsc.JSObjectGetPrivate(this);
    }
}
