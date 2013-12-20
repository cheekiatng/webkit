package com.appcelerator.javascriptcore.opaquetypes;

public abstract class PointerType {

    protected Pointer pointer;

    /** The default constructor wraps a NULL pointer. */
    protected PointerType() {
        this.pointer = Pointer.NULL;
    }

    protected PointerType(long l) {
        this.pointer = new Pointer(l);
    }
    
    protected PointerType(Pointer p) {
        this.pointer = p;
    }   

    /** Returns the associated native {@link Pointer}. */
    public Pointer getPointer() {
        return pointer;
    }
    
    public void setPointer(Pointer p) {
        this.pointer = p;
    }

    public long pointer() {
        if (pointer == Pointer.NULL) return 0;
        return pointer.value();
    }

    public long p() {
        return pointer();
    }

    public boolean isNullPointer(){
        return pointer() == 0;
    }

    /** The hash code for a <code>PointerType</code> is the same as that for
     * its pointer.
     */
    public int hashCode() {
        return pointer != null ? pointer.hashCode() : 0;
    }
    
    /** Instances of <code>PointerType</code> with identical pointers compare
     * equal by default.
     */
    public boolean equals(Object o) {
        if (o == this) return true;
        if (o instanceof PointerType) {
            Pointer p = ((PointerType)o).getPointer();
            if (pointer == null)
                return p == null;
            return pointer.equals(p);
        }
        return false;
    }

    public String toString() {
        return pointer == null ? "NULL" : pointer.toString() + " (" + super.toString() + ")";
    }
}