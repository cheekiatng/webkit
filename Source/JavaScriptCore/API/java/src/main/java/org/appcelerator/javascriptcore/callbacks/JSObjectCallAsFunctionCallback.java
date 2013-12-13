package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueArrayRef;

/**
 * The callback invoked when an object is called as a function. If you named
 * your function CallAsFunction, you would declare it like this:
 * 
 * {@code JSValueRef CallAsFunction(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);}
 * 
 * If your callback were invoked by the JavaScript expression
 * 'myObject.myFunction()', function would be set to myFunction, and thisObject
 * would be set to myObject. If this callback is NULL, calling your object as a
 * function will throw an exception.
 * 
 * @param ctx
 *            The execution context to use.
 * @param function
 *            A JSObject that is the function being called.
 * @param thisObject
 *            A JSObject that is the 'this' variable in the function's scope.
 * @param argumentCount
 *            An integer count of the number of arguments in arguments.
 * @param arguments
 *            A JSValue array of the arguments passed to the function.
 * @param exception
 *            A pointer to a JSValueRef in which to return an exception, if any.
 * @return A JSValue that is the function's return value.
 */
public interface JSObjectCallAsFunctionCallback {
    JSValueRef apply(JSContextRef ctx, JSObjectRef function,
            JSObjectRef thisObject, int argumentCount,
            JSValueArrayRef arguments, JSValueRef exception);
}