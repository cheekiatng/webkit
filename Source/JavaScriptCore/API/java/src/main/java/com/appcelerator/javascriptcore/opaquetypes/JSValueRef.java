package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;

public class JSValueRef extends PointerType {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();
    private JSContextRef context;

    public JSValueRef(long pointer) {
        super(pointer);
    }

    public JSValueRef(JSContextRef context, long pointer) {
        super(pointer);
        this.context = context;
    }

    public double toDouble() {
        JSValueRef exception = new JSValueRef(0);
        double value = jsc.JSValueToNumber(context, this, exception);
        if (!jsc.JSValueIsNull(context, exception)) {
            throw new JavaScriptException("Unable to convert to double");
        }
        return value;
    }
    
    public double toNumber() {
        return toDouble();
    }

    public float toFloat() {
        return (float)toDouble();
    }

    public int toInt() {
        return (int)toDouble();
    }

    public long toLong() {
        return (long)toDouble();
    }

    public boolean toBoolean() {
        return jsc.JSValueToBoolean(context, this);
    }

    public JSObjectRef toObject() {
        JSValueRef exception = new JSValueRef(0);
        JSObjectRef value = jsc.JSValueToObject(context, this, exception);
        if (!jsc.JSValueIsNull(context, exception)) {
            throw new JavaScriptException("Unable to convert to object");
        }
        return value;
    }

    public String toString() {
        JSValueRef exception = new JSValueRef(0);
        String value = jsc.JSValueToStringCopy(context, this, exception);
        if (!jsc.JSValueIsNull(context, exception)) {
            throw new JavaScriptException("Unable to convert to string");
        }
        return value;
    }

    public String toJSON() {
        return toJSON(0);
    }

    public String toJSON(int indent) {
        JSValueRef exception = new JSValueRef(0);
        String value = jsc.JSValueCreateJSONString(context, this, indent, exception);
        if (!jsc.JSValueIsNull(context, exception)) {
            throw new JavaScriptException("Unable to convert to JSON");
        }
        return value;
    }

    public boolean isUndefined() {
        return jsc.JSValueIsUndefined(context, this);
    }

    public boolean isNull() {
        return jsc.JSValueIsNull(context, this);
    }

    public boolean isNumber() {
        return jsc.JSValueIsNumber(context, this);
    }
    
    public boolean isBoolean() {
        return jsc.JSValueIsBoolean(context, this);
    }

    public boolean isString() {
        return jsc.JSValueIsString(context, this);
    }

    public boolean isObject() {
        return jsc.JSValueIsObject(context, this);
    }

    public boolean isEqual(JSValueRef other) {
        JSValueRef exception = new JSValueRef(0);
        boolean value = jsc.JSValueIsEqual(context, this, other, exception);
        if (!jsc.JSValueIsNull(context, exception)) {
            throw new JavaScriptException("Exception while isEqual");
        }
        return value;
    }

    public boolean isStrictEqual(JSValueRef other) {
        return jsc.JSValueIsStrictEqual(context, this, other);
    }

    public boolean isObjectOfClass(JSClassRef jsClass) {
        return jsc.JSValueIsObjectOfClass(context, this, jsClass);
    }

    public boolean isInstanceOfConstructor(JSObjectRef object) {
        JSValueRef exception = new JSValueRef(0);
        boolean value = jsc.JSValueIsInstanceOfConstructor(context, this, object, exception);
        if (!jsc.JSValueIsNull(context, exception)) {
            throw new JavaScriptException("Exception while isInstanceOfConstructor");
        }
        return value;
    }

    public void protect() {
        jsc.JSValueProtect(context, this);
    }

    public void unprotect() {
        jsc.JSValueUnprotect(context, this);
    }

    public void UpdateJSValueRef(long jsContextRef, long jsValueRef) {
        this.context = new JSContextRef(jsContextRef);
        this.pointer = new Pointer(jsValueRef);
    }
}