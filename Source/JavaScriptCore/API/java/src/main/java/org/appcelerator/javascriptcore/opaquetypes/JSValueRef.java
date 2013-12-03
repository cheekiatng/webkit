package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JSExceptionHandler;

public class JSValueRef extends PointerType {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();
    private JSContextRef context;
    private JSExceptionHandler exceptionHandler;

    public JSValueRef(JSContextRef context, long pointer) {
        super(pointer);
        this.context = context;
    }

    public double toDouble() {
        JSValueRef exception = jsc.JSValueMakeNull(context);
        double value = jsc.JSValueToNumber(context, this, exception);
        if (exceptionHandler != null && !exception.isNull()) {
            exceptionHandler.handleException(context, exception);           
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

    public String toString() {
        JSValueRef exception = jsc.JSValueMakeNull(context);
        String value = jsc.JSValueToStringCopy(context, this, exception);
        if (exceptionHandler != null && !exception.isNull()) {
            exceptionHandler.handleException(context, exception);           
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
        if (exceptionHandler != null && !exception.isNull()) {
            exceptionHandler.handleException(context, exception);           
        }
        return result;
    }

    public boolean isStrictEqual(JSValueRef other) {
        return jsc.JSValueIsStrictEqual(context, this, other);
    }

    public void setExceptionHandler(JSExceptionHandler handler) {
        this.exceptionHandler = handler;
    }
}