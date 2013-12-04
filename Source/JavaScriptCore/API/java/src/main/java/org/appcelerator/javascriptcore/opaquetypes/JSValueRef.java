package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;

public class JSValueRef extends PointerType {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();
    private JSContextRef context;

    public JSValueRef(JSContextRef context, long pointer) {
        super(pointer);
        this.context = context;
    }

    public double toDouble() {
        JSValueRef exception = jsc.JSValueMakeNull(context);
        double value = jsc.JSValueToNumber(context, this, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
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
        JSValueRef exception = jsc.JSValueMakeNull(context);
        JSObjectRef value = jsc.JSValueToObject(context, this, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
        }
        return value;
    }

    public String toString() {
        JSValueRef exception = jsc.JSValueMakeNull(context);
        String value = jsc.JSValueToStringCopy(context, this, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
        }
        return value;
    }

    public String toJSON(int indent) {
        JSValueRef exception = jsc.JSValueMakeNull(context);
        String value = jsc.JSValueCreateJSONString(context, this, indent, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
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
        JSValueRef exception = jsc.JSValueMakeNull(context);
        boolean result = jsc.JSValueIsEqual(context, this, other, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
        }
        return result;
    }

    public boolean isStrictEqual(JSValueRef other) {
        return jsc.JSValueIsStrictEqual(context, this, other);
    }

    public boolean isObjectOfClass(JSClassRef jsClass) {
        return jsc.JSValueIsObjectOfClass(context, this, jsClass);
    }

    public boolean isInstanceOfConstructor(JSObjectRef object) {
        JSValueRef exception = jsc.JSValueMakeNull(context);
        boolean result = jsc.JSValueIsInstanceOfConstructor(context, this, object, exception);
        if (!exception.isNull()) {
            throw new JavaScriptException(exception.toString(), exception);
        }
        return result;
    }

    public void protect() {
        jsc.JSValueProtect(context, this);
    }

    public void unprotect() {
        jsc.JSValueUnprotect(context, this);
    }
}