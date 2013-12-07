package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;

public class JSContextRef extends PointerType {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();

    public JSContextRef(long pointer) {
        super(pointer);
    }

    public JSValueRef evaluateScript(String script) {
        JSValueRef exception = jsc.JSValueMakeNull(this);
        JSValueRef value = jsc.JSEvaluateScript(this, script, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
        }
        return value;
    }

    public boolean checkScriptSyntax(String script) {
        JSValueRef exception = jsc.JSValueMakeNull(this);
        boolean value = jsc.JSCheckScriptSyntax(this, script, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
        }
        return value;
    }

    public void garbageCollect() {
        jsc.JSGarbageCollect(this);
    }

    public void gc() {
        garbageCollect();
    }
}
