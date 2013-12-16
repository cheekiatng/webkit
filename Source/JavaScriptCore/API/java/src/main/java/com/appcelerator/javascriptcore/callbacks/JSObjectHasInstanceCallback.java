package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;

/**
 * The callback invoked when an object is used as the target of an 'instanceof'
 * expression. If you named your function HasInstance, you would declare it like
 * this:
 * 
 * {@code boolean HasInstance(JSContextRef ctx, JSObjectRef constructor, JSValueRef possibleInstance, JSValueRef* exception);}
 * 
 * If your callback were invoked by the JavaScript expression 'someValue
 * instanceof myObject', constructor would be set to myObject and
 * possibleInstance would be set to someValue. If this callback is NULL,
 * 'instanceof' expressions that target your object will return false. Standard
 * JavaScript practice calls for objects that implement the callAsConstructor
 * callback to implement the hasInstance callback as well.
 * 
 * @param ctx
 *            The execution context to use.
 * @param constructor
 *            The JSObject that is the target of the 'instanceof' expression.
 * @param possibleInstance
 *            The JSValue being tested to determine if it is an instance of
 *            constructor.
 * @param exception
 *            A pointer to a JSValueRef in which to return an exception, if any.
 * @return true if possibleInstance is an instance of constructor, otherwise
 *         false.
 */
public interface JSObjectHasInstanceCallback {
    boolean apply(JSContextRef ctx, JSObjectRef constructor,
            JSValueRef possibleInstance, JSValueRef exception);
}