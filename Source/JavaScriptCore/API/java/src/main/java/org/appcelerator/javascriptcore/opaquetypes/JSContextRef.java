package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;

public class JSContextRef extends PointerType {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();

    public JSContextRef(long pointer) {
        super(pointer);
    }

    public JSValueRef evaluateScript(String script) {
        return jsc.JSEvaluateScript(this, script);
    }

    public JSValueRef checkScriptSyntax(String script) {
        return jsc.JSCheckScriptSyntax(this, script);
    }

    public void garbageCollect() {
        jsc.JSGarbageCollect(this);
    }

    public void gc() {
        garbageCollect();
    }
}
