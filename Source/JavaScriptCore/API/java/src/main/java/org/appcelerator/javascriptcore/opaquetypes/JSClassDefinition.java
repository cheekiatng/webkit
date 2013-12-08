package com.appcelerator.javascriptcore.opaquetypes;

import java.util.HashMap;

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
    public JSStaticValue[] staticValues;

    /**
     * A JSStaticFunction array containing the class's statically declared
     * function properties. Pass NULL to specify no statically declared function
     * properties. The array must be terminated by a JSStaticFunction whose name
     * field is NULL.
     */
    public JSStaticFunction[] staticFunctions;

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

    private String[] staticValueNamesCache;
    private String[] staticFunctionNamesCache;
    private int[] staticValueAttributesCache;
    private int[] staticFunctionAttributesCache;

    private HashMap<String, JSStaticValue> staticValuesCache;

    public JSClassDefinition() {
        super();
    }

    /* Cache values to avoid reflection from JNI */
    public void updateCache() {
        // staticValues
        if (this.staticValues == null) {
            staticValueNamesCache = new String[0];
            staticValueAttributesCache = new int[0];
            staticValuesCache = new HashMap<String, JSStaticValue>();
        } else {
            staticValueNamesCache = new String[staticValues.length]; 
            staticValueAttributesCache = new int[staticValues.length];
            staticValuesCache = new HashMap<String, JSStaticValue>();
            for (int i = 0; i < staticValues.length; i++) {
                staticValueNamesCache[i] = staticValues[i].name;
                staticValueAttributesCache[i] = staticValues[i].attributes;
                staticValuesCache.put(staticValues[i].name, staticValues[i]);
            }
        }
        // staticFunctions
        if (this.staticFunctions == null) {
            staticFunctionNamesCache = new String[0];
            staticFunctionAttributesCache = new int[0];
        } else {
            staticFunctionNamesCache = new String[staticFunctions.length]; 
            staticFunctionAttributesCache = new int[staticFunctions.length];
            for (int i = 0; i < staticFunctions.length; i++) {
                staticFunctionNamesCache[i] = staticFunctions[i].name;
                staticFunctionAttributesCache[i] = staticFunctions[i].attributes;
            }
        }
    }

    public String[] getStaticValueNames() {
        return staticValueNamesCache;
    }

    public int[] getStaticValueAttributes() {
        return staticValueAttributesCache;
    }

    public String[] getStaticFunctionNames() {
        return staticFunctionNamesCache;
    }

    public int[] getStaticFunctionAttributes() {
        return staticFunctionAttributesCache;
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

    public long JSObjectCallAsFunctionCallback(long ctx, long func, long thisObject, int argc, long[] argv, long exception) {
        if (callAsFunction != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueRef[] jargv = new JSValueRef[argc];
            for (int i = 0; i < argc; i++) {
                jargv[i] = new JSValueRef(context, argv[i]);
            }
            return callAsFunction.apply(context, new JSObjectRef(func), new JSObjectRef(thisObject),
                                        argc, jargv, new JSValueRef(context, exception)).pointer();
        }
        return 0;
    }

    public long JSObjectCallAsConstructorCallback(long ctx, long constructor, int argc, long[] argv, long exception) {
        if (callAsConstructor != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueRef[] jargv = new JSValueRef[argc];
            for (int i = 0; i < argc; i++) {
                jargv[i] = new JSValueRef(context, argv[i]);
            }
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
        if (staticValuesCache.containsKey(propertyName)) {
            JSObjectSetPropertyCallback callback = staticValuesCache.get(propertyName).setProperty;
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
        if (staticValuesCache.containsKey(propertyName)) {
            JSObjectGetPropertyCallback callback = staticValuesCache.get(propertyName).getProperty;
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
}
