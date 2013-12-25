package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import java.util.HashMap;
import java.util.ArrayList;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.JavaScriptException;
import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectSetPropertyCallback;

public class JSStaticValues {
    
    private static final long  NULL  = 0;
    private static final short LONG  = JavaScriptCoreLibrary.SizeOfLong;
    private static final short CHUNK = JavaScriptCoreLibrary.SizeOfJSStaticValue;
    private static final short LONG2 = (short)(LONG * 2);
    private static final short LONG3 = (short)(LONG * 3);

    private static final ByteOrder nativeOrder = ByteOrder.nativeOrder();
    private static ByteBuffer bufferTemplate = null;
    private static long getterFunction;
    private static long setterFunction;

    private ByteBuffer buffer = null;
    private long[] addressForNames;
    private boolean frozen = false;
    private HashMap<String, JSStaticValue> values = new HashMap<String, JSStaticValue>();
    private ArrayList<String> namesCache = new ArrayList<String>();

    public JSStaticValues() {
        if (bufferTemplate == null) {
            bufferTemplate = NativeGetStaticValueTemplate().order(nativeOrder);
            getterFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, LONG);
            setterFunction = JavaScriptCoreLibrary.getLong(bufferTemplate, LONG2);
        }
    }

    /*
     * Commit the changes.
     * Note that buffer allocation is done only once.
     */
    public ByteBuffer commit() {
        if (!frozen) {
            int size = namesCache.size();
            buffer = ByteBuffer.allocateDirect(CHUNK * (size + 1)).order(nativeOrder);
            addressForNames = JavaScriptCoreLibrary.NativeAllocateCharacterBuffer(namesCache.toArray(new String[size]));
            for (int i = 0; i < size; i++) {
                update(namesCache.get(i), i * CHUNK, addressForNames[i]);
            }
            updateLast(size);
            frozen = true;
            namesCache.clear(); // won't use this after commit
        }
        return buffer;
    }

    public void dispose() {
        bufferTemplate.clear();
        bufferTemplate = null;
        if (buffer != null) buffer.clear();
        buffer = null;
        namesCache = null;
        values = null;
        if (addressForNames != null) JavaScriptCoreLibrary.NativeReleasePointers(addressForNames);
    }

    private void update(String name, int index, long addressForNames) {
        JavaScriptCoreLibrary.putLong(buffer, index, addressForNames);
        if (values.get(name).getter == null) {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG, 0);
        } else {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG, getterFunction);
        }
        if (values.get(name).setter == null) {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG2, 0);
        } else {
            JavaScriptCoreLibrary.putLong(buffer, index+LONG2, setterFunction);
        }
        buffer.putInt(index +LONG3, values.get(name).attributes);
    }

    private void updateLast(int last) {
        int index = CHUNK * last;
        JavaScriptCoreLibrary.putLong(buffer, index, NULL);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG,  NULL);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG2, NULL);
        buffer.putInt(index +LONG3, 0);
    }

    public boolean containsGetter(String name) {
        return values.containsKey(name);
    }

    public boolean containsSetter(String name) {
        return values.containsKey(name);
    }

    public void add(String name, JSObjectGetPropertyCallback getter, JSObjectSetPropertyCallback setter, int attrs) {
        if (frozen) {
            throw new JavaScriptException("No changes can be done after commit()");
        }
        values.put(name, new JSStaticValue(getter, setter, attrs));
        namesCache.add(name);
    }

    public JSObjectGetPropertyCallback getGetPropertyCallback(String name) {
        return values.get(name).getter;

    }

    public JSObjectSetPropertyCallback getSetPropertyCallback(String name) {
        return values.get(name).setter;
    }


    private class JSStaticValue {
        public JSObjectGetPropertyCallback getter;
        public JSObjectSetPropertyCallback setter;
        public int attributes;
        public JSStaticValue(JSObjectGetPropertyCallback getter,
                             JSObjectSetPropertyCallback setter, int attributes) {
            this.getter = getter;
            this.setter = setter;
            this.attributes = attributes;
        }
    }

    private static native ByteBuffer NativeGetStaticValueTemplate();

}
