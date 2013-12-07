package com.appcelerator.javascriptcore.opaquetypes;

public class JSClassRef extends PointerType {
    private JSClassDefinition definition;
    public JSClassRef(JSClassDefinition definition, long pointer) {
        super(pointer);
        this.definition = definition;
    }

    public JSClassDefinition getDefinition() {
        return definition;
    }
}
