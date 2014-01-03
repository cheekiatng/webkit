package com.appcelerator.javascriptcore.enums;

/**
 * A set of JSPropertyAttributes. Combine multiple attributes by logically ORing
 * them together.
 */
public class JSPropertyAttribute {
    /**
     * Specifies that a property has no special attributes.
     */
    public static final JSPropertyAttribute None = new JSPropertyAttribute(0);

    /**
     * Specifies that a property is read-only.
     */
    public static final JSPropertyAttribute ReadOnly = new JSPropertyAttribute(1 << 1);

    /**
     * Specifies that a property should not be enumerated by
     * JSPropertyEnumerators and JavaScript for...in loops.
     */
    public static final JSPropertyAttribute DontEnum = new JSPropertyAttribute(1 << 2);

    /**
     * Specifies that the delete operation should fail on a property.
     */
    public static final JSPropertyAttribute DontDelete = new JSPropertyAttribute(1 << 3);

    private int value;

    private JSPropertyAttribute(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public JSPropertyAttribute add(JSPropertyAttribute attr) {
        return new JSPropertyAttribute(this.value | attr.getValue());
    }
}