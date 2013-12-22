package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import java.util.List;
import java.util.HashMap;
import java.util.ArrayList;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;
import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsFunctionCallback;

public class JSStaticFunctions {

    private static final long  NULL  = 0;
    private static final short LONG  = JavaScriptCoreLibrary.SizeOfLong;
    private static final short LONG2 = (short)(LONG * 2);
    private static final short CHUNK = JavaScriptCoreLibrary.SizeOfJSStaticFunction;

    private List<String> names = new ArrayList<String>();
    private HashMap<String, JSObjectCallAsFunctionCallback> functions = new HashMap<String, JSObjectCallAsFunctionCallback>();
    private HashMap<String, Integer> attributes = new HashMap<String, Integer>();

    private static final ByteOrder nativeOrder = ByteOrder.nativeOrder();
    private static ByteBuffer bufferTemplate = null;
    private static long callAsFunction;

    private ByteBuffer buffer = null;
    private long[] addressForNames;
    private boolean frozen = false;

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
        if (!frozen) {
            int size = attributes.size();
            buffer = ByteBuffer.allocateDirect(CHUNK * (size + 1)).order(nativeOrder);
            addressForNames = JavaScriptCoreLibrary.NativeAllocateCharacterBuffer(names.toArray(new String[size]));
            for (int i = 0; i < size; i++) {
                update(names.get(i), i * CHUNK, addressForNames[i]);
            }
            updateLast(size);
            frozen = true;
        }
        return buffer;
    }

    private void update(String name, int index, long addressForNames) {
        JavaScriptCoreLibrary.putLong(buffer, index, addressForNames);
        if (functions.get(name) == null) {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG, 0);
        } else {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG, callAsFunction);
        }
        buffer.putInt(index +LONG2, attributes.get(name));
    }

    private void updateLast(int last) {
        int index = CHUNK * last;
        JavaScriptCoreLibrary.putLong(buffer, index, NULL);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG, NULL);
        buffer.putInt(index+LONG2, 0);
    }

    public void dispose() {
        functionPointers.clear();
        bufferTemplate.clear();
        bufferTemplate = null;
        buffer.clear();
        buffer = null;
        names  = null;
        functions = null;
        attributes = null;
        JavaScriptCoreLibrary.NativeReleasePointers(addressForNames);
    }

    public int size() {
        return names.size();
    }
    
    public void add(String name, JSObjectCallAsFunctionCallback callback, int attrs) {
        if (frozen) throw new JavaScriptException("No changes can be done after commit()");
        if (callback != null) functions.put(name, callback);
        attributes.put(name, attrs);
        names.add(name);
    }

    private HashMap<Long, ArrayList<Long>> functionPointers = new HashMap<Long, ArrayList<Long>>();
    public void registerFunctions(long object, long[] pointers) {
        removeObject(object);
        ArrayList<Long> funcs = new ArrayList<Long>();
        for (int i = 0; i < pointers.length; i++) {
            funcs.add(pointers[i]);
        }
        functionPointers.put(object, funcs);       
    }
    public void removeObject(long object) {
        functionPointers.remove(object);
    }

    public JSObjectCallAsFunctionCallback getFunction(long object, long pointer) {
        if (!functionPointers.containsKey(object)) return null;
        int index = functionPointers.get(object).indexOf(pointer);
        if (index < 0) return null;
        return functions.get(names.get(index));
    }

    private static native ByteBuffer NativeGetStaticFunctionTemplate();

}