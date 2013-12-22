package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;

/**
 * The callback invoked when setting a property's value. If you named your
 * function SetProperty, you would declare it like this:
 * 
 * {@code boolean SetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception);}
 * 
 * If this function returns false, the set request forwards to object's
 * statically declared properties, then its parent class chain (which includes
 * the default object class).
 * 
 * @param ctx
 *            The execution context to use.
 * @param object
 *            The JSObject on which to set the property's value.
 * @param propertyName
 *            A String containing the name of the property to set.
 * @param value
 *            A JSValue to use as the property's value.
 * @param exception
 *            A pointer to a JSValueRef in which to return an exception, if any.
 * @return true if the property was set, otherwise false.
 */
public interface JSObjectSetPropertyCallback {
    boolean setProperty(JSContextRef ctx, JSObjectRef object,
            String propertyName, JSValueRef value, Pointer exception);
}