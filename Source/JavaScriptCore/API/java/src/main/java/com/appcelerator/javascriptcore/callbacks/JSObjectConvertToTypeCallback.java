package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.enums.JSType;
import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;

/**
 * The callback invoked when converting an object to a particular JavaScript
 * type. If you named your function ConvertToType, you would declare it like
 * this:
 * 
 * {@code JSValueRef ConvertToType(JSContextRef ctx, JSObjectRef object, JSType type, JSValueRef* exception);}
 * 
 * If this function returns false, the conversion request forwards to object's
 * parent class chain (which includes the default object class). This function
 * is only invoked when converting an object to number or string. An object
 * converted to boolean is 'true.' An object converted to object is itself.
 * 
 * @param ctx
 *            The execution context to use.
 * @param object
 *            The JSObject to convert.
 * @param type
 *            A JSType specifying the JavaScript type to convert to.
 * @param exception
 *            A pointer to a JSValueRef in which to return an exception, if any.
 * @return The objects's converted value, or NULL if the object was not
 *         converted.
 */
public interface JSObjectConvertToTypeCallback {
    JSValueRef convertToType(JSContextRef ctx, JSObjectRef object, JSType type,
            Pointer exception);
}