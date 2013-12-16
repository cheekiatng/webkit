package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;

/**
 * The callback invoked when an object is first created. If you named your
 * function Initialize, you would declare it like this:
 * 
 * {@code void Initialize(JSContextRef ctx, JSObjectRef object);}
 * 
 * Unlike the other object callbacks, the initialize callback is called on the
 * least derived class (the parent class) first, and the most derived class
 * last.
 * 
 * @param ctx
 *            The execution context to use.
 * @param object
 *            The JSObject being created.
 */
public interface JSObjectInitializeCallback {
    void apply(JSContextRef ctx, JSObjectRef object);
}