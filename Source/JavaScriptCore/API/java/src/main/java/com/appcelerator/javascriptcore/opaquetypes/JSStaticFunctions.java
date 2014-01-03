package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import java.util.List;
import java.util.HashMap;
import java.util.ArrayList;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;
import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsFunctionCallback;
import com.appcelerator.javascriptcore.enums.JSPropertyAttribute;

public class JSStaticFunctions {

    private static final long  NULL  = 0;
    private static final short LONG  = JavaScriptCoreLibrary.SizeOfLong;
    private static final short LONG2 = (short)(LONG * 2);
    private static final short CHUNK = JavaScriptCoreLibrary.SizeOfJSStaticFunction;

    private List<String> namesCache = new ArrayList<String>();
    private HashMap<String, JSStaticFunction> functions = new HashMap<String, JSStaticFunction>();

    private static final ByteOrder nativeOrder = ByteOrder.nativeOrder();
    private static ByteBuffer bufferTemplate = null;
    private static long callAsFunction;

    private ByteBuffer buffer = null;
    private long[] addressForNames;

    public JSStaticFunctions() {
        if (bufferTemplate == null) {
            bufferTemplate = NativeGetStaticFunctionTemplate().order(nativeOrder);
            callAsFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, LONG);
        }
    }

    /*
     * Commit the changes.
     * Note that buffer allocation is done only once.
     */
    public ByteBuffer commit() {
        if (buffer == null) {
            int size = namesCache.size();
            buffer = ByteBuffer.allocateDirect(CHUNK * (size + 1)).order(nativeOrder);
            addressForNames = JavaScriptCoreLibrary.NativeAllocateCharacterBuffer(namesCache.toArray(new String[size]));
            for (int i = 0; i < size; i++) {
                update(namesCache.get(i), i * CHUNK, addressForNames[i]);
            }
            updateLast(size);
        }
        return buffer;
    }

    private void update(String name, int index, long addressForNames) {
        JavaScriptCoreLibrary.putLong(buffer, index, addressForNames);
        JSStaticFunction function = functions.get(name);
        if (function.callback == null) {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG, 0);
        } else {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG, callAsFunction);
        }
        buffer.putInt(index +LONG2, function.attributes);
    }

    private void updateLast(int last) {
        int index = CHUNK * last;
        JavaScriptCoreLibrary.putLong(buffer, index, NULL);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG, NULL);
        buffer.putInt(index+LONG2, 0);
    }

    public void dispose() {
        functionPointers.clear();
        functionPointers = null;
        bufferTemplate.clear();
        bufferTemplate = null;
        if (buffer != null) buffer.clear();
        buffer = null;
        namesCache  = null;
        functions = null;
        if (addressForNames != null) JavaScriptCoreLibrary.NativeReleasePointers(addressForNames);
    }

    public int size() {
        return functions.size();
    }
    
    public void add(String name, JSObjectCallAsFunctionCallback callback, JSPropertyAttribute attrs) {
        if (buffer != null) throw new JavaScriptException("No changes can be done after commit()");
        functions.put(name, new JSStaticFunction(callback, attrs.getValue()));
        namesCache.add(name);
    }

    private HashMap<Long, HashMap<Long, String>> functionPointers = new HashMap<Long, HashMap<Long, String>>();
    public void registerFunctions(long object, long[] pointers) {
        removeObject(object);
        HashMap<Long, String> funcs = new HashMap<Long, String>();
        for (int i = 0; i < pointers.length; i++) {
            funcs.put(pointers[i], namesCache.get(i));
        }
        functionPointers.put(object, funcs);       
    }
    
    public void removeObject(long object) {
        functionPointers.remove(object);
    }

    public boolean requestFunctions(long object) {
        return !functionPointers.containsKey(object);
    }

    public JSObjectCallAsFunctionCallback getFunction(long object, long pointer) {
        if (!functionPointers.containsKey(object)) return null;
        if (!functionPointers.get(object).containsKey(pointer)) return null;
        return functions.get(functionPointers.get(object).get(pointer)).callback;
    }

    private class JSStaticFunction {
        public JSObjectCallAsFunctionCallback callback;
        public int attributes;
        public JSStaticFunction(JSObjectCallAsFunctionCallback callback, int attributes) {
            this.callback = callback;
            this.attributes = attributes;
        }        
    }

    private static native ByteBuffer NativeGetStaticFunctionTemplate();

}