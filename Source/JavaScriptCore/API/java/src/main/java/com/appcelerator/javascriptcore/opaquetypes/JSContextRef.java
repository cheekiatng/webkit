package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;

public class JSContextRef extends PointerType {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();

    public JSContextRef(long pointer) {
        super(pointer);
    }

    public JSValueRef evaluateScript(String script) {
        return evaluateScript(script, null, null);
    }

    public JSValueRef evaluateScript(String script, JSObjectRef thisObject, JSValueRef exception) {
        return jsc.JSEvaluateScript(this, script, thisObject, null, 1, exception);
    }

    public boolean checkScriptSyntax(String script) {
        return checkScriptSyntax(script, null);
    }

    public boolean checkScriptSyntax(String script, JSValueRef exception) {
        return jsc.JSCheckScriptSyntax(this, script, exception);
    }

    public void garbageCollect() {
        jsc.JSGarbageCollect(this);
    }

    public void gc() {
        garbageCollect();
    }
}
