package com.appcelerator.javascriptcore.opaquetypes;

public class JSObjectRef extends PointerType {
    // defaults to NULL pointer
    public JSObjectRef() {
        super();
    }
    public JSObjectRef(long pointer) {
        super(pointer);
    }
}
