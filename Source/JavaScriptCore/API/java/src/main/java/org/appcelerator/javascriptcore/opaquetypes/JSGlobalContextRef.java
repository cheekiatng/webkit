package com.appcelerator.javascriptcore.opaquetypes;

public class JSGlobalContextRef extends JSContextRef {
    // defaults to NULL pointer
    public JSGlobalContextRef() {
        super();
    }
    public JSGlobalContextRef(long pointer) {
        super(pointer);
    }
}
