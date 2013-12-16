package com.appcelerator.javascriptcore.opaquetypes;

public class Pointer {

    /** Convenience constant, same as <code>null</code>. */
    public static final Pointer NULL = null;

    /** Pointer value of the real native pointer. Use long to be 64-bit safe. */
    protected long peer;

    public Pointer(long peer) {
        this.peer = peer;
    }

    public long value() {
        return this.peer;
    }

    public boolean equals(Object o) {
        if (o == this) return true;
        if (o == null) return false;
        return o instanceof Pointer && ((Pointer)o).peer == peer;
    }

    public int hashCode() {
        return (int)((peer >>> 32) + (peer & 0xFFFFFFFF));
    }
    
    public String toString() {
        return "native@0x" + Long.toHexString(peer);
    }
}