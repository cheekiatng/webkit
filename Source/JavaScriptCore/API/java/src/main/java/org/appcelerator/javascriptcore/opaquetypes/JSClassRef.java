package com.appcelerator.javascriptcore.opaquetypes;

public class JSClassRef extends PointerType {
    // defaults to NULL pointer
    public JSClassRef() {
        super();
    }

    public JSClassRef(long pointer) {
        super(pointer);
    }
}
