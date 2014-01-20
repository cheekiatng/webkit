package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import java.util.List;
import java.util.HashMap;
import java.util.ArrayList;

import com.appcelerator.javascriptcore.util.LongSparseArray;

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
    private ArrayList<JSStaticFunction> functions = new ArrayList<JSStaticFunction>();

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
                update(i, i * CHUNK, addressForNames[i]);
            }
            updateLast(size);
        }
        return buffer;
    }

    private void update(int index, int bindex, long addressForNames) {
        JavaScriptCoreLibrary.putLong(buffer, bindex, addressForNames);
        JSStaticFunction function = functions.get(index);
        if (function.callback == null) {
            JavaScriptCoreLibrary.putLong(buffer, bindex+LONG, 0);
        } else {
            JavaScriptCoreLibrary.putLong(buffer, bindex+LONG, callAsFunction);
        }
        buffer.putInt(bindex +LONG2, function.attributes);
    }

    private void updateLast(int last) {
        int index = CHUNK * last;
        JavaScriptCoreLibrary.putLong(buffer, index, NULL);
        JavaScriptCoreLibrary.putLong(buffer, index+LONG, NULL);
        buffer.putInt(index+LONG2, 0);
    }

    public void dispose() {
        if (functionPointers != null) {
            functionPointers.clear();
            functionPointers = null;
        }
        if (bufferTemplate != null) {
            bufferTemplate.clear();
            bufferTemplate = null;
        }
        if (buffer != null) {
            buffer.clear();
            buffer = null;
        }
        namesCache  = null;
        functions = null;
        if (addressForNames != null) JavaScriptCoreLibrary.NativeReleasePointers(addressForNames);
    }

    public int size() {
        return functions.size();
    }
    
    public void add(String name, JSObjectCallAsFunctionCallback callback, JSPropertyAttribute attrs) {
        if (buffer != null) throw new JavaScriptException("No changes can be done after commit()");
        functions.add(new JSStaticFunction(callback, attrs.getValue()));
        namesCache.add(name);
    }

    private HashMap<Long, LongSparseArray<Integer>> functionPointers = new HashMap<Long, LongSparseArray<Integer>>(JavaScriptCoreLibrary.numberOfJSObjectBuckets);
    public void registerFunctions(long object, long[] pointers) {
        removeObject(object);
        LongSparseArray<Integer> funcs = new LongSparseArray<Integer>(pointers.length);
        for (int i = 0; i < pointers.length; i++) {
            funcs.put(pointers[i], i);
        }
        functionPointers.put(object, funcs);       
    }
    
    public void removeObject(long object) {
        functionPointers.remove(object);
    }

    public boolean requestFunctions(long object) {
        return functionPointers.get(object) == null;
    }

    public JSObjectCallAsFunctionCallback getFunction(long object, long pointer) {
        if (functionPointers.get(object) == null) return null;
        if (functionPointers.get(object).get(pointer) == null) return null;
        return functions.get(functionPointers.get(object).get(pointer)).callback;
    }

    private static native ByteBuffer NativeGetStaticFunctionTemplate();

}
class JSStaticFunction {
    public JSObjectCallAsFunctionCallback callback;
    public int attributes;
    public JSStaticFunction(JSObjectCallAsFunctionCallback callback, int attributes) {
        this.callback = callback;
        this.attributes = attributes;
    }        
}

