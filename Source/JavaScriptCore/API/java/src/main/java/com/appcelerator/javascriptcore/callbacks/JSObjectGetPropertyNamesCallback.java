package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSPropertyNameAccumulatorRef;

/**
 * The callback invoked when collecting the names of an object's properties.
 * 
 * Property name accumulators are used by JSObjectCopyPropertyNames and
 * JavaScript for...in loops.
 * 
 * Use JSPropertyNameAccumulatorAddName to add property names to accumulator. A
 * class's getPropertyNames callback only needs to provide the names of
 * properties that the class vends through a custom getProperty or setProperty
 * callback. Other properties, including statically declared properties,
 * properties vended by other classes, and properties belonging to object's
 * prototype, are added independently.
 * 
 * @param ctx
 *            The execution context to use.
 * @param object
 *            The JSObject whose property names are being collected.
 * @param accumulator
 *            A JavaScript property name accumulator in which to accumulate the
 *            names of object's properties.
 */
public interface JSObjectGetPropertyNamesCallback {
    void getPropertyNames(JSContextRef ctx, JSObjectRef object,
            JSPropertyNameAccumulatorRef propertyNames);
}