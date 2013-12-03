package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSContextGroupRef;
import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSGlobalContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassRef;

public class JavaScriptCoreLibrary {

    static {
        System.loadLibrary("JavaScriptCoreJNI");
    }   

    private static final JavaScriptCoreLibrary instance = new JavaScriptCoreLibrary();

    /*
     * Singleton
     */
    private JavaScriptCoreLibrary() {}
    public static JavaScriptCoreLibrary getInstance() {
        return instance;
    }

    /*
     * Native method wrappers
     */
    public JSObjectRef JSContextGetGlobalObject(JSContextRef context) {
        return new JSObjectRef(NativeJSContextGetGlobalObject(context.p()));
    }

    public JSContextGroupRef JSContextGetGroup(JSContextRef context) {
        return new JSContextGroupRef(NativeJSContextGetGroup(context.p()));
    }

    public JSContextGroupRef JSContextGroupCreate() {
        return new JSContextGroupRef(NativeJSContextGroupCreate());
    }

    public void JSContextGroupRelease(JSContextGroupRef group) {
        NativeJSContextGroupRelease(group.p());
    }

    public JSContextGroupRef JSContextGroupRetain(JSContextGroupRef group) {
        return new JSContextGroupRef(NativeJSContextGroupRetain(group.p()));
    }

    public JSGlobalContextRef JSGlobalContextCreate(JSClassRef jsClass) {
        return new JSGlobalContextRef(NativeJSGlobalContextCreate(jsClass.p()));
    }

    public JSGlobalContextRef JSGlobalContextCreateInGroup(JSContextGroupRef group, JSClassRef jsClass) {
        return new JSGlobalContextRef(NativeJSGlobalContextCreateInGroup(group.p(), jsClass.p()));
    }

    public void JSGlobalContextRelease(JSGlobalContextRef context) {
        NativeJSGlobalContextRelease(context.p());
    }

    public JSGlobalContextRef JSGlobalContextRetain(JSGlobalContextRef context) {
        return new JSGlobalContextRef(NativeJSGlobalContextRetain(context.p()));
    }

    public JSValueRef JSEvaluateScript(JSContextRef context, String script, JSValueRef exception) {
        return new JSValueRef(context, NativeJSEvaluateScript(context.p(), script, exception.p()));
    }

    public boolean JSCheckScriptSyntax(JSContextRef context, String script, JSValueRef exception) {
        return NativeJSCheckScriptSyntax(context.p(), script, exception.p());
    }

    public void JSGarbageCollect(JSContextRef context) {
        NativeJSGarbageCollect(context.p());
    }

    public JSValueRef JSValueMakeNull(JSContextRef context) {
        return new JSValueRef(context, NativeJSValueMakeNull(context.p()));
    }

    public JSValueRef JSValueMakeUndefined(JSContextRef context) {
        return new JSValueRef(context, NativeJSValueMakeUndefined(context.p()));
    }

    public boolean JSValueIsProtect(JSContextRef context, JSValueRef value) {
        return NativeJSValueProtect(context.p(), value.p());
    }

    public boolean JSValueIsUnprotect(JSContextRef context, JSValueRef value) {
        return NativeJSValueUnprotect(context.p(), value.p());
    }

    public boolean JSValueIsUndefined(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsUndefined(context.p(), value.p());
    }

    public boolean JSValueIsNull(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsNull(context.p(), value.p());
    }

    public boolean JSValueIsNumber(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsNumber(context.p(), value.p());
    }

    public boolean JSValueIsBoolean(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsBoolean(context.p(), value.p());
    }

    public boolean JSValueIsString(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsString(context.p(), value.p());
    }

    public boolean JSValueIsObject(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsObject(context.p(), value.p());
    }

    public boolean JSValueIsEqual(JSContextRef context, JSValueRef a, JSValueRef b, JSValueRef exception) {
        return NativeJSValueIsEqual(context.p(), a.p(), b.p(), exception.p());
    }

    public boolean JSValueIsInstanceOfConstructor(JSContextRef context, JSValueRef value, JSObjectRef constructor, JSValueRef exception) {
        return NativeJSValueIsInstanceOfConstructor(context.p(), value.p(), constructor.p(), exception.p());
    }

    public boolean JSValueIsObjectOfClass(JSContextRef context, JSValueRef value, JSClassRef jsClass) {
        return NativeJSValueIsObjectOfClass(context.p(), value.p(), jsClass.p());
    }

    public boolean JSValueIsStrictEqual(JSContextRef context, JSValueRef a, JSValueRef b) {
        return NativeJSValueIsStrictEqual(context.p(), a.p(), b.p());
    }

    public boolean JSValueToBoolean(JSContextRef context, JSValueRef value) {
        return NativeJSValueToBoolean(context.p(), value.p());
    }

    public double JSValueToNumber(JSContextRef context, JSValueRef value, JSValueRef exception) {
        return NativeJSValueToNumber(context.p(), value.p(), exception.p());
    }

    public String JSValueToStringCopy(JSContextRef context, JSValueRef value, JSValueRef exception) {
        return NativeJSValueToStringCopy(context.p(), value.p(), exception.p());
    }

    public JSObjectRef JSValueToObject(JSContextRef context, JSValueRef value, JSValueRef exception) {
        return new JSObjectRef(NativeJSValueToObject(context.p(), value.p(), exception.p()));
    }

    /*
     * Native methods
     */
    public native long NativeJSContextGetGlobalObject(long jsContextRef);
    public native long NativeJSContextGetGroup(long jsContextRef);
    public native long NativeJSContextGroupCreate();
    public native void NativeJSContextGroupRelease(long jsContextGroupRef);
    public native long NativeJSContextGroupRetain(long jsContextGroupRef);
    public native long NativeJSGlobalContextCreate(long jsClassRef);
    public native long NativeJSGlobalContextCreateInGroup(long jsContextGroupRef, long jsClassRef);
    public native void NativeJSGlobalContextRelease(long jsContextRef);
    public native long NativeJSGlobalContextRetain(long jsContextRef);
    public native long NativeJSEvaluateScript(long jsContextRef, String script, long jsExceptionRef);
    public native boolean NativeJSCheckScriptSyntax(long jsContextRef, String script, long jsExceptionRef);
    public native long NativeJSGarbageCollect(long jsContextRef);
    public native long NativeJSValueMakeNull(long jsContextRef);
    public native long NativeJSValueMakeUndefined(long jsContextRef);
    public native boolean NativeJSValueProtect(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueUnprotect(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsObjectOfClass(long jsContextRef, long jsValueRef, long jsClassRef);
    public native boolean NativeJSValueIsInstanceOfConstructor(long jsContextRef, long jsValueRef, long jsObjectRef, long jsExceptionRef);
    public native boolean NativeJSValueIsUndefined(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsNull(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsNumber(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsBoolean(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsString(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsObject(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsEqual(long jsContextRef, long jsValueRefA, long jsValueRefB, long jsExceptionRef);
    public native boolean NativeJSValueIsStrictEqual(long jsContextRef, long jsValueRefA, long jsValueRefB);
    public native boolean NativeJSValueToBoolean(long jsContextRef, long jsValueRef);
    public native double NativeJSValueToNumber(long jsContextRef, long jsValueRef, long jsExceptionRef);
    public native long NativeJSValueToObject(long jsContextRef, long jsValueRef, long jsExceptionRef);
    public native String NativeJSValueToStringCopy(long jsContextRef, long jsValueRef, long jsExceptionRef);
}
