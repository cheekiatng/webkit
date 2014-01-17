package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;

public class JSValueRef extends PointerType {

    protected JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();
    protected JSContextRef context = null;

    public JSValueRef(JSContextRef context, long pointer) {
        super(pointer);
        this.context = context;
    }

    public JSContextRef getContext() {
        return this.context;
    }

    public double toDouble() {
        checkContext();
        return jsc.JSValueToNumber(context, this, null);
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
        checkContext();
        return jsc.JSValueToBoolean(context, this);
    }

    public JSObjectRef toObject() {
        checkContext();
        return jsc.JSValueToObject(context, this, null);
    }

    public String toString() {
        checkContext();
        return jsc.JSValueToStringCopy(context, this, null);
    }

    public String toJSON() {
        return toJSON(0);
    }

    public String toJSON(int indent) {
        checkContext();
        return jsc.JSValueCreateJSONString(context, this, indent, null);
    }

    /*
     * Forcibly cast JSValueRef to JSObjectRef.
     * This idiom is not recommended but often used by C/C++.
     */
    public JSObjectRef castToObject() {
        return new JSObjectRef(context, pointer());
    }

    public boolean isUndefined() {
        checkContext();
        return jsc.JSValueIsUndefined(context, this);
    }

    public boolean isNull() {
        checkContext();
        return jsc.JSValueIsNull(context, this);
    }

    public boolean isNumber() {
        checkContext();
        return jsc.JSValueIsNumber(context, this);
    }
    
    public boolean isBoolean() {
        checkContext();
        return jsc.JSValueIsBoolean(context, this);
    }

    public boolean isString() {
        checkContext();
        return jsc.JSValueIsString(context, this);
    }

    public boolean isObject() {
        checkContext();
        return jsc.JSValueIsObject(context, this);
    }

    public boolean isEqual(JSValueRef other) {
        checkContext();
        return jsc.JSValueIsEqual(context, this, other, null);
    }

    public boolean isStrictEqual(JSValueRef other) {
        checkContext();
        return jsc.JSValueIsStrictEqual(context, this, other);
    }

    public boolean isObjectOfClass(JSClassRef jsClass) {
        checkContext();
        return jsc.JSValueIsObjectOfClass(context, this, jsClass);
    }

    public boolean isInstanceOfConstructor(JSObjectRef object) {
        checkContext();
        return jsc.JSValueIsInstanceOfConstructor(context, this, object, null);
    }

    public void protect() {
        checkContext();
        jsc.JSValueProtect(context, this);
    }

    public void unprotect() {
        checkContext();
        jsc.JSValueUnprotect(context, this);
    }

    public void UpdateJSValueRef(long jsContextRef, long jsValueRef) {
        this.context = new JSContextRef(jsContextRef);
        this.pointer = new Pointer(jsValueRef);
    }

    private void checkContext() {
        if (context == null) {
            throw new JavaScriptException(String.format("No context found at %d", p()));
        }
    }

    public static JSValueRef Null() {
        return new JSValueRef(null, 0);
    }
}