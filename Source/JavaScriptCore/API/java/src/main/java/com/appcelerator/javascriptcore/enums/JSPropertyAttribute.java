package com.appcelerator.javascriptcore.enums;

/**
 * A set of JSPropertyAttributes. Combine multiple attributes by logically ORing
 * them together.
 */
public enum JSPropertyAttribute {
    /**
     * Specifies that a property has no special attributes.
     */
    None(0),

    /**
     * Specifies that a property is read-only.
     */
    ReadOnly(1 << 1),

    /**
     * Specifies that a property should not be enumerated by
     * JSPropertyEnumerators and JavaScript for...in loops.
     */
    DontEnum(1 << 2),

    /**
     * Specifies that the delete operation should fail on a property.
     */
    DontDelete(1 << 3);

    private int value;

    private JSPropertyAttribute(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public JSPropertyAttribute add(JSPropertyAttribute attr) {
        this.value |= attr.getValue();
        return this;
    }
}