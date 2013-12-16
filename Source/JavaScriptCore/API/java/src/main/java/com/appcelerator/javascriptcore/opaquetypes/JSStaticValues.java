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

    private ArrayList<String> names = new ArrayList<String>();
    private HashMap<String, JSObjectGetPropertyCallback> getters = new HashMap<String, JSObjectGetPropertyCallback>();
    private HashMap<String, JSObjectSetPropertyCallback> setters = new HashMap<String, JSObjectSetPropertyCallback>();
    private HashMap<String, Integer> attributes = new HashMap<String, Integer>();

    private boolean frozen = false;

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

    public void dispose() {
        bufferTemplate.clear();
        bufferTemplate = null;
        buffer.clear();
        buffer = null;
        names = null;
        getters = null;
        setters = null;
        attributes = null;
        JavaScriptCoreLibrary.NativeReleasePointers(addressForNames);
    }

    private void update(String name, int index, long addressForNames) {
        JavaScriptCoreLibrary.putLong(buffer, index, addressForNames);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG,  getterFunction);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG2, setterFunction);
        buffer.putInt(index +LONG3, attributes.get(name));
    }

    private void updateLast(int last) {
        int index = CHUNK * last;
        JavaScriptCoreLibrary.putLong(buffer, index, NULL);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG,  NULL);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG2, NULL);
        buffer.putInt(index +LONG3, 0);
    }

    public boolean containsGetter(String name) {
        return getters.containsKey(name);
    }

    public boolean containsSetter(String name) {
        return setters.containsKey(name);
    }

    public void add(String name, JSObjectGetPropertyCallback getter, JSObjectSetPropertyCallback setter, int attrs) {
        if (frozen) throw new JavaScriptException("No changes can be done after commit()");
        if (getter != null) getters.put(name, getter);
        if (setter != null) setters.put(name, setter);
        attributes.put(name, attrs);
        names.add(name);
    }

    public JSObjectGetPropertyCallback getGetPropertyCallback(String name) {
        return getters.get(name);

    }

    public JSObjectSetPropertyCallback getSetPropertyCallback(String name) {
        return setters.get(name);
    }

    private static native ByteBuffer NativeGetStaticValueTemplate();

}
