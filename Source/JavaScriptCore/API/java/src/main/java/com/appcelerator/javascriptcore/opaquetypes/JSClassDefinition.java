package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;
import java.util.ArrayList;
import java.util.HashMap;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.enums.JSClassAttribute;
import com.appcelerator.javascriptcore.enums.JSType;
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
    private boolean hasParent = false;

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

            if (parentClass != null) JavaScriptCoreLibrary.putLong(buffer, parentClassIndex, parentClass.p());
            if (initialize  != null) JavaScriptCoreLibrary.putLong(buffer, initializeIndex, initializeFunction);
            if (finalize    != null) JavaScriptCoreLibrary.putLong(buffer, finalizeIndex, finalizeFunction);
            if (hasProperty != null) JavaScriptCoreLibrary.putLong(buffer, hasPropertyIndex, hasPropertyFunction);
            if (getProperty != null) JavaScriptCoreLibrary.putLong(buffer, getPropertyIndex, getPropertyFunction);
            if (setProperty != null) JavaScriptCoreLibrary.putLong(buffer, setPropertyIndex, setPropertyFunction);
            if (deleteProperty    != null) JavaScriptCoreLibrary.putLong(buffer, deletePropertyIndex, deletePropertyFunction);
            if (getPropertyNames  != null) JavaScriptCoreLibrary.putLong(buffer, getPropertyNamesIndex, getPropertyNamesFunction);
            if (callAsFunction    != null) JavaScriptCoreLibrary.putLong(buffer, callAsFunctionIndex, callAsFunctionFunction);
            if (callAsConstructor != null) JavaScriptCoreLibrary.putLong(buffer, callAsConstructorIndex, callAsConstructorFunction);
            if (hasInstance   != null) JavaScriptCoreLibrary.putLong(buffer, hasInstanceIndex, hasInstanceFunction);
            if (convertToType != null) JavaScriptCoreLibrary.putLong(buffer, convertToTypeIndex, convertToTypeFunction);

            hasParent = (parentClass != null && parentClass.getDefinition() != null);
        }

        return buffer;

    }

    public void JSObjectInitializeCallback(long ctx, long object) {
        if (hasParent) {
            parentClass.getDefinition().JSObjectInitializeCallback(ctx, object);
        }
        if (initialize != null) {
            initialize.initialize(new JSContextRef(ctx), new JSObjectRef(object));
        }
        clearPrototypeChain(object);
    }

    public void JSObjectFinalizeCallback(long object) {
        if (finalize != null) {
            finalize.finalize(new JSObjectRef(object));
            if (hasParent) {
                parentClass.getDefinition().JSObjectFinalizeCallback(object);
            }
        }
        clearPrototypeChain(object);
    }

    private static HashMap<Long, JSClassDefinition> setPropertyChain = new HashMap<Long, JSClassDefinition>();
    public boolean JSObjectSetPropertyCallback(long ctx, long object, String propertyName, long value, long exception) {
        if (setPropertyChain.containsKey(object) && !this.equals(setPropertyChain.get(object))) {
            return setPropertyChain.get(object).JSObjectSetPropertyCallback(ctx, object, propertyName, value, exception);
        }
        JSContextRef context = new JSContextRef(ctx);
        if (setProperty != null && setProperty.setProperty(context, new JSObjectRef(object), propertyName,
                                        new JSValueRef(context, value), new JSValueRef(context, exception))) {
            setPropertyChain.remove(object);
            return true;
        }
        if (hasParent) {
            setPropertyChain.put(object, parentClass.getDefinition());
            if (setProperty == null) {
                return JSObjectSetPropertyCallback(ctx, object, propertyName, value, exception);
            }
        } else {
            setPropertyChain.remove(object);
        }
        return false;
    }

    private static HashMap<Long, JSClassDefinition> getPropertyChain = new HashMap<Long, JSClassDefinition>();
    public long JSObjectGetPropertyCallback(long ctx, long object, String propertyName, long exception) {
        if (getPropertyChain.containsKey(object) && !this.equals(getPropertyChain.get(object))) {
            return getPropertyChain.get(object).JSObjectGetPropertyCallback(ctx, object, propertyName, exception);
        }
        if (getProperty != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueRef prop = getProperty.getProperty(context, new JSObjectRef(object), propertyName, new JSValueRef(context, exception));
            if (p(prop) != 0) {
                getPropertyChain.remove(object);
                return p(prop);
            }
        }
        if (hasParent) {
            getPropertyChain.put(object, parentClass.getDefinition());
            if (getProperty == null) {
                return JSObjectGetPropertyCallback(ctx, object, propertyName, exception);
            }
        } else {
            getPropertyChain.remove(object);
        }
        return 0;
    }

    public long JSObjectCallAsFunctionCallback(long ctx, long func, long thisObject, int argc, ByteBuffer argv, long exception) {
        if (staticFunctions != null && staticFunctions.contains(func)) {
            JSObjectCallAsFunctionCallback staticFunction = staticFunctions.getFunction(func);
            if (staticFunction != null) {
                JSContextRef context = new JSContextRef(ctx);
                JSValueArrayRef jargv = new JSValueArrayRef(argc, argv);
                return p(staticFunction.callAsFunction(context, new JSObjectRef(func), new JSObjectRef(thisObject),
                                        argc, jargv, new JSValueRef(context, exception)));
            }
        } else if (callAsFunction != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueArrayRef jargv = new JSValueArrayRef(argc, argv);
            return p(callAsFunction.callAsFunction(context, new JSObjectRef(func), new JSObjectRef(thisObject),
                                        argc, jargv, new JSValueRef(context, exception)));
        }
        return 0;
    }

    public long JSObjectCallAsConstructorCallback(long ctx, long constructor, int argc, ByteBuffer argv, long exception) {
        if (callAsConstructor != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueArrayRef jargv = new JSValueArrayRef(argc, argv);
            return p(callAsConstructor.callAsConstructor(context, new JSObjectRef(constructor),
                                           argc, jargv, new JSValueRef(context, exception)));
        }
        return 0;
    }

    private static HashMap<Long, JSClassDefinition> convertToTypeChain = new HashMap<Long, JSClassDefinition>();
    public long JSObjectConvertToTypeCallback(long ctx, long object, int type, long exception) {
        if (convertToTypeChain.containsKey(object) && !this.equals(convertToTypeChain.get(object))) {
            return convertToTypeChain.get(object).JSObjectConvertToTypeCallback(ctx, object, type, exception);
        }
        if (convertToType != null) {
            JSContextRef context = new JSContextRef(ctx);
            JSValueRef prop = convertToType.convertToType(context, new JSObjectRef(object), JSType.request(type), new JSValueRef(context, exception));
            if (p(prop) != 0) {
                convertToTypeChain.remove(object);
                return p(prop);
            }
        }
        if (hasParent) {
            convertToTypeChain.put(object, parentClass.getDefinition());
            if (convertToType == null) {
                return JSObjectConvertToTypeCallback(ctx, object, type, exception);
            }
        } else {
            convertToTypeChain.remove(object);
        }
        return 0;
    }
    
    private static HashMap<Long, JSClassDefinition> deletePropertyChain = new HashMap<Long, JSClassDefinition>();
    public boolean JSObjectDeletePropertyCallback(long ctx, long object, String name, long exception) {
        if (deletePropertyChain.containsKey(object) && !this.equals(deletePropertyChain.get(object))) {
            return deletePropertyChain.get(object).JSObjectDeletePropertyCallback(ctx, object, name, exception);
        }
        JSContextRef context = new JSContextRef(ctx);
        if (deleteProperty != null && deleteProperty.deleteProperty(new JSContextRef(ctx), new JSObjectRef(object), name, new JSValueRef(context, exception))) {
            deletePropertyChain.remove(object);
            return true;
        }
        if (hasParent) {
            deletePropertyChain.put(object, parentClass.getDefinition());
            if (deleteProperty == null) {
                return JSObjectDeletePropertyCallback(ctx, object, name, exception);
            }
        } else {
            deletePropertyChain.remove(object);
        }
        return false;
    }

    private static HashMap<Long, JSClassDefinition> getPropertyNamesChain = new HashMap<Long, JSClassDefinition>();
    public void JSObjectGetPropertyNamesCallback(long ctx, long object, long propertyNames) {
        if (getPropertyNamesChain.containsKey(object) && !this.equals(getPropertyNamesChain.get(object))) {
            getPropertyNamesChain.get(object).JSObjectGetPropertyNamesCallback(ctx, object, propertyNames);
            return;
        }
        if (getPropertyNames != null) {
            getPropertyNames.getPropertyNames(new JSContextRef(ctx), new JSObjectRef(object),
                                              new JSPropertyNameAccumulatorRef(propertyNames));
        }
        if (hasParent) {
            getPropertyNamesChain.put(object, parentClass.getDefinition());
            if (getPropertyNames == null) {
                JSObjectGetPropertyNamesCallback(ctx, object, propertyNames);
                return;
            }
        } else {
            getPropertyNamesChain.remove(object);
        }
        return;
    }

    public boolean JSObjectHasInstanceCallback(long ctx, long constructor, long possibleInstance, long exception) {
        if (hasInstance != null) {
            JSContextRef context = new JSContextRef(ctx);
            return hasInstance.hasInstance(context, new JSObjectRef(constructor),
                                     new JSValueRef(context, possibleInstance), new JSValueRef(context, exception));
        }
        return false;
    }

    private static HashMap<Long, JSClassDefinition> hasPropertyChain = new HashMap<Long, JSClassDefinition>();
    public boolean JSObjectHasPropertyCallback(long ctx, long object, String name) {
        if (hasPropertyChain.containsKey(object) && !this.equals(hasPropertyChain.get(object))) {
            return hasPropertyChain.get(object).JSObjectHasPropertyCallback(ctx, object, name);
        }
        if (hasProperty != null && hasProperty.hasProperty(new JSContextRef(ctx), new JSObjectRef(object), name)) {
            hasPropertyChain.remove(object);
            return true;
        }
        if (hasParent) {
            hasPropertyChain.put(object, parentClass.getDefinition());
            if (hasProperty == null) {
                return JSObjectHasPropertyCallback(ctx, object, name);
            }
        } else {
            hasPropertyChain.remove(object);
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
            return callback.setProperty(context, new JSObjectRef(object), propertyName,
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
            return p(callback.getProperty(context, new JSObjectRef(object),
                        propertyName, new JSValueRef(context, exception)));
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

            initializeFunction  = JavaScriptCoreLibrary.getLong(bufferTemplate, initializeIndex);
            finalizeFunction    = JavaScriptCoreLibrary.getLong(bufferTemplate, finalizeIndex);
            hasPropertyFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, hasPropertyIndex);
            getPropertyFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, getPropertyIndex);
            setPropertyFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, setPropertyIndex);
            deletePropertyFunction    = JavaScriptCoreLibrary.getLong(bufferTemplate, deletePropertyIndex);
            getPropertyNamesFunction  = JavaScriptCoreLibrary.getLong(bufferTemplate, getPropertyNamesIndex);
            callAsFunctionFunction    = JavaScriptCoreLibrary.getLong(bufferTemplate, callAsFunctionIndex);
            callAsConstructorFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, callAsConstructorIndex);
            hasInstanceFunction   = JavaScriptCoreLibrary.getLong(bufferTemplate, hasInstanceIndex);
            convertToTypeFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, convertToTypeIndex);
        }
    }

    private long p(PointerType p) {
        if (p == null) return 0;
        return p.pointer();
    }

    /*
     * Just in case make sure to remove all references associated with the object
     */
    private static void clearPrototypeChain(long object) {
        setPropertyChain.remove(object);
        getPropertyChain.remove(object);
        convertToTypeChain.remove(object);
        deletePropertyChain.remove(object);
        getPropertyNamesChain.remove(object);
        hasPropertyChain.remove(object);
    }

    private static native ByteBuffer NativeGetClassDefinitionTemplate();
}
