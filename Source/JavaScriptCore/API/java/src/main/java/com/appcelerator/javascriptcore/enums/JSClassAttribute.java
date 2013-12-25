package com.appcelerator.javascriptcore.enums;

/**
 * A set of JSClassAttributes. Combine multiple attributes by logically ORing
 * them together.
 */
public enum JSClassAttribute {
    /**
     * Specifies that a class has no special attributes.
     */
    None(0),

    /**
     * Specifies that a class should not automatically generate a shared
     * prototype for its instance objects. Use NoAutomaticPrototype in
     * combination with JSObjectSetPrototype to manage prototypes manually.
     */
    NoAutomaticPrototype(1 << 1);

    private final int value;

    private JSClassAttribute(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }
}