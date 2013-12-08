package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSContextGroupRef;
import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSGlobalContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassDefinition;
import com.appcelerator.javascriptcore.opaquetypes.PointerType;
import com.appcelerator.javascriptcore.opaquetypes.JSPropertyNameArrayRef;
import com.appcelerator.javascriptcore.opaquetypes.JSPropertyNameAccumulatorRef;

import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsConstructorCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsFunctionCallback;

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

    private long p(PointerType p) {
        if (p == null) return 0;
        return p.pointer();
    }

    /*
     * Native method wrappers
     */
    public JSObjectRef JSContextGetGlobalObject(JSContextRef context) {
        return new JSObjectRef(NativeJSContextGetGlobalObject(p(context)));
    }

    public JSContextGroupRef JSContextGetGroup(JSContextRef context) {
        return new JSContextGroupRef(NativeJSContextGetGroup(p(context)));
    }

    public JSContextGroupRef JSContextGroupCreate() {
        return new JSContextGroupRef(NativeJSContextGroupCreate());
    }

    public void JSContextGroupRelease(JSContextGroupRef group) {
        NativeJSContextGroupRelease(p(group));
    }

    public JSContextGroupRef JSContextGroupRetain(JSContextGroupRef group) {
        return new JSContextGroupRef(NativeJSContextGroupRetain(p(group)));
    }

    public JSGlobalContextRef JSGlobalContextCreate(JSClassRef jsClass) {
        return new JSGlobalContextRef(NativeJSGlobalContextCreate(p(jsClass)));
    }

    public JSGlobalContextRef JSGlobalContextCreateInGroup(JSContextGroupRef group, JSClassRef jsClass) {
        return new JSGlobalContextRef(NativeJSGlobalContextCreateInGroup(p(group), p(jsClass)));
    }

    public void JSGlobalContextRelease(JSGlobalContextRef context) {
        NativeJSGlobalContextRelease(p(context));
    }

    public JSGlobalContextRef JSGlobalContextRetain(JSGlobalContextRef context) {
        return new JSGlobalContextRef(NativeJSGlobalContextRetain(p(context)));
    }

    public JSValueRef JSEvaluateScript(JSContextRef context, String script) {
        return new JSValueRef(context, NativeJSEvaluateScript(p(context), script));
    }

    /*
     * @returns true if syntax is OK. Otherwise it returns error string or false.
     */
    public JSValueRef JSCheckScriptSyntax(JSContextRef context, String script) {
        return new JSValueRef(context, NativeJSCheckScriptSyntax(p(context), script));
    }

    public void JSGarbageCollect(JSContextRef context) {
        NativeJSGarbageCollect(p(context));
    }

    public void JSValueProtect(JSContextRef context, JSValueRef value) {
        NativeJSValueProtect(p(context), p(value));
    }

    public void JSValueUnprotect(JSContextRef context, JSValueRef value) {
        NativeJSValueUnprotect(p(context), p(value));
    }

    public String JSValueCreateJSONString(JSContextRef context, JSValueRef value, int indent) {
        return NativeJSValueCreateJSONString(p(context), p(value), indent);
    }

    public JSValueRef JSValueMakeBoolean(JSContextRef context, boolean value) {
        return new JSValueRef(context, NativeJSValueMakeBoolean(p(context), value));
    }

    public JSValueRef JSValueMakeNull(JSContextRef context) {
        return new JSValueRef(context, NativeJSValueMakeNull(p(context)));
    }

    public JSValueRef JSValueMakeNumber(JSContextRef context, double number) {
        return new JSValueRef(context, NativeJSValueMakeNumber(p(context), number));
    }

    public JSValueRef JSValueMakeString(JSContextRef context, String string) {
        return new JSValueRef(context, NativeJSValueMakeString(p(context), string));
    }

    public JSValueRef JSValueMakeUndefined(JSContextRef context) {
        return new JSValueRef(context, NativeJSValueMakeUndefined(p(context)));
    }

    public JSValueRef JSValueMakeFromJSONString(JSContextRef context, String string) {
        return new JSValueRef(context, NativeJSValueMakeString(p(context), string));
    }

    public boolean JSValueIsUndefined(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsUndefined(p(context), p(value));
    }

    public boolean JSValueIsNull(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsNull(p(context), p(value));
    }

    public boolean JSValueIsNumber(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsNumber(p(context), p(value));
    }

    public boolean JSValueIsBoolean(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsBoolean(p(context), p(value));
    }

    public boolean JSValueIsString(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsString(p(context), p(value));
    }

    public boolean JSValueIsObject(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsObject(p(context), p(value));
    }

    public boolean JSValueIsEqual(JSContextRef context, JSValueRef a, JSValueRef b) {
        return NativeJSValueIsEqual(p(context), p(a), p(b));
    }

    public boolean JSValueIsInstanceOfConstructor(JSContextRef context, JSValueRef value, JSObjectRef constructor) {
        return NativeJSValueIsInstanceOfConstructor(p(context), p(value), p(constructor));
    }

    public boolean JSValueIsObjectOfClass(JSContextRef context, JSValueRef value, JSClassRef jsClass) {
        return NativeJSValueIsObjectOfClass(p(context), p(value), p(jsClass));
    }

    public boolean JSValueIsStrictEqual(JSContextRef context, JSValueRef a, JSValueRef b) {
        return NativeJSValueIsStrictEqual(p(context), p(a), p(b));
    }

    public boolean JSValueToBoolean(JSContextRef context, JSValueRef value) {
        return NativeJSValueToBoolean(p(context), p(value));
    }

    public double JSValueToNumber(JSContextRef context, JSValueRef value) {
        return NativeJSValueToNumber(p(context), p(value));
    }

    public String JSValueToStringCopy(JSContextRef context, JSValueRef value) {
        return NativeJSValueToStringCopy(p(context), p(value));
    }

    public JSObjectRef JSValueToObject(JSContextRef context, JSValueRef value) {
        return new JSObjectRef(NativeJSValueToObject(p(context), p(value)));
    }

    public JSClassRef JSClassCreate(JSClassDefinition d) {
        // Construct names/attributes arrays on ahead to avoid reflection in JNI
        d.updateCache();

        // Avoid passing objects as much as possible
        return new JSClassRef(d, NativeJSClassCreate(
            d.version, d.attributes, d.className, p(d.parentClass),
            d.getStaticValueNames(), d.getStaticValueAttributes(),
            d.getStaticFunctionNames(), d.getStaticFunctionAttributes(),
            d.initialize != null, d.finalize != null, d.hasProperty != null,
            d.getProperty != null, d.setProperty != null,
            d.deleteProperty != null, d.getPropertyNames != null,
            d.callAsFunction != null, d.callAsConstructor != null,
            d.hasInstance != null, d.convertToType != null
            ));
    }

    public JSObjectRef JSObjectCallAsConstructor(JSContextRef context, JSObjectRef jsObject, JSValueRef argv[]) {
        long[] pargv = new long[argv.length];
        for (int i = 0; i < argv.length; i++) {
            pargv[i] = p(argv[i]);
        }
        return new JSObjectRef(NativeJSObjectCallAsConstructor(p(context), p(jsObject), pargv.length, pargv));
    }

    private long[] convertToLong(JSValueRef argv[]) {
        long[] pargv = new long[argv.length];
        for (int i = 0; i < argv.length; i++) {
            pargv[i] = p(argv[i]);
        }
        return pargv;
    }

    public JSValueRef JSObjectCallAsFunction(JSContextRef context, JSObjectRef jsObject,
                            JSObjectRef thisObject, JSValueRef argv[]) {
        long[] pargv = convertToLong(argv);
        return new JSValueRef(context, NativeJSObjectCallAsFunction(p(context), p(jsObject), p(thisObject), pargv.length, pargv));
    }

    public void JSObjectSetProperty(JSContextRef context, JSObjectRef jsObject,
                                    String propertyName, JSValueRef value, int attributes) {
        NativeJSObjectSetProperty(p(context), p(jsObject), propertyName, p(value), attributes);
    }

    public JSValueRef JSObjectGetProperty(JSContextRef context, JSObjectRef jsObject,
                                            String propertyName) {
        return new JSValueRef(context, NativeJSObjectGetProperty(p(context), p(jsObject), propertyName));
    }
    
    public void JSClassRelease(JSClassRef jsClass) {
        NativeJSClassRelease(p(jsClass));
    }
    public JSClassRef JSClassRetain(JSClassRef jsClass) {
        return new JSClassRef(NativeJSClassRetain(p(jsClass)));
    }

    public JSPropertyNameArrayRef JSObjectCopyPropertyNames(JSContextRef context, JSObjectRef jsObject) {
        return new JSPropertyNameArrayRef(NativeJSObjectCopyPropertyNames(p(context), p(jsObject)));
    }

    public boolean JSObjectDeleteProperty(JSContextRef context, JSObjectRef jsObject, String propertyName) {
        return NativeJSObjectDeleteProperty(p(context), p(jsObject), propertyName);
    }

    public JSValueRef JSObjectGetPropertyAtIndex(JSContextRef context, JSObjectRef jsObject, int propertyIndex) {
        return new JSValueRef(NativeJSObjectGetPropertyAtIndex(p(context), p(jsObject), propertyIndex));
    }

    public JSValueRef JSObjectGetPrototype(JSContextRef context, JSObjectRef jsObject) {
        return new JSValueRef(NativeJSObjectGetPrototype(p(context), p(jsObject)));
    }

    public boolean JSObjectHasProperty(JSContextRef context, JSObjectRef jsObject, String propertyName) {
        return NativeJSObjectHasProperty(p(context), p(jsObject), propertyName);
    }

    public boolean JSObjectIsConstructor(JSContextRef context, JSObjectRef jsObject) {
        return NativeJSObjectIsConstructor(p(context), p(jsObject));
    }

    public boolean JSObjectIsFunction(JSContextRef context, JSObjectRef jsObject) {
        return NativeJSObjectIsFunction(p(context), p(jsObject));
    }

    public JSObjectRef JSObjectMakeArray(JSContextRef context, JSValueRef argv[]) {
        long[] pargv = convertToLong(argv);
        return new JSObjectRef(NativeJSObjectMakeArray(p(context), pargv.length, pargv));
    }

    public JSObjectRef JSObjectMakeDate(JSContextRef context, JSValueRef argv[]) {
        long[] pargv = convertToLong(argv);
        return new JSObjectRef(NativeJSObjectMakeDate(p(context), pargv.length, pargv));
    }
    public JSObjectRef JSObjectMakeError(JSContextRef context, JSValueRef argv[]) {
        long[] pargv = convertToLong(argv);
        return new JSObjectRef(NativeJSObjectMakeError(p(context), pargv.length, pargv));
    }
    public JSObjectRef JSObjectMakeRegExp(JSContextRef context, JSValueRef argv[]) {
        long[] pargv = convertToLong(argv);
        return new JSObjectRef(NativeJSObjectMakeRegExp(p(context), pargv.length, pargv));
    }
    public JSObjectRef JSObjectMakeFunction(JSContextRef context, String name, int paramCount,
                                            String paramNames[], String body, String sourceURL,
                                            int line) {
        return new JSObjectRef(NativeJSObjectMakeFunction(p(context), name, paramCount,
                                            paramNames, body, sourceURL, line));
    }

    public void JSObjectSetPropertyAtIndex(JSContextRef context, JSObjectRef jsObject,
                                            int propertyIndex, JSValueRef value) {
        NativeJSObjectSetPropertyAtIndex(p(context), p(jsObject), propertyIndex, p(value));
    }

    public void JSObjectSetPrototype(JSContextRef context, JSObjectRef jsObject, JSValueRef value) {
        NativeJSObjectSetPrototype(p(context), p(jsObject), p(value));
    }

    public void JSPropertyNameAccumulatorAddName(JSPropertyNameAccumulatorRef accumulator, String propertyName) {
        NativeJSPropertyNameAccumulatorAddName(p(accumulator), propertyName);
    }

    public int JSPropertyNameArrayGetCount(JSPropertyNameArrayRef names) {
        return NativeJSPropertyNameArrayGetCount(p(names));
    }

    public String JSPropertyNameArrayGetNameAtIndex(JSPropertyNameArrayRef names, int index) {
        return NativeJSPropertyNameArrayGetNameAtIndex(p(names), index);
    }

    public void JSPropertyNameArrayRelease(JSPropertyNameArrayRef names) {
        NativeJSPropertyNameArrayRelease(p(names));
    }

    public JSPropertyNameArrayRef JSPropertyNameArrayRetain(JSPropertyNameArrayRef names) {
        return new JSPropertyNameArrayRef(NativeJSPropertyNameArrayRetain(p(names)));
    }

    public Object JSObjectGetPrivate(JSObjectRef jsObject) {
        return NativeJSObjectGetPrivate(p(jsObject));
    }

    public boolean JSObjectSetPrivate(JSObjectRef jsObject, Object object) {
        return NativeJSObjectSetPrivate(p(jsObject), object);
    }

    public JSObjectRef JSObjectMake(JSContextRef context, JSClassRef jsClass) {
        return JSObjectMake(context, jsClass, null);
    }

    public JSObjectRef JSObjectMake(JSContextRef context, JSClassRef jsClass, Object object) {
        return new JSObjectRef(NativeJSObjectMake(p(context), p(jsClass), jsClass.getDefinition(), object));
    }

    /**
     * Convenience method for creating a JavaScript constructor
     */
    public JSObjectRef JSObjectMakeConstructor(JSContextRef context, JSClassRef jsClassToCopy,
                                               JSObjectCallAsConstructorCallback callback) {
        JSClassDefinition definition;
        if (jsClassToCopy == null) {
            definition = new JSClassDefinition();
        } else {
            definition = jsClassToCopy.getDefinition().copy();
        }
        definition.callAsConstructor = callback;
        JSClassRef jsClass = JSClassCreate(definition);
        JSObjectRef object = JSObjectMake(context, jsClass);
        JSClassRelease(jsClass);
        return object;
    }

    /**
     * Convenience method for creating a JavaScript function with a given callback as its implementation
     */
    public JSObjectRef JSObjectMakeFunctionWithCallback(JSContextRef context, String name,
                                                        JSObjectCallAsFunctionCallback callback) {
        JSClassDefinition definition = new JSClassDefinition();
        definition.className = name;
        definition.callAsFunction = callback;
        JSClassRef jsClass = JSClassCreate(definition);
        JSObjectRef object = JSObjectMake(context, jsClass);
        JSClassRelease(jsClass);
        return object;
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
    public native long NativeJSEvaluateScript(long jsContextRef, String script);
    public native long NativeJSCheckScriptSyntax(long jsContextRef, String script);
    public native long NativeJSGarbageCollect(long jsContextRef);
    public native void NativeJSValueProtect(long jsContextRef, long jsValueRef);
    public native void NativeJSValueUnprotect(long jsContextRef, long jsValueRef);
    public native String NativeJSValueCreateJSONString(long jsContextRef, long jsValueRef, int indent);
    public native long NativeJSValueMakeNull(long jsContextRef);
    public native long NativeJSValueMakeUndefined(long jsContextRef);
    public native long NativeJSValueMakeBoolean(long jsContextRef, boolean value);
    public native long NativeJSValueMakeNumber(long jsContextRef, double number);
    public native long NativeJSValueMakeString(long jsContextRef, String string);
    public native long NativeJSValueMakeFromJSONString(long jsContextRef, String string);
    public native boolean NativeJSValueIsObjectOfClass(long jsContextRef, long jsValueRef, long jsClassRef);
    public native boolean NativeJSValueIsInstanceOfConstructor(long jsContextRef, long jsValueRef, long jsObjectRef);
    public native boolean NativeJSValueIsUndefined(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsNull(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsNumber(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsBoolean(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsString(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsObject(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsEqual(long jsContextRef, long jsValueRefA, long jsValueRefB);
    public native boolean NativeJSValueIsStrictEqual(long jsContextRef, long jsValueRefA, long jsValueRefB);
    public native boolean NativeJSValueToBoolean(long jsContextRef, long jsValueRef);
    public native double NativeJSValueToNumber(long jsContextRef, long jsValueRef);
    public native long NativeJSValueToObject(long jsContextRef, long jsValueRef);
    public native String NativeJSValueToStringCopy(long jsContextRef, long jsValueRef);

    public native long NativeJSClassCreate(
        int version, int attributes, String className, long parentClass,
        String[] staticValueNames, int[] staticValueAttributes,
        String[] staticFunctionNames, int[] staticFunctionAttributes,
        boolean initialize, boolean finalize, boolean hasProperty, boolean getProperty,
        boolean setProperty, boolean deleteProperty, boolean getPropertyNames, boolean callAsFunction,
        boolean callAsConstructor, boolean hasInstance, boolean convertToType);

    public native void NativeJSClassRelease(long jsClassRef);
    public native long NativeJSClassRetain(long jsClassRef);
    public native long NativeJSObjectCallAsConstructor(long jsContextRef, long jsObjectRef, int argc, long argv[]);
    public native long NativeJSObjectCallAsFunction(long jsContextRef, long jsObjectRef, long thisObjectRef, int argc, long argv[]);
    public native long NativeJSObjectCopyPropertyNames(long jsContextRef, long jsObjectRef);
    public native boolean NativeJSObjectDeleteProperty(long jsContextRef, long jsObjectRef, String propertyName);
    public native Object NativeJSObjectGetPrivate(long jsObjectRef);
    public native boolean NativeJSObjectSetPrivate(long jsObjectRef, Object object);
    public native long NativeJSObjectGetProperty(long jsContextRef, long jsObjectRef, String propertyName);
    public native long NativeJSObjectGetPropertyAtIndex(long jsContextRef, long jsObjectRef, int propertyIndex);
    public native long NativeJSObjectGetPrototype(long jsContextRef, long jsObjectRef);
    public native boolean NativeJSObjectHasProperty(long jsContextRef, long jsObjectRef, String propertyName);
    public native boolean NativeJSObjectIsConstructor(long jsContextRef, long jsObjectRef);
    public native boolean NativeJSObjectIsFunction(long jsContextRef, long jsObjectRef);
    public native long NativeJSObjectMake(long jsContextRef, long jsClassRef, JSClassDefinition definition, Object object);
    public native long NativeJSObjectMakeArray(long jsContextRef, int argc, long argv[]);
    public native long NativeJSObjectMakeDate(long jsContextRef, int argc, long argv[]);
    public native long NativeJSObjectMakeError(long jsContextRef, int argc, long argv[]);
    public native long NativeJSObjectMakeFunction(long jsContextRef, String name, int paramCount, String paramNames[], String body, String sourceURL, int line);
    public native long NativeJSObjectMakeRegExp(long jsContextRef, int argc, long argv[]);
    public native void NativeJSObjectSetProperty(long jsContextRef, long jsObjectRef, String propertyName, long jsValueRef, int attributes);
    public native void NativeJSObjectSetPropertyAtIndex(long jsContextRef, long jsObjectRef, int propertyIndex, long jsValueRef);
    public native void NativeJSObjectSetPrototype(long jsContextRef, long jsObjectRef, long jsValueRef);
    public native void NativeJSPropertyNameAccumulatorAddName(long accumulator, String propertyName);
    public native int NativeJSPropertyNameArrayGetCount(long jsPropertyNameArrayRef);
    public native String NativeJSPropertyNameArrayGetNameAtIndex(long jsPropertyNameArrayRef, int index);
    public native void NativeJSPropertyNameArrayRelease(long jsPropertyNameArrayRef);
    public native long NativeJSPropertyNameArrayRetain(long jsPropertyNameArrayRef);

}
