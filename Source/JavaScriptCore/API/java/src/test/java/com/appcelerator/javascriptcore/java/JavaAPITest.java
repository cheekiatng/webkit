package com.appcelerator.javascriptcore.java;

import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.StringWriter;
import java.io.PrintWriter;

import com.appcelerator.javascriptcore.*;
import com.appcelerator.javascriptcore.opaquetypes.*;


import static org.junit.Assert.*;

import org.junit.Test;
import org.junit.Before;
import org.junit.After;

public class JavaAPITest {

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
    public void testJavaString() {
        JSGlobalContextRef context  = vm.getDefaultContext();
        JSObjectRef globalObject = jsc.JSContextGetGlobalObject(context);
        JSObjectRef parentObject = JSJavaObjectUtil.registerJSNamespace(context, globalObject, JS_java_lang_Object.getNamespace());
        assertTrue(JS_java_lang_Object.registerClass(context, parentObject));
        assertTrue(JS_java_lang_String.registerClass(context, parentObject));

        JSValueRef exception = JSValueRef.Null();
        JSValueRef result = context.evaluateScript("java.lang.String.format('String equals %s and %s', 'Hello', 'World');", globalObject, exception);
        assertTrue(jsc.JSValueIsNull(context, exception));
        assertTrue(jsc.JSValueIsObject(context, result));
        assertTrue("String equals Hello and World".equals(result.toString()));

        exception = JSValueRef.Null();
        result = context.evaluateScript("java.lang.String.format('Hello, World').equals(java.lang.String.format('Hello, World'));", globalObject, exception);
        assertTrue(jsc.JSValueIsNull(context, exception));
        assertTrue(result.toBoolean());

        exception = JSValueRef.Null();
        result = context.evaluateScript("new java.lang.String('Hello, Java String');", globalObject, exception);
        assertTrue(jsc.JSValueIsNull(context, exception));
        assertTrue(jsc.JSValueIsObject(context, result));
        assertTrue("Hello, Java String".equals(result.toString()));

        exception = JSValueRef.Null();
        result = context.evaluateScript("function hello() { return new java.lang.String('Hello, JavaScript function'); }", globalObject, exception);
        assertTrue(jsc.JSValueIsNull(context, exception));

        exception = JSValueRef.Null();
        JSObjectRef hello = jsc.JSObjectGetProperty(context, globalObject, "hello", exception).toObject();
        assertTrue(jsc.JSValueIsNull(context, exception));
        assertTrue(jsc.JSObjectIsFunction(context, hello));

        exception = JSValueRef.Null();
        result = jsc.JSObjectCallAsFunction(context, hello, globalObject, JSValueArrayRef.noArg(), exception);
        assertTrue(jsc.JSValueIsNull(context, exception));
        assertTrue(jsc.JSValueIsObject(context, result));
        assertTrue("Hello, JavaScript function".equals(result.toString()));
    }

    private String createStringWithContentsOfFile(String fileName) {
        String newline = System.getProperty("line.separator");
        StringBuilder sb = new StringBuilder();
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new InputStreamReader(this.getClass().getResourceAsStream(fileName), "UTF-8"));
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line).append(newline);
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {if (reader != null) reader.close(); } catch (IOException e) {}
        }
        return sb.toString();
    }
}
