package com.appcelerator.javascriptcore.opaquetypes;

import java.nio.ByteOrder;
import java.nio.ByteBuffer;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;

public class JSValueArrayRef {
    public static final short sizeOfPointer = JavaScriptCoreLibrary.SizeOfLong;
    public static final ByteOrder nativeOrder = ByteOrder.nativeOrder();

    public static final JSValueArrayRef EMPTY = new JSValueArrayRef(0);

    private int length; 
    private ByteBuffer buffer;

    public JSValueArrayRef(int length, ByteBuffer buffer) {
        this.length  = length;
        if (buffer != null) {
            this.buffer  = buffer.order(nativeOrder);
        }
    }

    public JSValueArrayRef(int length) {
        this.length  = length;

        if (length <= 0) {
            this.buffer = null;
        } else {
            this.buffer = ByteBuffer.allocateDirect(getByteIndex(length)).order(nativeOrder);
        }
    }

    private int getByteIndex(int index) {
        return index * sizeOfPointer;
    }

    public JSValueRef get(JSContextRef context, int index) {
        if (index >= length) throw new IndexOutOfBoundsException(String.format("JSValueArrayRef index=%d count=%d", index, length));
        return new JSValueRef(context, buffer.getLong(getByteIndex(index)));
    }

    public void set(int index, JSValueRef value) {
        if (index >= length) throw new IndexOutOfBoundsException(String.format("JSValueArrayRef index=%d count=%d", index, length));
        this.buffer.putLong(getByteIndex(index), value.p());
    }

    public int length() {
        return this.length;
    }

    public ByteBuffer getByteBuffer() {
        return this.buffer;
    }

    public void dispose() {
        if (this == EMPTY) return;

        if (this.buffer != null) {
            this.buffer.clear();
        }

        this.buffer  = null;
        this.length  = 0;
    }

}