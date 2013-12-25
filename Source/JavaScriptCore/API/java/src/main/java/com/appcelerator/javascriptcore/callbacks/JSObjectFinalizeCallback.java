package com.appcelerator.javascriptcore.callbacks;

import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;

/**
 * The callback invoked when an object is finalized (prepared for garbage
 * collection). An object may be finalized on any thread.
 * 
 * The finalize callback is called on the most derived class first, and the
 * least derived class (the parent class) last. You must not call any function
 * that may cause a garbage collection or an allocation of a garbage collected
 * object from within a JSObjectFinalizeCallback. This includes all functions
 * that have a JSContextRef parameter.
 * 
 * @param object
 *            The JSObject being finalized.
 */
public interface JSObjectFinalizeCallback {
    void finalize(JSObjectRef object);
}