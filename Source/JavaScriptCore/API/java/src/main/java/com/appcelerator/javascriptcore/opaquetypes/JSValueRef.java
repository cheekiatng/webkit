package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;

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
        return jsc.JSValueToNumber(context, this);
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
        return jsc.JSValueToObject(context, this);
    }

    public String toString() {
        return jsc.JSValueToStringCopy(context, this);
    }

    public String toJSON(int indent) {
        return jsc.JSValueCreateJSONString(context, this, indent);
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
        return jsc.JSValueIsEqual(context, this, other);
    }

    public boolean isStrictEqual(JSValueRef other) {
        return jsc.JSValueIsStrictEqual(context, this, other);
    }

    public boolean isObjectOfClass(JSClassRef jsClass) {
        return jsc.JSValueIsObjectOfClass(context, this, jsClass);
    }

    public boolean isInstanceOfConstructor(JSObjectRef object) {
        return jsc.JSValueIsInstanceOfConstructor(context, this, object);
    }

    public void protect() {
        jsc.JSValueProtect(context, this);
    }

    public void unprotect() {
        jsc.JSValueUnprotect(context, this);
    }
}