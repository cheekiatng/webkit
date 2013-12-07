package com.appcelerator.javascriptcore.opaquetypes;

import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectSetPropertyCallback;

public class JSStaticValue {
    /**
     * A null-terminated UTF8 string containing the property's name.
     */
    public String name;

    /**
     * A JSObjectGetPropertyCallback to invoke when getting the property's
     * value.
     */

    public JSObjectGetPropertyCallback getProperty;

    /**
     * A JSObjectSetPropertyCallback to invoke when setting the property's
     * value. May be NULL if the ReadOnly attribute is set.
     */
    public JSObjectSetPropertyCallback setProperty;

    /**
     * A logically ORed set of JSPropertyAttributes to give to the property.
     */
    public int attributes;
}