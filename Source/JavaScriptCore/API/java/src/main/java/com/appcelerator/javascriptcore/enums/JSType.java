package com.appcelerator.javascriptcore.enums;

import com.appcelerator.javascriptcore.JavaScriptException;
/**
 * A constant identifying the type of a JSValue.
 * 
 * /System/Library/Frameworks/JavaScriptCore.framework/Headers/JSValueRef.h
 */
public enum JSType {
    // @formatter:off
    // @formatter:on
    /**
     * The unique undefined value.
     */
    Undefined(0), //

    /**
     * The unique null value.
     */
    Null(1),

    /**
     * A primitive boolean value, one of true or false.
     */
    Boolean(2),

    /**
     * A primitive number value.
     */
    Number(3),

    /**
     * A primitive string value.
     */
    String(4),

    /**
     * An object value (meaning that this JSValueRef is a JSObjectRef).
     */
    Object(5);

    private final int value;

    private JSType(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public static JSType request(int value) {
        switch(value) {
            case 0: return Undefined;
            case 1: return Null;
            case 2: return Boolean;
            case 3: return Number;
            case 4: return String;
            case 5: return Object;
            default: throw new JavaScriptException(java.lang.String.format("Unsupported JSType: %d", value));
        }
    }
}