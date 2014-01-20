package com.appcelerator.javascriptcore;

import java.nio.ByteBuffer;

import com.appcelerator.javascriptcore.opaquetypes.JSContextGroupRef;
import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSGlobalContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueArrayRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassDefinition;
import com.appcelerator.javascriptcore.opaquetypes.PointerType;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;
import com.appcelerator.javascriptcore.opaquetypes.JSPropertyNameArrayRef;
import com.appcelerator.javascriptcore.opaquetypes.JSPropertyNameAccumulatorRef;

import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsConstructorCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsFunctionCallback;

import com.appcelerator.javascriptcore.enums.JSType;
import com.appcelerator.javascriptcore.enums.JSPropertyAttribute;

public class JavaScriptCoreLibrary {

    static {
        System.loadLibrary("JavaScriptCoreJNI");
    }   

    private static final JavaScriptCoreLibrary instance = new JavaScriptCoreLibrary();

    public static final short SizeOfLong = NativeSizeOfLong();
    public static final short SizeOfInt  = NativeSizeOfInt();
    public static final short SizeOfUnsigned = NativeSizeOfUnsigned();
    public static final short SizeOfJSClassDefinition = NativeSizeOfJSClassDefinition();
    public static final short SizeOfJSStaticValue    = NativeSizeOfJSStaticValue();
    public static final short SizeOfJSStaticFunction = NativeSizeOfJSStaticFunction();

    /*
     * Number of buckets for JS object map that is used by callbacks. 
     * If object map requires more bucket, LongSparseArray realocates the map and GC may be taking place.
     * If you want to avoid GC to handle more JS objects just increase this number.
     */
    public static int numberOfJSObjectBuckets = 16384;

    /*
     * Number of buckets for prototype hierarchy that is used by callbacks.
     * If hierarchy map requires more space, LongSparseArray reallocates the map.
     */
    public static int numberOfPrototypeHierarchy = 16;

    /*
     * Singleton
     */
    private JavaScriptCoreLibrary() {}
    public static JavaScriptCoreLibrary getInstance() {
        return instance;
    }

    /*
     * Utility functions
     */
    public static void putLong(ByteBuffer buffer, int index, long value) {
        if (SizeOfLong == 8) {
            // 64 bit
            buffer.putLong(index, value);
        } else {
            // 32 bit
            buffer.putInt(index, (int)value);
        }
    }
    public static long getLong(ByteBuffer buffer, int index) {
        if (SizeOfLong == 8) {
            // 64 bit
            return buffer.getLong(index);
        } else {
            // 32 bit
            return buffer.getInt(index);
        }
    }

    private long p(PointerType p) {
        if (p == null) return 0;
        return p.pointer();
    }

    /*
     * Native method wrappers
     */
    public JSObjectRef JSContextGetGlobalObject(JSContextRef context) {
        return new JSObjectRef(context, NativeJSContextGetGlobalObject(p(context)));
    }

    public JSContextGroupRef JSContextGetGroup(JSContextRef context) {
        return new JSContextGroupRef(NativeJSContextGetGroup(p(context)));
    }

    public JSContextGroupRef JSContextGroupCreate() {
        return new JSContextGroupRef(NativeJSContextGroupCreate());
    }

    public void JSContextGroupRelease(JSContextGroupRef group) {
        NativeJSContextGroupRelease(p(group));
    }

    public JSContextGroupRef JSContextGroupRetain(JSContextGroupRef group) {
        return new JSContextGroupRef(NativeJSContextGroupRetain(p(group)));
    }

    public JSGlobalContextRef JSGlobalContextCreate(JSClassRef jsClass) {
        return new JSGlobalContextRef(NativeJSGlobalContextCreate(p(jsClass),
                                        jsClass == null ? null : jsClass.getDefinition()));
    }

    public JSGlobalContextRef JSGlobalContextCreateInGroup(JSContextGroupRef group, JSClassRef jsClass) {
        return new JSGlobalContextRef(NativeJSGlobalContextCreateInGroup(p(group), p(jsClass),
                                        jsClass == null ? null : jsClass.getDefinition()));
    }

    public void JSGlobalContextRelease(JSGlobalContextRef context) {
        NativeJSGlobalContextRelease(p(context));
    }

    public JSGlobalContextRef JSGlobalContextRetain(JSGlobalContextRef context) {
        return new JSGlobalContextRef(NativeJSGlobalContextRetain(p(context)));
    }

    public JSValueRef JSEvaluateScript(JSContextRef context, String script, JSValueRef exception) {
        return new JSValueRef(context, NativeJSEvaluateScriptShort(p(context), script, exception));
    }

    /**
     * Evaluates a string of JavaScript.
     * 
     * @param ctx
     *            The execution context to use.
     * @param script
     *            A JSString containing the script to evaluate.
     * @param thisObject
     *            The object to use as "this," or NULL to use the global object
     *            as "this."
     * @param sourceURL
     *            A JSString containing a URL for the script's source file. This
     *            is only used when reporting exceptions. Pass NULL if you do
     *            not care to include source file information in exceptions.
     * @param startingLineNumber
     *            An integer value specifying the script's starting line number
     *            in the file located at sourceURL. This is only used when
     *            reporting exceptions.
     * @param exception
     *            A pointer to a JSValueRef in which to store an exception, if
     *            any. Pass NULL if you do not care to store an exception.
     * @return The JSValue that results from evaluating script, or NULL if an
     *         exception is thrown.
     */
    public JSValueRef JSEvaluateScript(JSContextRef context, String script, JSObjectRef object, String sourceURL, int line, JSValueRef exception) {
        return new JSValueRef(context, NativeJSEvaluateScriptFull(p(context), script, p(object), sourceURL, line, exception));
    }

    public boolean JSCheckScriptSyntax(JSContextRef context, String script, JSValueRef exception) {
        return NativeJSCheckScriptSyntax(p(context), script, exception);
    }

    /**
     * Performs a JavaScript garbage collection. JavaScript values that are on
     * the machine stack, in a register, protected by JSValueProtect, set as the
     * global object of an execution context, or reachable from any such value
     * will not be collected. During JavaScript execution, you are not required
     * to call this function; the JavaScript engine will garbage collect as
     * needed. JavaScript values created within a context group are
     * automatically destroyed when the last reference to the context group is
     * released.
     * 
     * @param ctx
     *            The execution context to use.
     */
    public void JSGarbageCollect(JSContextRef context) {
        NativeJSGarbageCollect(p(context));
    }

    public void JSValueProtect(JSContextRef context, JSValueRef value) {
        NativeJSValueProtect(p(context), p(value));
    }

    public void JSValueUnprotect(JSContextRef context, JSValueRef value) {
        NativeJSValueUnprotect(p(context), p(value));
    }

    public String JSValueCreateJSONString(JSContextRef context, JSValueRef value, int indent, JSValueRef exception) {
        return NativeJSValueCreateJSONString(p(context), p(value), indent, exception);
    }

    public JSValueRef JSValueMakeBoolean(JSContextRef context, boolean value) {
        return new JSValueRef(context, NativeJSValueMakeBoolean(p(context), value));
    }

    public JSValueRef JSValueMakeNull(JSContextRef context) {
        return new JSValueRef(context, NativeJSValueMakeNull(p(context)));
    }

    public JSValueRef JSValueMakeNumber(JSContextRef context, double number) {
        return new JSValueRef(context, NativeJSValueMakeNumber(p(context), number));
    }

    public JSValueRef JSValueMakeString(JSContextRef context, String string) {
        return new JSValueRef(context, NativeJSValueMakeString(p(context), string));
    }

    public JSValueRef JSValueMakeUndefined(JSContextRef context) {
        return new JSValueRef(context, NativeJSValueMakeUndefined(p(context)));
    }

    public JSValueRef JSValueMakeFromJSONString(JSContextRef context, String string) {
        return new JSValueRef(context, NativeJSValueMakeFromJSONString(p(context), string));
    }

    public boolean JSValueIsUndefined(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsUndefined(p(context), p(value));
    }

    public boolean JSValueIsNull(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsNull(p(context), p(value));
    }

    public boolean JSValueIsNumber(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsNumber(p(context), p(value));
    }

    public boolean JSValueIsBoolean(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsBoolean(p(context), p(value));
    }

    public boolean JSValueIsString(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsString(p(context), p(value));
    }

    public boolean JSValueIsObject(JSContextRef context, JSValueRef value) {
        return NativeJSValueIsObject(p(context), p(value));
    }

    public boolean JSValueIsEqual(JSContextRef context, JSValueRef a, JSValueRef b, JSValueRef exception) {
        return NativeJSValueIsEqual(p(context), p(a), p(b), exception);
    }

    public boolean JSValueIsInstanceOfConstructor(JSContextRef context, JSValueRef value, JSObjectRef constructor, JSValueRef exception) {
        return NativeJSValueIsInstanceOfConstructor(p(context), p(value), p(constructor), exception);
    }

    public boolean JSValueIsObjectOfClass(JSContextRef context, JSValueRef value, JSClassRef jsClass) {
        return NativeJSValueIsObjectOfClass(p(context), p(value), p(jsClass));
    }

    public boolean JSValueIsStrictEqual(JSContextRef context, JSValueRef a, JSValueRef b) {
        return NativeJSValueIsStrictEqual(p(context), p(a), p(b));
    }

    public boolean JSValueToBoolean(JSContextRef context, JSValueRef value) {
        return NativeJSValueToBoolean(p(context), p(value));
    }

    public double JSValueToNumber(JSContextRef context, JSValueRef value, JSValueRef exception) {
        return NativeJSValueToNumber(p(context), p(value), exception);
    }

    public String JSValueToStringCopy(JSContextRef context, JSValueRef value, JSValueRef exception) {
        return NativeJSValueToStringCopy(p(context), p(value), exception);
    }

    public JSObjectRef JSValueToObject(JSContextRef context, JSValueRef value, JSValueRef exception) {
        return new JSObjectRef(context, NativeJSValueToObject(p(context), p(value), exception));
    }

    public JSType JSValueGetType(JSContextRef context, JSValueRef value) {
        return JSType.request(NativeJSValueGetType(p(context), p(value)));
    }

    /*
     * Special function to create global object class
     */
    public JSClassRef JSGlobalClassCreate(JSClassDefinition d) {
        d.enableFinalize(false);
        return new JSClassRef(d, NativeJSClassCreate(d.commit(), d.className, d.getStaticValues(), d.getStaticFunctions()));
    }

    public JSClassRef JSClassCreate(JSClassDefinition d) {
        return new JSClassRef(d, NativeJSClassCreate(d.commit(), d.className, d.getStaticValues(), d.getStaticFunctions()));
    }

    public JSObjectRef JSObjectCallAsConstructor(JSContextRef context, JSObjectRef jsObject, JSValueArrayRef argv, JSValueRef exception) {
        if (argv == null) {
            return new JSObjectRef(context, NativeJSObjectCallAsConstructor(p(context), p(jsObject), 0, null, exception));
        } else {
            return new JSObjectRef(context, NativeJSObjectCallAsConstructor(p(context), p(jsObject), argv.length(), argv.getByteBuffer(), exception));
        }
    }

    public JSValueRef JSObjectCallAsFunction(JSContextRef context, JSObjectRef jsObject,
                                             JSObjectRef thisObject, JSValueArrayRef argv, JSValueRef exception) {
        if (argv == null) {
            return new JSValueRef(context, NativeJSObjectCallAsFunction(p(context), p(jsObject), p(thisObject), 0, null, exception));
        } else {
            return new JSValueRef(context, NativeJSObjectCallAsFunction(p(context), p(jsObject), p(thisObject), argv.length(), argv.getByteBuffer(), exception));
        }
    }

    public void JSObjectSetProperty(JSContextRef context, JSObjectRef jsObject,
                                    String propertyName, JSValueRef value, JSPropertyAttribute attributes, JSValueRef exception) {
        NativeJSObjectSetProperty(p(context), p(jsObject), propertyName, p(value), attributes.getValue(), exception);
    }

    public JSValueRef JSObjectGetProperty(JSContextRef context, JSObjectRef jsObject,
                                            String propertyName, JSValueRef exception) {
        return new JSValueRef(context, NativeJSObjectGetProperty(p(context), p(jsObject), propertyName, exception));
    }
    
    public void JSClassRelease(JSClassRef jsClass) {
        NativeJSClassRelease(p(jsClass));
    }
    public JSClassRef JSClassRetain(JSClassRef jsClass) {
        return new JSClassRef(jsClass.getDefinition(), NativeJSClassRetain(p(jsClass)));
    }

    /**
     * Gets the names of an object's enumerable properties.
     * 
     * @param ctx
     *            The execution context to use.
     * @param object
     *            The object whose property names you want to get.
     * @return A JSPropertyNameArray containing the names object's enumerable
     *         properties. Ownership follows the Create Rule.
     */
    public JSPropertyNameArrayRef JSObjectCopyPropertyNames(JSContextRef context, JSObjectRef jsObject) {
        return new JSPropertyNameArrayRef(NativeJSObjectCopyPropertyNames(p(context), p(jsObject)));
    }

    public boolean JSObjectDeleteProperty(JSContextRef context, JSObjectRef jsObject, String propertyName, JSValueRef exception) {
        return NativeJSObjectDeleteProperty(p(context), p(jsObject), propertyName, exception);
    }

    public JSValueRef JSObjectGetPropertyAtIndex(JSContextRef context, JSObjectRef jsObject, int propertyIndex, JSValueRef exception) {
        return new JSValueRef(context, NativeJSObjectGetPropertyAtIndex(p(context), p(jsObject), propertyIndex, exception));
    }

    public JSValueRef JSObjectGetPrototype(JSContextRef context, JSObjectRef jsObject) {
        return new JSValueRef(context, NativeJSObjectGetPrototype(p(context), p(jsObject)));
    }

    public boolean JSObjectHasProperty(JSContextRef context, JSObjectRef jsObject, String propertyName) {
        return NativeJSObjectHasProperty(p(context), p(jsObject), propertyName);
    }

    public boolean JSObjectIsConstructor(JSContextRef context, JSObjectRef jsObject) {
        return NativeJSObjectIsConstructor(p(context), p(jsObject));
    }

    public boolean JSObjectIsFunction(JSContextRef context, JSObjectRef jsObject) {
        return NativeJSObjectIsFunction(p(context), p(jsObject));
    }

    public JSObjectRef JSObjectMakeArray(JSContextRef context, JSValueArrayRef argv, JSValueRef exception) {
        if (argv == null) {
            return new JSObjectRef(context, NativeJSObjectMakeArray(p(context), 0, null, exception));
        } else {
            return new JSObjectRef(context, NativeJSObjectMakeArray(p(context), argv.length(), argv.getByteBuffer(), exception));
        }
    }

    public JSObjectRef JSObjectMakeDate(JSContextRef context, JSValueArrayRef argv, JSValueRef exception) {
        if (argv == null) {
            return new JSObjectRef(context, NativeJSObjectMakeDate(p(context), 0, null, exception));
        } else {
            return new JSObjectRef(context, NativeJSObjectMakeDate(p(context), argv.length(), argv.getByteBuffer(), exception));
        }
    }
    public JSObjectRef JSObjectMakeError(JSContextRef context, JSValueArrayRef argv, JSValueRef exception) {
        if (argv == null) {
            return new JSObjectRef(context, NativeJSObjectMakeError(p(context), 0, null, exception));
        } else {
            return new JSObjectRef(context, NativeJSObjectMakeError(p(context), argv.length(), argv.getByteBuffer(), exception));
        }
    }
    public JSObjectRef JSObjectMakeRegExp(JSContextRef context, JSValueArrayRef argv, JSValueRef exception) {
        if (argv == null) {
            return new JSObjectRef(context, NativeJSObjectMakeRegExp(p(context), 0, null, exception));
        } else {
            return new JSObjectRef(context, NativeJSObjectMakeRegExp(p(context), argv.length(), argv.getByteBuffer(), exception));
        }
    }
    public JSObjectRef JSObjectMakeFunction(JSContextRef context, String name, int paramCount,
                                            String paramNames[], String body, String sourceURL,
                                            int line, JSValueRef exception) {
        return new JSObjectRef(context, NativeJSObjectMakeFunction(p(context), name, paramCount,
                                            paramNames, body, sourceURL, line, exception));
    }

    public void JSObjectSetPropertyAtIndex(JSContextRef context, JSObjectRef jsObject,
                                            int propertyIndex, JSValueRef value, JSValueRef exception) {
        NativeJSObjectSetPropertyAtIndex(p(context), p(jsObject), propertyIndex, p(value), exception);
    }

    public void JSObjectSetPrototype(JSContextRef context, JSObjectRef jsObject, JSValueRef value) {
        NativeJSObjectSetPrototype(p(context), p(jsObject), p(value));
    }

    public void JSPropertyNameAccumulatorAddName(JSPropertyNameAccumulatorRef accumulator, String propertyName) {
        NativeJSPropertyNameAccumulatorAddName(p(accumulator), propertyName);
    }

    public int JSPropertyNameArrayGetCount(JSPropertyNameArrayRef names) {
        return NativeJSPropertyNameArrayGetCount(p(names));
    }

    public String JSPropertyNameArrayGetNameAtIndex(JSPropertyNameArrayRef names, int index) {
        return NativeJSPropertyNameArrayGetNameAtIndex(p(names), index);
    }

    public void JSPropertyNameArrayRelease(JSPropertyNameArrayRef names) {
        NativeJSPropertyNameArrayRelease(p(names));
    }

    public JSPropertyNameArrayRef JSPropertyNameArrayRetain(JSPropertyNameArrayRef names) {
        return new JSPropertyNameArrayRef(NativeJSPropertyNameArrayRetain(p(names)));
    }

    public Object JSObjectGetPrivate(JSObjectRef jsObject) {
        return NativeJSObjectGetPrivate(p(jsObject));
    }

    public boolean JSObjectSetPrivate(JSObjectRef jsObject, Object object) {
        return NativeJSObjectSetPrivate(p(jsObject), object);
    }

    public JSObjectRef JSObjectMake(JSContextRef context, JSClassRef jsClass) {
        return JSObjectMake(context, jsClass, null);
    }

    public JSObjectRef JSObjectMake(JSContextRef context, JSClassRef jsClass, Object object) {
        if (jsClass == null) {
            return new JSObjectRef(context, NativeJSObjectMake(p(context), 0, null, null, 0, object));
        } else {
            return JSObjectMakeWithDefinition(context, jsClass, jsClass.getDefinition(), object);
        }
    }

    private JSObjectRef JSObjectMakeWithDefinition(JSContextRef context, JSClassRef jsClass,
                                                  JSClassDefinition definition, Object object) {
        return new JSObjectRef(context, NativeJSObjectMake(p(context), p(jsClass),
                        definition, definition.getStaticFunctions(), definition.getStaticFunctionCount(), object));
    }

    /**
     * Convenience method for creating a JavaScript constructor
     */
    public JSObjectRef JSObjectMakeConstructor(final JSContextRef context, final JSClassRef jsClass,
                                               JSObjectCallAsConstructorCallback callback) {
        final JSClassDefinition jsClassDefinition = jsClass == null ? null : jsClass.getDefinition().copy();
        if (jsClass != null) {
            jsClassDefinition.enableConstructor(true);
            jsClassDefinition.commit();
            if (callback == null) {
                callback = new JSObjectCallAsConstructorCallback() {
                    public JSObjectRef callAsConstructor(JSContextRef ctx, JSObjectRef constructor,
                                   int argumentCount, JSValueArrayRef arguments, Pointer exception) {
                        return JSObjectMakeWithDefinition(context, jsClass, jsClassDefinition, null);
                    }
                };
            }
            jsClassDefinition.callAsConstructor = callback;
        }
        long constructor = NativeJSObjectMakeConstructor(p(context), p(jsClass), jsClassDefinition);
        JSClassDefinition.registerMakeConstructorCallback(constructor, callback);
        return new JSObjectRef(context, constructor);
    }

    /**
     * Convenience method for creating a JavaScript function with a given callback as its implementation
     */
    public JSObjectRef JSObjectMakeFunctionWithCallback(JSContextRef context, String name,
                                                        JSObjectCallAsFunctionCallback callback) {
        long function = NativeJSObjectMakeFunctionWithCallback(p(context), name);
        JSClassDefinition.registerMakeFunctionCallback(function, callback);
        return new JSObjectRef(context, function);
    }

    /*
     * Private API
     */
    public JSGlobalContextRef JSContextGetGlobalContext(JSContextRef ctx) {
        return new JSGlobalContextRef(NativeJSContextGetGlobalContext(p(ctx)));
    }

    public String JSContextCreateBacktrace(JSContextRef ctx, int maxStackSize) {
        return NativeJSContextCreateBacktrace(p(ctx), maxStackSize);
    }

    public boolean JSObjectSetPrivateProperty(JSContextRef ctx, JSObjectRef object, String propertyName, JSValueRef value) {
        return NativeJSObjectSetPrivateProperty(p(ctx), p(object), propertyName, p(value));
    }

    public JSValueRef JSObjectGetPrivateProperty(JSContextRef ctx, JSObjectRef object, String propertyName) {
        return new JSValueRef(ctx, NativeJSObjectGetPrivateProperty(p(ctx), p(object), propertyName));
    }

    public boolean JSObjectDeletePrivateProperty(JSContextRef ctx, JSObjectRef object, String propertyName) {
        return NativeJSObjectDeletePrivateProperty(p(ctx), p(object), propertyName);
    }


    /*
     * Native methods
     */
    public static native short NativeSizeOfLong();
    public static native short NativeSizeOfInt();
    public static native short NativeSizeOfUnsigned();
    public static native short NativeSizeOfJSClassDefinition();
    public static native short NativeSizeOfJSStaticFunction();
    public static native short NativeSizeOfJSStaticValue();
    public static native long[] NativeAllocateCharacterBuffer(String[] values);
    public static native void  NativeReleasePointers(long[] pointers);

    public native long NativeJSContextGetGlobalObject(long jsContextRef);
    public native long NativeJSContextGetGroup(long jsContextRef);
    public native long NativeJSContextGroupCreate();
    public native void NativeJSContextGroupRelease(long jsContextGroupRef);
    public native long NativeJSContextGroupRetain(long jsContextGroupRef);
    public native long NativeJSGlobalContextCreate(long jsClassRef, JSClassDefinition callabck);
    public native long NativeJSGlobalContextCreateInGroup(long jsContextGroupRef, long jsClassRef, JSClassDefinition callback);
    public native void NativeJSGlobalContextRelease(long jsContextRef);
    public native long NativeJSGlobalContextRetain(long jsContextRef);
    public native long NativeJSEvaluateScriptShort(long jsContextRef, String script, JSValueRef exception);
    public native long NativeJSEvaluateScriptFull(long jsContextRef, String script, long jsObjectRef, String sourceURL, int line, JSValueRef exception);
    public native boolean NativeJSCheckScriptSyntax(long jsContextRef, String script, JSValueRef exception);
    public native long NativeJSGarbageCollect(long jsContextRef);
    public native void NativeJSValueProtect(long jsContextRef, long jsValueRef);
    public native void NativeJSValueUnprotect(long jsContextRef, long jsValueRef);
    public native String NativeJSValueCreateJSONString(long jsContextRef, long jsValueRef, int indent, JSValueRef exception);
    public native long NativeJSValueMakeNull(long jsContextRef);
    public native long NativeJSValueMakeUndefined(long jsContextRef);
    public native long NativeJSValueMakeBoolean(long jsContextRef, boolean value);
    public native long NativeJSValueMakeNumber(long jsContextRef, double number);
    public native long NativeJSValueMakeString(long jsContextRef, String string);
    public native long NativeJSValueMakeFromJSONString(long jsContextRef, String string);
    public native boolean NativeJSValueIsObjectOfClass(long jsContextRef, long jsValueRef, long jsClassRef);
    public native boolean NativeJSValueIsInstanceOfConstructor(long jsContextRef, long jsValueRef, long jsObjectRef, JSValueRef exception);
    public native boolean NativeJSValueIsUndefined(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsNull(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsNumber(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsBoolean(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsString(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsObject(long jsContextRef, long jsValueRef);
    public native boolean NativeJSValueIsEqual(long jsContextRef, long jsValueRefA, long jsValueRefB, JSValueRef exception);
    public native boolean NativeJSValueIsStrictEqual(long jsContextRef, long jsValueRefA, long jsValueRefB);
    public native boolean NativeJSValueToBoolean(long jsContextRef, long jsValueRef);
    public native double NativeJSValueToNumber(long jsContextRef, long jsValueRef, JSValueRef exception);
    public native long NativeJSValueToObject(long jsContextRef, long jsValueRef, JSValueRef exception);
    public native String NativeJSValueToStringCopy(long jsContextRef, long jsValueRef, JSValueRef exception);
    public native int NativeJSValueGetType(long jsContextRef, long jsValueRef);

    public native long NativeJSClassCreate(ByteBuffer definition, String className, ByteBuffer staticValues, ByteBuffer staticFunctions);

    public native void NativeJSClassRelease(long jsClassRef);
    public native long NativeJSClassRetain(long jsClassRef);
    public native long NativeJSObjectCallAsConstructor(long jsContextRef, long jsObjectRef, int argc, ByteBuffer argv, JSValueRef exception);
    public native long NativeJSObjectCallAsFunction(long jsContextRef, long jsObjectRef, long thisObjectRef, int argc, ByteBuffer argv, JSValueRef exception);
    public native long NativeJSObjectCopyPropertyNames(long jsContextRef, long jsObjectRef);
    public native boolean NativeJSObjectDeleteProperty(long jsContextRef, long jsObjectRef, String propertyName, JSValueRef exception);
    public native Object NativeJSObjectGetPrivate(long jsObjectRef);
    public native boolean NativeJSObjectSetPrivate(long jsObjectRef, Object object);
    public native long NativeJSObjectGetProperty(long jsContextRef, long jsObjectRef, String propertyName, JSValueRef exception);
    public native long NativeJSObjectGetPropertyAtIndex(long jsContextRef, long jsObjectRef, int propertyIndex, JSValueRef exception);
    public native long NativeJSObjectGetPrototype(long jsContextRef, long jsObjectRef);
    public native boolean NativeJSObjectHasProperty(long jsContextRef, long jsObjectRef, String propertyName);
    public native boolean NativeJSObjectIsConstructor(long jsContextRef, long jsObjectRef);
    public native boolean NativeJSObjectIsFunction(long jsContextRef, long jsObjectRef);
    public native long NativeJSObjectMake(long jsContextRef, long jsClassRef, JSClassDefinition definition, ByteBuffer staticFunctions, int staticFunctionCount, Object object);
    public native long NativeJSObjectMakeArray(long jsContextRef, int argc, ByteBuffer argv, JSValueRef exception);
    public native long NativeJSObjectMakeDate(long jsContextRef, int argc, ByteBuffer argv, JSValueRef exception);
    public native long NativeJSObjectMakeError(long jsContextRef, int argc, ByteBuffer argv, JSValueRef exception);
    public native long NativeJSObjectMakeFunction(long jsContextRef, String name, int paramCount, String paramNames[], String body, String sourceURL, int line, JSValueRef exception);
    public native long NativeJSObjectMakeRegExp(long jsContextRef, int argc, ByteBuffer argv, JSValueRef exception);
    public native void NativeJSObjectSetProperty(long jsContextRef, long jsObjectRef, String propertyName, long jsValueRef, int attributes, JSValueRef exception);
    public native void NativeJSObjectSetPropertyAtIndex(long jsContextRef, long jsObjectRef, int propertyIndex, long jsValueRef, JSValueRef exception);
    public native void NativeJSObjectSetPrototype(long jsContextRef, long jsObjectRef, long jsValueRef);
    public native void NativeJSPropertyNameAccumulatorAddName(long accumulator, String propertyName);
    public native int NativeJSPropertyNameArrayGetCount(long jsPropertyNameArrayRef);
    public native String NativeJSPropertyNameArrayGetNameAtIndex(long jsPropertyNameArrayRef, int index);
    public native void NativeJSPropertyNameArrayRelease(long jsPropertyNameArrayRef);
    public native long NativeJSPropertyNameArrayRetain(long jsPropertyNameArrayRef);
    public native long NativeJSObjectMakeConstructor(long jsContextRef, long jsClassRef, JSClassDefinition definition);
    public native long NativeJSObjectMakeFunctionWithCallback(long jsContextRef, String name);

    /* Private API */
    public native long NativeJSContextGetGlobalContext(long jsContextRef);
    public native String NativeJSContextCreateBacktrace(long jsContextRef, int maxStackSize);
    public native boolean NativeJSObjectSetPrivateProperty(long jsContextRef, long jsObjectRef, String propertyName, long jsValueRef);
    public native long NativeJSObjectGetPrivateProperty(long jsContextRef, long jsObjectRef, String propertyName);
    public native boolean NativeJSObjectDeletePrivateProperty(long jsContextRef, long jsObjectRef, String propertyName);
}
