package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;

/**
 * The callback invoked when determining whether an object has a property. If
 * you named your function HasProperty, you would declare it like this:
 * 
 * {@code boolean HasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName);}
 * 
 * If this function returns false, the hasProperty request forwards to object's
 * statically declared properties, then its parent class chain (which includes
 * the default object class), then its prototype chain. This callback enables
 * optimization in cases where only a property's existence needs to be known,
 * not its value, and computing its value would be expensive.
 * 
 * If this callback is NULL, the getProperty callback will be used to service
 * hasProperty requests.
 * 
 * @param ctx
 *            The execution context to use.
 * @param object
 *            The JSObject to search for the property.
 * @param propertyName
 *            A JSString containing the name of the property look up.
 * @return true if object has the property, otherwise false.
 */
public interface JSObjectHasPropertyCallback {
    boolean apply(JSContextRef ctx, JSObjectRef object, String propertyName);
}