package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.enums.JSClassAttribute;
import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsConstructorCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsFunctionCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectConvertToTypeCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectDeletePropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectFinalizeCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyNamesCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectHasInstanceCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectHasPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectInitializeCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectSetPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyCallback;

public class JSClassDefinition {

    /**
     * The version number of this structure. The current version is 0.
     */
    public int version;
    /**
     * A logically ORed set of JSClassAttributes to give to the class.
     */
    public int attributes;
    /**
     * A null-terminated UTF8 string containing the class's name.
     */
    public String className;
    /**
     * A JSClass to set as the class's parent class. Pass NULL use the default
     * object class.
     */ 
    public JSClassRef parentClass;

    /**
     * A JSStaticValue array containing the class's statically declared value
     * properties. Pass NULL to specify no statically declared value properties.
     * The array must be terminated by a JSStaticValue whose name field is NULL.
     */
    public JSStaticValues staticValues;

    /**
     * A JSStaticFunction array containing the class's statically declared
     * function properties. Pass NULL to specify no statically declared function
     * properties. The array must be terminated by a JSStaticFunction whose name
     * field is NULL.
     */
    public JSStaticFunctions staticFunctions;

    /**
     * The callback invoked when an object is first created. Use this callback
     * to initialize the object.
     */
    public JSObjectInitializeCallback initialize;

   /**
     * The callback invoked when an object is finalized (prepared for garbage
     * collection). Use this callback to release resources allocated for the
     * object, and perform other cleanup.
     */
    public JSObjectFinalizeCallback finalize;

    /**
     * The callback invoked when determining whether an object has a property.
     * If this field is NULL, getProperty is called instead. The hasProperty
     * callback enables optimization in cases where only a property's existence
     * needs to be known, not its value, and computing its value is expensive.
     */
    public JSObjectHasPropertyCallback hasProperty;

    /**
     * The callback invoked when getting a property's value.
     */
    public JSObjectGetPropertyCallback getProperty;

    /**
     * The callback invoked when setting a property's value.
     */
    public JSObjectSetPropertyCallback setProperty;

    /**
     * The callback invoked when deleting a property.
     */
    public JSObjectDeletePropertyCallback deleteProperty;

    /**
     * The callback invoked when collecting the names of an object's properties.
     */
    public JSObjectGetPropertyNamesCallback getPropertyNames;

    /**
     * The callback invoked when an object is called as a function.
     */
    public JSObjectCallAsFunctionCallback callAsFunction;

    /**
     * The callback invoked when an object is used as a constructor in a 'new'
     * expression.
     */
    public JSObjectCallAsConstructorCallback callAsConstructor;

    /**
     * The callback invoked when an object is used as the target of an
     * 'instanceof' expression.
     */
    public JSObjectHasInstanceCallback hasInstance;

    /**
     * The callback invoked when converting an object to a particular JavaScript
     * type.
     */
    public JSObjectConvertToTypeCallback convertToType;

    private static final short LONG     = JavaScriptCoreLibrary.SizeOfLong;
    private static final short INT      = JavaScriptCoreLibrary.SizeOfInt;
    private static final short UNSIGNED = JavaScriptCoreLibrary.SizeOfUnsigned;

    private static long initializeFunction;
    private static long finalizeFunction;
    private static long hasPropertyFunction;
    private static long getPropertyFunction;
    private static long setPropertyFunction;
    private static long deletePropertyFunction;
    private static long getPropertyNamesFunction;
    private static long callAsFunctionFunction;
    private static long callAsConstructorFunction;
    private static long hasInstanceFunction;
    private static long convertToTypeFunction;

    private static int versionIndex = 0;
    private static int attributesIndex;
    private static int classNameIndex; 
    private static int parentClassIndex;
    private static int staticValuesIndex;
    private static int staticFunctionsIndex;
    private static int initializeIndex;
    private static int finalizeIndex;
    private static int hasPropertyIndex;
    private static int getPropertyIndex;
    private static int setPropertyIndex;
    private static int deletePropertyIndex;
    private static int getPropertyNamesIndex;
    private static int callAsFunctionIndex;
    private static int callAsConstructorIndex;
    private static int hasInstanceIndex;
    private static int convertToTypeIndex;
 
    private static final ByteOrder nativeOrder = ByteOrder.nativeOrder();
    private static ByteBuffer bufferTemplate = null;

    private ByteBuffer buffer;

    public JSClassDefinition() {
        constructBufferTemplate();
    }

    public ByteBuffer getStaticValues() {
        if (staticValues == null) {
            return null;
        } else {
            return staticValues.commit();
        }
    }

    public ByteBuffer getStaticFunctions() {
        if (staticFunctions == null) {
            return null;
        } else {
            return staticFunctions.commit();
        }
    }

    public int getStaticFunctionCount() {
        if (staticFunctions == null) {
            return 0;
        } else {
            return staticFunctions.size();
        }
    }

    public void registerStaticFunctions(long[] pointers) {
        if (staticFunctions == null || pointers.length <= 1) return;
        staticFunctions.registerFunctions(pointers);
    }

    public ByteBuffer commit() {
        if (buffer == null) {
            buffer = ByteBuffer.allocateDirect(JavaScriptCoreLibrary.SizeOfJSClassDefinition).order(nativeOrder);

            buffer.putInt(versionIndex, version);
            buffer.putInt(attributesIndex, attributes);

            if (parentClass != null) buffer.putLong(parentClassIndex, parentClass.p());
            if (initialize  != null) buffer.putLong(initializeIndex, initializeFunction);
            if (finalize    != null) buffer.putLong(finalizeIndex, finalizeFunction);
            if (hasProperty != null) buffer.putLong(hasPropertyIndex, hasPropertyFunction);
            if (getProperty != null) buffer.putLong(getPropertyIndex, getPropertyFunction);
            if (setProperty != null) buffer.putLong(setPropertyIndex, setPropertyFunction);
            if (deleteProperty    != null) buffer.putLong(deletePropertyIndex, deletePropertyFunction);
            if (getPropertyNames  != null) buffer.putLong(getPropertyNamesIndex, getPropertyNamesFunction);
            if (callAsFunction    != null) buffer.putLong(callAsFunctionIndex, callAsFunctionFunction);
            if (callAsConstructor != null) buffer.putLong(callAsConstructorIndex, callAsConstructorFunction);
            if (hasInstance   != null) buffer.putLong(hasInstanceIndex, hasInstanceFunction);
            if (convertToType != null) buffer.putLong(convertToTypeIndex, convertToTypeFunction);
        }

        return buffer;

    }

    public void JSObjectInitializeCallback(long ctx, long object) {
        if (initialize != null) {
            initialize.apply(new JSContextRef(ctx), new JSObjectRef(object));
        }
    }

    public void JSObjectFinalizeCallback(long object) {
        if (finalize != null) {
            finalize.apply(new JSObjectRef(object));
        }
    }

    public boolean JSObjectSetPropertyCallback(long ctx, long object, String propertyName, long value, long exception) {
        if (this.setProperty != null) {
            JSContextRef context = new JSContextRef(ctx);
            return setProperty.apply(context, new JSObjectRef(object), propertyName,
                        new JSValueRef(context, value), new JSValueRef(context, exception));
        }
        return false;
    }

    public long JSObjectGetPropertyCallback(long ctx, long object, String propertyName, long exception) {
        if (getProperty != null) {
            JSContextRef context = new JSContextRef(ctx);
            return getProperty.apply(context, new JSObjectRef(object), 
                    propertyName, new JSValueRef(context, exception)).pointer();
        }
        return 0;
    }

    public long JSObjectCallAsFunctionCallback(long ctx, long func, long thisObject, int argc, ByteBuffer argv, long exception) {
        if (staticFunctions != null && staticFunctions.contains(func)) {
            JSObjectCallAsFunctionCallback staticFunction = staticFunctions.getFunction(func);
            JSContextRef context = new JSContextRef(ctx);
            JSValueArrayRef jargv = new JSValueArrayRef(argc, argv);
            return staticFunction.apply(context, new JSObjectRef(func), new JSObjectRef(thisObject),
                                        argc, jargv, new JSValueRef(context, exception)).pointer();
        } else if (callAsFunction != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueArrayRef jargv = new JSValueArrayRef(argc, argv);
            return callAsFunction.apply(context, new JSObjectRef(func), new JSObjectRef(thisObject),
                                        argc, jargv, new JSValueRef(context, exception)).pointer();
        }
        return 0;
    }

    public long JSObjectCallAsConstructorCallback(long ctx, long constructor, int argc, ByteBuffer argv, long exception) {
        if (callAsConstructor != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueArrayRef jargv = new JSValueArrayRef(argc, argv);
            return callAsConstructor.apply(context, new JSObjectRef(constructor),
                                           argc, jargv, new JSValueRef(context, exception)).pointer();
        }
        return 0;
    }

    public long JSObjectConvertToTypeCallback(long ctx, long object, int type, long exception) {
        if (convertToType != null) {
            JSContextRef context = new JSContextRef(ctx);
            return convertToType.apply(context, new JSObjectRef(object), type, new JSValueRef(context, exception)).pointer();
        }
        return 0;
    }
    
    public boolean JSObjectDeletePropertyCallback(long ctx, long object, String name, long exception) {
        if (deleteProperty != null) {
            JSContextRef context = new JSContextRef(ctx);
            return deleteProperty.apply(context, new JSObjectRef(object), name, new JSValueRef(context, exception));
        }
        return false;
    }

    public void JSObjectGetPropertyNamesCallback(long ctx, long object, long propertyNames) {
        if (getPropertyNames != null) {
            getPropertyNames.apply(new JSContextRef(ctx), new JSObjectRef(object), 
                                   new JSPropertyNameAccumulatorRef(propertyNames));
        }
    }

    public boolean JSObjectHasInstanceCallback(long ctx, long constructor, long possibleInstance, long exception) {
        if (hasInstance != null) {
            JSContextRef context = new JSContextRef(ctx);
            return hasInstance.apply(context, new JSObjectRef(constructor),
                                     new JSValueRef(context, possibleInstance), new JSValueRef(context, exception));
        }
        return false;
    }

    public boolean JSObjectHasPropertyCallback(long ctx, long object, String name) {
        if (hasProperty != null) {
            return hasProperty.apply(new JSContextRef(ctx), new JSObjectRef(object), name);
        }
        return false;
    }

    public boolean JSObjectSetStaticValueCallback(long ctx, long object, String propertyName, long value, long exception) {
        if (staticValues != null && staticValues.containsSetter(propertyName)) {
            JSObjectSetPropertyCallback callback = staticValues.getSetPropertyCallback(propertyName);
            if (callback == null) {
                return false;
            }
            JSContextRef context = new JSContextRef(ctx);
            return callback.apply(context, new JSObjectRef(object), propertyName,
                new JSValueRef(context, value), new JSValueRef(context, exception));
        }
        return false;
    }

    public long JSObjectGetStaticValueCallback(long ctx, long object, String propertyName, long exception) {
        if (staticValues != null && staticValues.containsGetter(propertyName)) {
            JSObjectGetPropertyCallback callback = staticValues.getGetPropertyCallback(propertyName);
            if (callback == null) {
                return 0;
            }
            JSContextRef context = new JSContextRef(ctx);
            return callback.apply(context, new JSObjectRef(object),
                        propertyName, new JSValueRef(context, exception)).pointer();
        }
        return 0;
    }

    public JSClassDefinition copy() {
        JSClassDefinition copy = new JSClassDefinition();
        copy.version = this.version;
        copy.attributes = this.attributes;
        copy.className = this.className;
        copy.parentClass = this.parentClass;
        copy.staticValues = this.staticValues;
        copy.staticFunctions = this.staticFunctions;
        copy.initialize = this.initialize;
        copy.finalize = this.finalize;
        copy.hasProperty = this.hasProperty;
        copy.getProperty = this.getProperty;
        copy.setProperty = this.setProperty;
        copy.deleteProperty = this.deleteProperty;
        copy.getPropertyNames = this.getPropertyNames;
        copy.callAsFunction = this.callAsFunction;
        copy.callAsConstructor = this.callAsConstructor;
        copy.hasInstance = this.hasInstance;
        copy.convertToType = this.convertToType;
        return copy;
    }

    private void constructBufferTemplate() {
        if (bufferTemplate == null) {
            bufferTemplate = NativeGetClassDefinitionTemplate().order(nativeOrder);
            attributesIndex   = INT;
            classNameIndex    = attributesIndex + UNSIGNED;
            parentClassIndex  = classNameIndex + LONG;
            staticValuesIndex = parentClassIndex + LONG;
            staticFunctionsIndex = staticValuesIndex + LONG;
            initializeIndex    = staticFunctionsIndex + LONG;
            finalizeIndex      = initializeIndex + LONG;
            hasPropertyIndex   = finalizeIndex + LONG;
            getPropertyIndex   = hasPropertyIndex + LONG;
            setPropertyIndex   = getPropertyIndex + LONG;
            deletePropertyIndex   = setPropertyIndex + LONG;
            getPropertyNamesIndex = deletePropertyIndex + LONG;
            callAsFunctionIndex   = getPropertyNamesIndex + LONG;
            callAsConstructorIndex = callAsFunctionIndex + LONG;
            hasInstanceIndex     = callAsConstructorIndex + LONG;
            convertToTypeIndex   = hasInstanceIndex + LONG;

            initializeFunction  = bufferTemplate.getLong(initializeIndex);
            finalizeFunction    = bufferTemplate.getLong(finalizeIndex);
            hasPropertyFunction = bufferTemplate.getLong(hasPropertyIndex);
            getPropertyFunction = bufferTemplate.getLong(getPropertyIndex);
            setPropertyFunction = bufferTemplate.getLong(setPropertyIndex);
            deletePropertyFunction    = bufferTemplate.getLong(deletePropertyIndex);
            getPropertyNamesFunction  = bufferTemplate.getLong(getPropertyNamesIndex);
            callAsFunctionFunction    = bufferTemplate.getLong(callAsFunctionIndex);
            callAsConstructorFunction = bufferTemplate.getLong(callAsConstructorIndex);
            hasInstanceFunction   = bufferTemplate.getLong(hasInstanceIndex);
            convertToTypeFunction = bufferTemplate.getLong(convertToTypeIndex);
        }
    }

    private static native ByteBuffer NativeGetClassDefinitionTemplate();
}
