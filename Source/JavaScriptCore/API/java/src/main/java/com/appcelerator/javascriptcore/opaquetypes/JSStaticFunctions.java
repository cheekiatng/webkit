package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

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

    private ArrayList<String> names = new ArrayList<String>();
    private ArrayList<Long> functionPointers = new ArrayList<Long>();
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
            callAsFunction = bufferTemplate.getLong(LONG);
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
        buffer.putLong(index, addressForNames);
        buffer.putLong(index+LONG,  callAsFunction);
        buffer.putInt(index +LONG2, attributes.get(name));
    }

    private void updateLast(int last) {
        int index = CHUNK * last;
        buffer.putLong(index, NULL);
        buffer.putLong(index+LONG, NULL);
        buffer.putInt(index+LONG2, 0);
    }

    public void dispose() {
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

    public boolean contains(long func) {
        return functionPointers.contains(func);
    }

    public void registerFunctions(long[] pointers) {
        functionPointers.clear();
        if (pointers.length <= 1) return;
        for (int i = 1; i < pointers.length; i++) {
            functionPointers.add(pointers[i]);
        }
    }

    public JSObjectCallAsFunctionCallback getFunction(long func) {
        return functions.get(names.get(functionPointers.indexOf(func)));
    }

    public void add(String name, JSObjectCallAsFunctionCallback callback, int attrs) {
        if (frozen) throw new JavaScriptException("No changes can be done after commit()");
        if (callback != null) functions.put(name, callback);
        attributes.put(name, attrs);
        names.add(name);
    }

    private static native ByteBuffer NativeGetStaticFunctionTemplate();

}