package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.JavaScriptCoreLibrary;
import com.appcelerator.javascriptcore.opaquetypes.JSContextGroupRef;
import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSGlobalContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassRef;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class JSVirtualMachine {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();

    private JSContextGroupRef contextGroupRef;
    private JSGlobalContextRef defaultContext;
    private ConcurrentMap<Long, JSGlobalContextRef> contextCache = new ConcurrentHashMap<Long, JSGlobalContextRef>();

    public JSVirtualMachine() {
        contextGroupRef = jsc.JSContextGroupCreate();
        defaultContext  = createContext();
    }

    public JSGlobalContextRef createContext() {
        JSGlobalContextRef context = jsc.JSGlobalContextCreateInGroup(getContextGroupRef(), new JSClassRef());
        contextCache.put(context.pointer(), context);
        return context;
    }

    public void releaseContext(JSGlobalContextRef context) {
        jsc.JSGlobalContextRelease(context);
        contextCache.remove(context.pointer());
    }

    public void releaseContexts() {
        for (JSGlobalContextRef context : contextCache.values()) {
            jsc.JSGlobalContextRelease(context);
        }
        contextCache.clear();
    }

    public JSGlobalContextRef getDefaultContext() {
        return defaultContext;
    }

    public int getContextCount() {
        return contextCache.size();
    }

    public JSContextGroupRef getContextGroupRef() {
        return contextGroupRef;
    }

    public void release() {
        releaseContexts();
        jsc.JSContextGroupRelease(contextGroupRef);
    }

}
