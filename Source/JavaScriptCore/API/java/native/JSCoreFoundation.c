//
//  JSCoreFoundation.c
//  JSCoreFoundation
//
//  Created by Kota Iguchi on 11/30/13.
//  Copyright (c) 2013 Appcelerator, Inc. All rights reserved.
//

#include <jni.h>
#include <JavaScriptCore/JavaScriptCore.h>

#ifdef __cplusplus
extern "C" {
#endif

static jclass  jniMethodInvokerClass = NULL;
static jobject jniMethodInvokerObj   = NULL;

extern bool JSCoreApp_Load(JavaVM* vm, JNIEnv* env, jclass invokerClass, jobject invokerObj,
                           JSGlobalContextRef context, JSObjectRef global_object);
    
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSCVirtualMachineInitialize
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jobject jniMethodInvoker)
{
    JavaVM* jvm;
    
    JSGlobalContextRef context = (JSGlobalContextRef)jsContextRef;
    JSObjectRef global_object = JSContextGetGlobalObject(context);
    
    jniMethodInvokerClass = (*env)->GetObjectClass(env, jniMethodInvoker);
    jniMethodInvokerObj   = (*env)->NewGlobalRef(env, jniMethodInvoker);
    
    (*env)->GetJavaVM(env, &jvm);
    return JSCoreApp_Load(jvm, env, jniMethodInvokerClass, jniMethodInvokerObj, context, global_object) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSCVirtualMachineRelease
    (JNIEnv *env, jobject thiz)
{
    (*env)->DeleteGlobalRef(env, jniMethodInvokerObj);
    
    jniMethodInvokerClass = NULL;
    jniMethodInvokerObj   = NULL;
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
    return (jlong)JSValueMakeBoolean(ctx, (bool)arg);
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
    
    return JSValueToBoolean(ctx, value);
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

#ifdef __cplusplus
}
#endif