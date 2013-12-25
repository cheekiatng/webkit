package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;

/**
 * The callback invoked when getting a property's value.
 * 
 * If this function returns NULL, the get request forwards to object's
 * statically declared properties, then its parent class chain (which includes
 * the default object class), then its prototype chain.
 * 
 * @param ctx
 *            The execution context to use.
 * @param object
 *            The JSObject to search for the property.
 * @param propertyName
 *            A String containing the name of the property to get.
 * @param exception
 *            A pointer to a JSValueRef in which to return an exception, if any.
 * @return The property's value if object has the property, otherwise NULL.
 */
public interface JSObjectGetPropertyCallback {
    JSValueRef getProperty(JSContextRef ctx, JSObjectRef object,
            String propertyName, Pointer exception);
}