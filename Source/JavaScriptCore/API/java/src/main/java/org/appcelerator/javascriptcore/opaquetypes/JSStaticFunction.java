package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsFunctionCallback;

public class JSStaticFunction {
    /**
     * A null-terminated UTF8 string containing the property's name.
     */
    public String name;

    /**
     * A JSObjectCallAsFunctionCallback to invoke when the property is called as
     * a function.
     */
    public JSObjectCallAsFunctionCallback callAsFunction;

    /**
     * A logically ORed set of JSPropertyAttributes to give to the property.
     */
    public int attributes;
}