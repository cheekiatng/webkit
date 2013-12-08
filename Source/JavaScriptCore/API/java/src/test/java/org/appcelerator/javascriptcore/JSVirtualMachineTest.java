package com.appcelerator.javascriptcore;

import com.appcelerator.javascriptcore.opaquetypes.JSContextGroupRef;
import com.appcelerator.javascriptcore.opaquetypes.JSContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSGlobalContextRef;
import com.appcelerator.javascriptcore.opaquetypes.JSValueRef;
import com.appcelerator.javascriptcore.opaquetypes.JSObjectRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassRef;
import com.appcelerator.javascriptcore.opaquetypes.JSClassDefinition;
import com.appcelerator.javascriptcore.opaquetypes.JSStaticValue;
import com.appcelerator.javascriptcore.opaquetypes.JSStaticFunction;
import com.appcelerator.javascriptcore.opaquetypes.Pointer;

import com.appcelerator.javascriptcore.enums.JSPropertyAttribute;

import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsConstructorCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectCallAsFunctionCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectConvertToTypeCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectDeletePropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectFinalizeCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyNamesCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectHasInstanceCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectHasPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectInitializeCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectSetPropertyCallback;
import com.appcelerator.javascriptcore.callbacks.JSObjectGetPropertyCallback;

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
    public void testEvaluateInvalidScriptSyntax() {
        JSGlobalContextRef context = vm.getDefaultContext();
        boolean errorThrown = false;
        try {
            context.evaluateScript("{#@%){");
        } catch (JavaScriptException e) {
            errorThrown = true;
            assertTrue(e.getMessage().length() > 0);
        }
        assertTrue(errorThrown);
    }

    @Test
    public void testJSCheckScriptSyntax() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef result = context.checkScriptSyntax("var a = 0;");
        assertTrue(result.isBoolean() && result.toBoolean());
    }

    @Test
    public void testJSCheckInvalidScriptSyntax() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef result = context.checkScriptSyntax("{#@%){");
        assertFalse(result.isBoolean() && result.toBoolean());
        assertTrue(result.toString().length() > 0);
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

    @Test
    public void testJSClassCreate() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        assertTrue(jsClass.pointer() != 0);
    }

    @Test
    public void testJSObjectMake() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsObj = jsc.JSObjectMake(context, jsClass);
        assertTrue(jsObj.pointer() != 0);
    }

    @Test
    public void testJSObjectMakeInitializeCallback() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        definition.initialize = new JSObjectInitializeCallback() {
            public void apply(JSContextRef ctx, JSObjectRef object) {
                assertTrue(ctx.pointer() != 0);
                assertTrue(object.pointer() != 0);
            }
        };
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsObj = jsc.JSObjectMake(context, jsClass);
        assertTrue(jsObj.pointer() != 0);
    }

    @Test
    public void testJSObjectMakeFinalizeCallback() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        definition.finalize = new JSObjectFinalizeCallback() {
            public void apply(JSObjectRef object) {
                assertTrue(object.pointer() != 0);
            }
        };
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsObj = jsc.JSObjectMake(context, jsClass);
        assertTrue(jsObj.pointer() != 0);
    }

    @Test
    public void testJSObjectCallAsConstructorCallback() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef[] argv = {
            jsc.JSValueMakeNumber(context, 1),
            jsc.JSValueMakeNumber(context, 2),
            jsc.JSValueMakeNumber(context, 3)
        };
        JSClassDefinition definition = new JSClassDefinition();
        definition.callAsConstructor = new JSObjectCallAsConstructorCallback() {
            public JSObjectRef apply(JSContextRef ctx, JSObjectRef constructor,
                                    int argumentCount, JSValueRef[] arguments, 
                                    JSValueRef exception) {
                assertTrue(ctx.p() != 0);
                assertTrue(constructor.p() != 0);
                assertTrue(argumentCount == 3);
                assertTrue(arguments[0].toNumber() == 1);
                assertTrue(arguments[1].toNumber() == 2);
                assertTrue(arguments[2].toNumber() == 3);

                return constructor;
            }
        };
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsObj = jsc.JSObjectMake(context, jsClass);
        JSObjectRef value = jsc.JSObjectCallAsConstructor(context, jsObj, argv);
        assertTrue(value.p() != 0);
    }

    @Test
    public void testJSObjectCallAsFunctionCallback() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSValueRef[] argv = {
            jsc.JSValueMakeNumber(context, 1),
            jsc.JSValueMakeNumber(context, 2),
            jsc.JSValueMakeNumber(context, 3)
        };
        JSClassDefinition definition = new JSClassDefinition();
        definition.callAsFunction = new JSObjectCallAsFunctionCallback() {
            public JSValueRef apply(JSContextRef ctx, JSObjectRef function,
                        JSObjectRef thisObject, int argumentCount,
                        JSValueRef[] arguments, JSValueRef exception) {
                assertTrue(ctx.p() != 0);
                assertTrue(function.p() != 0);
                assertTrue(argumentCount == 3);
                assertTrue(arguments[0].toNumber() == 1);
                assertTrue(arguments[1].toNumber() == 2);
                assertTrue(arguments[2].toNumber() == 3);
                return jsc.JSValueMakeNumber(ctx, 111);
            }
        };
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsObj = jsc.JSObjectMake(context, jsClass);
        JSValueRef value = jsc.JSObjectCallAsFunction(context, jsObj, jsObj, argv);
        assertTrue(value.p() != 0);
        assertTrue(value.toInt() == 111);
    }

    @Test
    public void testJSObjectGetProperty() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        JSStaticValue[] staticValues = new JSStaticValue[1];
        JSStaticValue property1 = new JSStaticValue();
        property1.name = "property1";
        property1.getProperty = new JSObjectGetPropertyCallback() {
            public JSValueRef apply(JSContextRef ctx, JSObjectRef object,
                                    String propertyName, JSValueRef exception) {
                assertTrue(ctx.p() != 0);
                assertTrue(object.p() != 0);
                assertTrue("property1".equals(propertyName));
                return jsc.JSValueMakeNumber(ctx, 123);
            }
        };
        staticValues[0] = property1;

        definition.staticValues = staticValues;

        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsObj = jsc.JSObjectMake(context, jsClass);
        JSValueRef value = jsc.JSObjectGetProperty(context, jsObj, "property1");
        assert(value.toInt() == 123);
    }

    @Test
    public void testJSObjectSetProperty() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        JSStaticValue[] staticValues = new JSStaticValue[1];
        JSStaticValue property1 = new JSStaticValue();
        property1.name = "property1";
        property1.setProperty = new JSObjectSetPropertyCallback() {
            public boolean apply(JSContextRef ctx, JSObjectRef object,
                        String propertyName, JSValueRef value, JSValueRef exception) {
                assertTrue(ctx.p() != 0);
                assertTrue(object.p() != 0);
                assertTrue(value.p() != 0);
                assertTrue("property1".equals(propertyName));
                assertTrue(value.toInt() == 100);
                return true;
            }
        };
        staticValues[0] = property1;

        definition.staticValues = staticValues;

        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsObj = jsc.JSObjectMake(context, jsClass);
        jsc.JSObjectSetProperty(context, jsObj, "property1",
                        jsc.JSValueMakeNumber(context, 100),
                        JSPropertyAttribute.None.getValue() | JSPropertyAttribute.DontDelete.getValue());
    }

    @Test
    public void testJSObjectGetPrivate() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsobj = jsc.JSObjectMake(context, jsClass);
        Object obj = jsc.JSObjectGetPrivate(jsobj);
        assertTrue(obj == null);
    }

    @Test
    public void testJSObjectSetPrivateObject() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsobj = jsc.JSObjectMake(context, jsClass);
        Object obj = jsc.JSObjectGetPrivate(jsobj);
        assertTrue(obj == null);
        Object pObj = new Object();
        assertTrue(jsc.JSObjectSetPrivate(jsobj, pObj));
        Object tObj = jsc.JSObjectGetPrivate(jsobj);
        assertTrue(tObj != null);
        assertTrue(tObj.equals(pObj));
    }

    @Test
    public void testJSObjectSetPrivateString() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSClassDefinition definition = new JSClassDefinition();
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef jsobj = jsc.JSObjectMake(context, jsClass);
        Object obj = jsc.JSObjectGetPrivate(jsobj);
        assertTrue(obj == null);
        String pObj = "This is String Object";
        assertTrue(jsc.JSObjectSetPrivate(jsobj, pObj));
        Object tObj = jsc.JSObjectGetPrivate(jsobj);
        assertTrue(tObj != null);
        assertTrue(tObj.equals(pObj));
    }

    @Test
    public void testJSObjectMakeFunctionWithCallback() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSObjectRef globalObject = jsc.JSContextGetGlobalObject(context);
        JSObjectRef println = jsc.JSObjectMakeFunctionWithCallback(context,
                                "println", new JSObjectCallAsFunctionCallback() {
            public JSValueRef apply(JSContextRef ctx, JSObjectRef function,
                        JSObjectRef thisObject, int argumentCount,
                        JSValueRef[] arguments, JSValueRef exception) {
                assertTrue(function.p() != 0);
                assertTrue(argumentCount == 1);
                assertTrue("Hello, World".equals(arguments[0].toString()));
                return jsc.JSValueMakeNumber(ctx, 1234);
            }
        });
        assertTrue(println.p() != 0);
        jsc.JSObjectSetProperty(context, globalObject, "println", println, JSPropertyAttribute.None.getValue());
        JSValueRef value = context.evaluateScript("println('Hello, World');");
        assertTrue(value.p() != 0);
        assertTrue(value.toInt() == 1234);
    }

    @Test
    public void testJSObjectMakeConstructor() {
        JSGlobalContextRef context = vm.getDefaultContext();
        JSObjectRef globalObject = jsc.JSContextGetGlobalObject(context);
        JSClassDefinition definition = new JSClassDefinition();
        JSClassRef jsClass = jsc.JSClassCreate(definition);
        JSObjectRef constructor = jsc.JSObjectMakeConstructor(context, jsClass, new JSObjectCallAsConstructorCallback() {
            public JSObjectRef apply(JSContextRef ctx, JSObjectRef constructor,
                                    int argumentCount, JSValueRef[] arguments, JSValueRef exception) {
                assertTrue(constructor.p() != 0);
                assertTrue(argumentCount == 1);
                assertTrue("Hello, World".equals(arguments[0].toString()));
                return constructor;
            }
        });
        jsc.JSObjectSetProperty(context, globalObject, "TestObject", constructor, JSPropertyAttribute.None.getValue());
        JSValueRef value = context.evaluateScript("new TestObject('Hello, World');");
        assertTrue(value.p() != 0);
        assertTrue(value.isObject());
    }
}
