//
//  JavaScriptCoreJNI.c
//  JavaScriptCoreJNI
//
//  Created by Kota Iguchi on 11/30/13.
//  Copyright (c) 2013 Appcelerator, Inc. All rights reserved.
//

#include "JavaScriptCoreForJNI.h"

#ifdef __cplusplus
extern "C" {
#endif

static JavaVM* jvm;
static JSStaticValue JSStaticEmptyValue = {0,0,0,0};
static JSStaticFunction JSStaticEmptyFunction = {0,0,0};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    jvm = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupCreate
    (JNIEnv *env, jobject thiz)
{
    JSContextGroupRef group = JSContextGroupCreate();
    return (jlong)group;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupRelease
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef)
{
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    JSContextGroupRelease(group);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupRetain
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef)
{
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    return (jlong)JSContextGroupRetain(group);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGetGlobalObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSContextGetGlobalObject(ctx);
}
    
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGetGroup
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSContextGetGroup(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextCreate
    (JNIEnv *env, jobject thiz, jlong jsClassRef)
{
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    JSGlobalContextRef ctx = JSGlobalContextCreate(jsClass);
    return (jlong)ctx;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextCreateInGroup
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef, jlong jsClassRef)
{
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    JSGlobalContextRef ctx = JSGlobalContextCreateInGroup(group, jsClass);
    return (jlong)ctx;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextRelease
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSGlobalContextRelease(ctx);
}

JNIEXPORT long JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextRetain
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSGlobalContextRetain(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSEvaluateScript
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring script, jlong exception)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef* jsexception = (JSValueRef*)&exception;
    
    const char* scriptchars = (*env)->GetStringUTFChars(env, script, NULL);
    JSStringRef scriptJS = JSStringCreateWithUTF8CString(scriptchars);
    (*env)->ReleaseStringUTFChars(env, script, scriptchars);
    
    return (jlong)JSEvaluateScript(ctx, scriptJS, NULL, NULL, 0, jsexception);
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSCheckScriptSyntax
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring script, jlong exception)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef* jsexception = (JSValueRef*)&exception;
    
    const char* scriptchars = (*env)->GetStringUTFChars(env, script, NULL);
    JSStringRef scriptJS = JSStringCreateWithUTF8CString(scriptchars);
    (*env)->ReleaseStringUTFChars(env, script, scriptchars);
    
    return JSCheckScriptSyntax(ctx, scriptJS, NULL, 0, jsexception) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGarbageCollect
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSGarbageCollect(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeUndefined
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeUndefined(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeNull
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeNull(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jdouble arg)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeNumber(ctx, (double)arg);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jboolean arg)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeBoolean(ctx, arg == JNI_TRUE ? true : false);
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsObjectOfClass
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsClassRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    return JSValueIsObjectOfClass(ctx, value, jsClass) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsInstanceOfConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsObjectRef, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSObjectRef constructor = (JSObjectRef)jsObjectRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    return JSValueIsInstanceOfConstructor(ctx, value, constructor, exception) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsUndefined
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsUndefined(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsNull
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsNull(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsNumber(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsBoolean(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsString(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsObject(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    return JSValueToBoolean(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jdouble JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    
    return JSValueToNumber(ctx, value, exception);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    
    return (jlong)JSValueToObject(ctx, value, exception);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeFromJSONString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring jjson)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    
    const char* jsonchars = (*env)->GetStringUTFChars(env, jjson, NULL);
    JSStringRef json = JSStringCreateWithUTF8CString(jsonchars);
    (*env)->ReleaseStringUTFChars(env, jjson, jsonchars);
    
    return (jlong)JSValueMakeFromJSONString(ctx, json);
}

JNIEXPORT jstring JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToStringCopy
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    JSStringRef jsstring = JSValueToStringCopy(ctx, value, exception);
    
    if (!JSValueIsNull(ctx, *exception))
    {
        return NULL;
    }

    size_t length = JSStringGetMaximumUTF8CStringSize(jsstring);
    char* aschars = (char*)malloc(length);
    JSStringGetUTF8CString(jsstring, aschars, length);
    JSStringRelease(jsstring);
    
    jstring copy = (*env)->NewStringUTF(env, aschars);
    free(aschars);
    return copy;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsEqual
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRefA, jlong jsValueRefB, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef a = (JSValueRef)jsValueRefA;
    JSValueRef b = (JSValueRef)jsValueRefB;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    
    return JSValueIsEqual(ctx, a, b, exception) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsStrictEqual
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRefA, jlong jsValueRefB)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef a = (JSValueRef)jsValueRefA;
    JSValueRef b = (JSValueRef)jsValueRefB;
    
    return JSValueIsStrictEqual(ctx, a, b) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueProtect
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueProtect(ctx, value);
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueUnprotect
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueUnprotect(ctx, value);
}

JNIEXPORT jstring JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueCreateJSONString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jint indent, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    
    JSStringRef jsstring = JSValueCreateJSONString(ctx, value, indent, exception);
    
    if (!JSValueIsNull(ctx, *exception))
    {
        return NULL;
    }
    
    size_t length = JSStringGetMaximumUTF8CStringSize(jsstring);
    char* aschars = (char*)malloc(length);
    JSStringGetUTF8CString(jsstring, aschars, length);
    JSStringRelease(jsstring);
    
    jstring copy = (*env)->NewStringUTF(env, aschars);
    free(aschars);
    return copy;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring value)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSSTRINGREF_FROM_JSTRING(value, jsvalue)
    return (jlong)JSValueMakeString(ctx, jsvalue);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCallAsConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint argc, jlongArray argv, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    
    jlong* p_argv = (*env)->GetLongArrayElements(env, argv, NULL);
    const JSValueRef* js_argv = (JSValueRef*)p_argv;
    JSObjectRef value = JSObjectCallAsConstructor(ctx, object, argc, js_argv, exception);
    (*env)->ReleaseLongArrayElements(env, argv, p_argv, 0);
    
    return (jlong)value;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCallAsFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jlong jsThisObjectRef,
     jint argc, jlongArray argv, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSObjectRef thisObject = (JSObjectRef)jsThisObjectRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    
    jlong* p_argv = (*env)->GetLongArrayElements(env, argv, NULL);
    const JSValueRef* js_argv = (JSValueRef*)p_argv;
    JSValueRef value = JSObjectCallAsFunction(ctx, object, thisObject, argc, js_argv, exception);
    (*env)->ReleaseLongArrayElements(env, argv, p_argv, 0);
    
    return (jlong)value;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring name,
     jlong jsValueRef, jint attributes, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    
    JSObjectSetProperty(ctx, object, jsname, value, attributes, exception);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef,
     jstring name, jlong jsExceptionRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef* exception = (JSValueRef*)&jsExceptionRef;
    JSSTRINGREF_FROM_JSTRING(name, jsname)

    return (jlong)JSObjectGetProperty(ctx, object, jsname, exception);
}

/*
 * JavaScriptCore Callbacks
 */
void NativeCallback_JSObjectInitializeCallback(JSContextRef ctx, JSObjectRef object)
{
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectInitializeCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectInitializeCallback", "(JJ)V");
        (*env)->CallVoidMethod(env, prv->definition,
                               jmethodId_JSObjectInitializeCallback, (jlong)ctx, (jlong)object);
    }
    JNI_ENV_EXIT
}

void NativeCallback_JSObjectFinalizeCallback(JSObjectRef object)
{
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv)
    {
        if (prv->definition)
        {
            jmethodID jmethodId_JSObjectFinalizeCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectFinalizeCallback", "(J)V");
            (*env)->CallVoidMethod(env, prv->definition,
                                   jmethodId_JSObjectFinalizeCallback, (jlong)object);
            (*env)->DeleteGlobalRef(env, prv->definition);
            (*env)->DeleteGlobalRef(env, prv->definitionClass);
        }
        if (prv->object)
        {
            (*env)->DeleteGlobalRef(env, prv->object);
        }
        free(prv);
        
        JSObjectSetPrivate(object, NULL);
    }
    JNI_ENV_EXIT
}

JSValueRef NativeCallback_JSObjectGetStaticValueCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef* exception)
{
    JSValueRef value = NULL;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectGetStaticValueCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectGetStaticValueCallback", "(JJLjava/lang/String;J)J");
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->definition,
                               jmethodId_JSObjectGetStaticValueCallback,
                               (jlong)ctx, (jlong)object, jname, (jlong)*exception);
        free(cname);
    }
    JNI_ENV_EXIT
    
    return value;
}

bool NativeCallback_JSObjectSetStaticValueCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* exception)
{
    bool result = false;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectSetStaticValueCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectSetStaticValueCallback", "(JJLjava/lang/String;JJ)Z");
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        result = (*env)->CallBooleanMethod(env, prv->definition, jmethodId_JSObjectSetStaticValueCallback,
            (jlong)ctx, (jlong)object, jname, (jlong)value, (jlong)*exception) == JNI_TRUE ? true : false;
        free(cname);
    }
    JNI_ENV_EXIT

    return result;
}
    
JSValueRef NativeCallback_JSObjectGetPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef* exception)
{
    JSValueRef value = NULL;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectGetPropertyCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectGetPropertyCallback", "(JJLjava/lang/String;J)J");
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->definition,
                               jmethodId_JSObjectGetPropertyCallback,
                               (jlong)ctx, (jlong)object, jname, (jlong)*exception);
        free(cname);
    }
    JNI_ENV_EXIT
    
    return value;
}

bool NativeCallback_JSObjectSetPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* exception)
{
    bool result = false;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectSetPropertyCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectSetPropertyCallback", "(JJLjava/lang/String;JJ)Z");
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        result = (*env)->CallBooleanMethod(env, prv->definition, jmethodId_JSObjectSetPropertyCallback,
            (jlong)ctx, (jlong)object, jname, (jlong)value, (jlong)*exception) == JNI_TRUE ? true : false;
        free(cname);
    }
    JNI_ENV_EXIT

    return result;
}

JSObjectRef NativeCallback_JSObjectCallAsConstructorCallback(
    JSContextRef ctx, JSObjectRef constructor,
    size_t argc, const JSValueRef argv[], JSValueRef *exception)
{
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(constructor);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectCallAsConstructorCallback = (*env)->GetMethodID(
                    env, prv->definitionClass, "JSObjectCallAsConstructorCallback", "(JJI[JJ)J");
        jlongArray j_argv = (*env)->NewLongArray(env, (jsize)argc);
        jlong* p_argv = (*env)->GetLongArrayElements(env, j_argv, NULL);
        for (size_t i = 0; i < argc; i++) {
            p_argv[i] = (jlong)argv[i];
        }
        (*env)->ReleaseLongArrayElements(env, j_argv, p_argv, 0);
        
        constructor = (JSObjectRef)(*env)->CallLongMethod(env, prv->definition,
                            jmethodId_JSObjectCallAsConstructorCallback,
                            (jlong)ctx, (jlong)constructor, (jint)argc, j_argv, (jlong)*exception);
    }
    JNI_ENV_EXIT

    return constructor;
}

JSValueRef NativeCallback_JSObjectCallAsFunctionCallback(
    JSContextRef ctx, JSObjectRef func, JSObjectRef thisObject,
    size_t argc, const JSValueRef argv[], JSValueRef* exception) {
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(thisObject);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectCallAsFunctionCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectCallAsFunctionCallback", "(JJJI[JJ)J");
        jlongArray j_argv = (*env)->NewLongArray(env, (jsize)argc);
        jlong* p_argv = (*env)->GetLongArrayElements(env, j_argv, NULL);
        for (size_t i = 0; i < argc; i++) {
            p_argv[i] = (jlong)argv[i];
        }
        (*env)->ReleaseLongArrayElements(env, j_argv, p_argv, 0);
        
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->definition, jmethodId_JSObjectCallAsFunctionCallback,
                               (jlong)ctx, (jlong)func, (jlong)thisObject, (jint)argc, j_argv, (jlong)*exception);
    }
    JNI_ENV_EXIT
    return value;
}

JSValueRef NativeCallback_JSObjectConvertToTypeCallback(
   JSContextRef ctx,JSObjectRef object,
   JSType type, JSValueRef *exception)
{
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectConvertToTypeCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectConvertToTypeCallback", "(JJIJ)J");
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->definition, jmethodId_JSObjectConvertToTypeCallback,
                                                   (jlong)ctx, (jlong)object, (jint)type, (jlong)*exception);
    }
    JNI_ENV_EXIT
    return value;
}
    
bool NativeCallback_JSObjectDeletePropertyCallback(
    JSContextRef ctx, JSObjectRef object,
    JSStringRef name, JSValueRef *exception)
{
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectDeletePropertyCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectDeletePropertyCallback", "(JJLjava/lang/String;J)Z");
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (*env)->CallBooleanMethod(env, prv->definition, jmethodId_JSObjectDeletePropertyCallback,
                    (jlong)ctx, (jlong)object, jname, (jlong)*exception) == JNI_TRUE ? true : false;
        free(cname);
    }
    JNI_ENV_EXIT
    return value;
}

void NativeCallback_JSObjectGetPropertyNamesCallback(
    JSContextRef ctx, JSObjectRef object,
    JSPropertyNameAccumulatorRef propertyNames)
{
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectGetPropertyNamesCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectGetPropertyNamesCallback", "(JJJ)V");
        (*env)->CallVoidMethod(env, prv->definition, jmethodId_JSObjectGetPropertyNamesCallback,
                               (jlong)ctx, (jlong)object, (jlong)propertyNames);
    }
    JNI_ENV_EXIT
}

bool NativeCallback_JSObjectHasInstanceCallback(
    JSContextRef ctx, JSObjectRef constructor,
    JSValueRef instance, JSValueRef *exception)
{
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(constructor);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectHasInstanceCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectHasInstanceCallback", "(JJJJ)Z");
        value = (*env)->CallBooleanMethod(env, prv->definition, jmethodId_JSObjectHasInstanceCallback,
                    (jlong)ctx, (jlong)constructor, (jlong)instance, (jlong)*exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT
    return value;
}

bool NativeCallback_JSObjectHasPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name)
{
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->definition)
    {
        jmethodID jmethodId_JSObjectHasPropertyCallback = (*env)->GetMethodID(
                env, prv->definitionClass, "JSObjectHasPropertyCallback", "(JJLjava/lang/String;)Z");
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (*env)->CallBooleanMethod(env, prv->definition, jmethodId_JSObjectHasPropertyCallback,
                                (jlong)ctx, (jlong)object, jname) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT
    return value;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassCreate
    (JNIEnv *env, jobject thiz,
     jint version, jint attributes, jstring className, jlong parentClass,
     jobjectArray staticValueNames, jintArray staticValueAttributes,
     jobjectArray staticFunctionNames, jintArray staticFunctionAttributes,
     jboolean initialize, jboolean finalize,
     jboolean hasProperty, jboolean getProperty,
     jboolean setProperty, jboolean deleteProperty,
     jboolean getPropertyNames, jboolean callAsFunction,
     jboolean callAsConstructor, jboolean hasInstance,
     jboolean convertToType)
{
    JSClassDefinition definition = kJSClassDefinitionEmpty;
    definition.version = version;
    definition.attributes = attributes;
    definition.parentClass = (JSClassRef)parentClass;
    
    if (initialize == JNI_TRUE)  definition.initialize = NativeCallback_JSObjectInitializeCallback;
    if (finalize == JNI_TRUE)    definition.finalize = NativeCallback_JSObjectFinalizeCallback;
    if (getProperty == JNI_TRUE) definition.getProperty = NativeCallback_JSObjectGetPropertyCallback;
    if (setProperty == JNI_TRUE)       definition.setProperty = NativeCallback_JSObjectSetPropertyCallback;
    if (deleteProperty == JNI_TRUE)    definition.deleteProperty = NativeCallback_JSObjectDeletePropertyCallback;
    if (getPropertyNames == JNI_TRUE)  definition.getPropertyNames = NativeCallback_JSObjectGetPropertyNamesCallback;
    if (callAsFunction == JNI_TRUE)    definition.callAsFunction = NativeCallback_JSObjectCallAsFunctionCallback;
    if (callAsConstructor == JNI_TRUE) definition.callAsConstructor = NativeCallback_JSObjectCallAsConstructorCallback;
    if (hasInstance == JNI_TRUE)   definition.hasInstance = NativeCallback_JSObjectHasInstanceCallback;
    if (convertToType == JNI_TRUE) definition.convertToType = NativeCallback_JSObjectConvertToTypeCallback;
    
    // JSClassDefinition.className could be null
    const char* classNameAsChars = NULL;
    if (className != NULL)
    {
        classNameAsChars = (*env)->GetStringUTFChars(env, className, NULL);
        definition.className = classNameAsChars;
    }

    // Setup JSClassDefinition.staticValues
    int valueCount = (*env)->GetArrayLength(env, staticValueNames);
    JSStaticValue* values = (JSStaticValue*)malloc(sizeof(JSStaticValue) * (valueCount + 1));
    if (valueCount > 0) {
        jint* attributes = (*env)->GetIntArrayElements(env, staticValueAttributes, 0);
        for (int i = 0; i < valueCount; i++) {
            jobject name = (*env)->GetObjectArrayElement(env, staticValueNames, i);
            const char* cname = (*env)->GetStringUTFChars(env, name, NULL);
            values[i].attributes = attributes[i];
            values[i].name = strdup(cname); // should be freed later
            values[i].getProperty = NativeCallback_JSObjectGetStaticValueCallback;
            values[i].setProperty = NativeCallback_JSObjectSetStaticValueCallback;
            (*env)->ReleaseStringUTFChars(env, name, cname);
            (*env)->DeleteLocalRef(env, name);
        }
        (*env)->ReleaseIntArrayElements(env, staticValueAttributes, attributes, 0);
        definition.staticValues = values;
    }
    values[valueCount] = JSStaticEmptyValue;

    // Setup JSClassDefinition.staticFunctions
    int funcCount  = (*env)->GetArrayLength(env, staticFunctionNames);
    JSStaticFunction* funcs = (JSStaticFunction*)malloc(sizeof(JSStaticFunction) * (funcCount + 1));
    if (funcCount > 0) {
        jint* attributes = (*env)->GetIntArrayElements(env, staticFunctionAttributes, 0);
        
        for (int i = 0; i < funcCount; i++) {
            jobject name = (*env)->GetObjectArrayElement(env, staticFunctionNames, i);
            const char* cname = (*env)->GetStringUTFChars(env, name, NULL);
            funcs[i].attributes = attributes[i];
            funcs[i].name = strdup(cname); // should be freed later
            funcs[i].callAsFunction = NativeCallback_JSObjectCallAsFunctionCallback;
            (*env)->ReleaseStringUTFChars(env, name, cname);
            (*env)->DeleteLocalRef(env, name);
        }
        (*env)->ReleaseIntArrayElements(env, staticValueAttributes, attributes, 0);
        definition.staticFunctions = funcs;
    }
    funcs[funcCount] = JSStaticEmptyFunction;
    
    JSClassRef jsclass = JSClassCreate(&definition);

    /* Clean up */
    
    // we don't need the definition any more, so clean it up
    for (int i = 0; i < valueCount; i++) {
        free((void*)values[i].name);
    }
    for (int i = 0; i < funcCount; i++) {
        free((void*)funcs[i].name);
    }
    free((void*)funcs);
    free((void*)values);

    if (classNameAsChars != NULL) {
        (*env)->ReleaseStringUTFChars(env, className, classNameAsChars);
    }
    
    return (jlong)jsclass;
}

static JSObjectPrivateData* NewJSObjectPrivateData() {
    JSObjectPrivateData* prv = (JSObjectPrivateData*)malloc(sizeof(JSObjectPrivateData));
    prv->definition = NULL;
    prv->object     = NULL;
    return prv;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMake
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsClassRef,
     jclass definitionClass, jobject definition, jobject object)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    
    JSObjectPrivateData* prv = NewJSObjectPrivateData();
    
    if (definition != NULL)
    {
        prv->definition = (*env)->NewGlobalRef(env, definition);
        prv->definitionClass = (*env)->NewGlobalRef(env, definitionClass);
    }
    if (object != NULL)
    {
        prv->object = (*env)->NewGlobalRef(env, object);
    }
    
    return (jlong)JSObjectMake(ctx, jsClass, prv);
}

JNIEXPORT jobject JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPrivate
(JNIEnv *env, jobject thiz, jlong jsObjectRef)
{
    JSObjectRef jsObject = (JSObjectRef)jsObjectRef;
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(jsObject);
    if (prv && prv->object) {
        return prv->object;
    }
    return NULL;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetPrivate
(JNIEnv *env, jobject thiz, jlong jsObjectRef, jobject object)
{
    JSObjectRef jsObject = (JSObjectRef)jsObjectRef;
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(jsObject);
    if (prv == NULL)
    {
        prv = NewJSObjectPrivateData();
    }
    if (prv->object)
    {
        (*env)->DeleteGlobalRef(env, prv->object);
    }
    if (object != NULL)
    {
        prv->object = (*env)->NewGlobalRef(env, object);
    }
    return JSObjectSetPrivate(jsObject, prv) ? JNI_TRUE : JNI_FALSE;
}

#ifdef __cplusplus
}
#endif