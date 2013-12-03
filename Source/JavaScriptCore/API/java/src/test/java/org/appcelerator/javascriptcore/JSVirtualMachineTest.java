package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSContextGroupRef;
import com.appcelerator.javascriptcore.opaquetypes.JSGlobalContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;

import static org.junit.Assert.*;

import org.junit.Test;
import org.junit.Before;
import org.junit.After;

public class JSVirtualMachineTest {

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
}
