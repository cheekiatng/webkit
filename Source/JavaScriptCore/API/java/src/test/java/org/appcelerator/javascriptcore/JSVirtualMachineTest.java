package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSContextGroupRef;
import com.appcelerator.javascriptcore.opaquetypes.JSGlobalContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;

import static org.junit.Assert.*;

import org.junit.Test;
import org.junit.Before;
import org.junit.After;

public class JSVirtualMachineTest {

    private JavaScriptCoreLibrary jsc = JavaScriptCoreLibrary.getInstance();
    private JSVirtualMachine vm;

    @Before
    public void setUp() throws Exception {
        vm = new JSVirtualMachine();
    }

    @After
    public void tearDown() throws Exception {
        vm.release();
    }

    @Test
    public void testContextGroupRefNotNull() {
        assertTrue(vm.getContextGroupRef() != null);
        assertTrue(vm.getContextGroupRef().getPointer() != Pointer.NULL);
    }

    @Test
    public void testJSContextGetGroup() {
        JSGlobalContextRef context  = vm.getDefaultContext();
        JSContextGroupRef group = jsc.JSContextGetGroup(context);
        assertTrue(vm.getContextGroupRef().equals(group));
    }

    @Test
    public void testCreateContext() {
        JSGlobalContextRef context = vm.createContext();
        assertTrue(context != null);
        assertTrue(context.getPointer() != Pointer.NULL);
        vm.releaseContext(context);
    }

    @Test
    public void testReleaseContext() {
        int count = vm.getContextCount();
        JSGlobalContextRef context = vm.createContext();
        vm.releaseContext(context);
        assertTrue(vm.getContextCount() == count);
    }

    @Test
    public void testRetainReleaseContext() {
        JSGlobalContextRef context1 = vm.createContext();
        JSGlobalContextRef context2 = jsc.JSGlobalContextRetain(context1);
        assertTrue(context1.equals(context2));
        jsc.JSGlobalContextRelease(context2);
        vm.releaseContext(context1);
    }

    @Test
    public void testRetainReleaseContextGroup() {
        JSContextGroupRef group1 = vm.getContextGroupRef();
        JSContextGroupRef group2 = jsc.JSContextGroupRetain(group1);
        assertTrue(group1.equals(group2));
        jsc.JSContextGroupRelease(group2);
    }

    @Test
    public void testEvaluateScriptForUndefined() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("undefined");
        assertTrue(value.isUndefined());
    }

    @Test
    public void testEvaluateScriptForNull() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("null");
        assertTrue(value.isNull());
    }

    @Test
    public void testEvaluateScriptForNumber() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("5+5");
        assertTrue(value.isNumber());
        assertTrue(value.toDouble() == 10.0);
        assertTrue(value.toInt() == 10);
    }

    @Test
    public void testEvaluateScriptForDouble() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("1.1");
        assertTrue(value.isNumber());
        assertTrue(value.toDouble() == 1.1);
    }

    @Test
    public void testEvaluateScriptForString() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("'Hello'+'World'");
        assertTrue(value.isString());
        assertTrue("HelloWorld".equals(value.toString()));
    }

    @Test
    public void testEvaluateScriptForObject() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("new Date();");
        assertTrue(value.isObject());
        assertTrue(value.toObject() instanceof JSObjectRef);
    }

    @Test
    public void testJSCheckScriptSyntax() {
        JSGlobalContextRef context = vm.getDefaultContext();
        assertTrue(context.checkScriptSyntax("var a = 0;"));
    }

    @Test
    public void testJSCheckInvalidScriptSyntax() {
        JSGlobalContextRef context = vm.getDefaultContext();
        assertFalse(context.checkScriptSyntax("{#@%){"));
    }

    @Test
    public void testJSValueToJSON() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("[1,2,3]");
        assertTrue("[1,2,3]".equals(value.toJSON(0)));
    }

    @Test
    public void testJSValueToBoolean() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("false");
        assertFalse(value.toBoolean());
    }

    @Test
    public void testJSValueIsEqual() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef valueA = context.evaluateScript("3");
        JSValueRef valueB = context.evaluateScript("'3'");
        assertTrue(valueA.isEqual(valueB));
    }

    @Test
    public void testJSValueIsStrictEqual() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef valueA = context.evaluateScript("3");
        JSValueRef valueB = context.evaluateScript("'3'");
        assertFalse(valueA.isStrictEqual(valueB));
    }

    @Test
    public void testJSValueProtectCall() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("new Date();");
        value.protect();
        value.unprotect();
        // Can't test it actually.
        // at least it assures it doesn't throw error
        assertTrue(value.isObject());
    }

    @Test
    public void testJSValueGCCall() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = context.evaluateScript("new Date();");
        context.garbageCollect();
        // Can't test it actually.
        // at least it assures it doesn't throw error
        assertTrue(value.isObject());
    }

    @Test
    public void testJSValueMakeBoolean() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = jsc.JSValueMakeBoolean(context, false);
        assertTrue(value.isBoolean());
        assertFalse(value.toBoolean());
    }

    @Test
    public void testJSValueMakeNull() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = jsc.JSValueMakeNull(context);
        assertTrue(value.isNull());
    }

    @Test
    public void testJSValueMakeUndefined() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = jsc.JSValueMakeUndefined(context);
        assertTrue(value.isUndefined());
    }

    @Test
    public void testJSValueMakeNumber() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = jsc.JSValueMakeNumber(context, 10);
        assertTrue(value.isNumber());
        assertTrue(value.toNumber() == 10.0);
    }

    @Test
    public void testJSValueMakeString() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = jsc.JSValueMakeString(context, "Lorem ipsum dolor sit amet");
        assertTrue(value.isString());
        assertTrue("Lorem ipsum dolor sit amet".equals(value.toString()));
    }

    @Test
    public void testJSValueMakeFromJSONString() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef value = jsc.JSValueMakeFromJSONString(context, "'Lorem ipsum dolor sit amet'");
        assertTrue(value.isString());
        assertTrue("'Lorem ipsum dolor sit amet'".equals(value.toString()));
    }

}
