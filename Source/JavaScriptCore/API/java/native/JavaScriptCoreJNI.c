//
//  JavaScriptCoreJNI.c
//  JavaScriptCoreJNI
//
//  Created by Kota Iguchi on 11/30/13.
//  Copyright (c) 2013 Appcelerator, Inc. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSValueRef.h>
#include "JavaScriptCoreJNI.h"

#ifdef ENABLE_JAVASCRIPTCORE_PRIVATE_API
#ifdef __APPLE__
#include "JSObjectRefPrivate.h"
#include "JSContextRefPrivate.h"
#else
#include <JavaScriptCore/JSObjectRefPrivate.h>
#include <JavaScriptCore/JSContextRefPrivate.h>
#endif
#endif

#define JSCORE_LOG_TAG "JavaScriptCore"
#define NEWLINE "\n"

#ifdef __ANDROID__
#include <stdarg.h>
#include <android/log.h>
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, JSCORE_LOG_TAG, __VA_ARGS__));
#else
#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__));fprintf(stdout, NEWLINE);fflush(stdout);
#define LOGI(...) ((void)fprintf(stdout, __VA_ARGS__));fprintf(stdout, NEWLINE);fflush(stdout);
#define LOGW(...) ((void)fprintf(stdout, __VA_ARGS__));fprintf(stdout, NEWLINE);fflush(stdout);
#define LOGE(...) ((void)fprintf(stdout, __VA_ARGS__));fprintf(stdout, NEWLINE);fflush(stdout);
#endif

/*
 * Get JNIEnv* from JVM
 */
#define JNI_ENV_ENTER \
JNIEnv* env = NULL;\
bool jvm_attached = false;\
if (jvm != NULL) {\
    jint jvm_attach_status = (*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6);\
    if (jvm_attach_status == JNI_EDETACHED) {\
        jvm_attach_status = (*jvm)->AttachCurrentThread(jvm, (void**)&env, NULL);\
        if (jvm_attach_status == JNI_OK){\
            jvm_attached = true;\
        }\
    }\
}

#define JNI_ENV_EXIT \
if (jvm_attached) {\
    (*jvm)->DetachCurrentThread(jvm);\
}\
env = NULL;

/* 
 *  Create char* from JSStringRef
 * (varout char should be freed later)
 */
#define CCHAR_FROM_JSSTRINGREF(varin, varout)\
size_t length##varin = JSStringGetMaximumUTF8CStringSize(varin);\
char* varout = (char*)malloc(length##varin);\
JSStringGetUTF8CString(varin, varout, length##varin);\

/* 
 * Create jstring from JSStringRef
 * (varcout char is freed)
 */
#define JSTRING_FROM_JSSTRINGREF(varin, varcout, varjout)\
CCHAR_FROM_JSSTRINGREF(varin, varcout);\
jstring varjout = (*env)->NewStringUTF(env, varcout);\
free(varcout);

/* 
 * Create JSStringRef from jstring
 * (varout should be freed by JSStringRelease later)
 */
#define JSSTRINGREF_FROM_JSTRING(varin, varout)\
JSStringRef varout = NULL;\
if(varin != NULL) {\
    const char* aschars##varin = (*env)->GetStringUTFChars(env, varin, NULL);\
    varout = JSStringCreateWithUTF8CString(aschars##varin);\
    (*env)->ReleaseStringUTFChars(env, varin, aschars##varin);\
}

#define JSSTRING_RELEASE(varin)\
if (varin != NULL) JSStringRelease(varin);

/*
 * Call JSObjectMake* function from arguments
 */
#define JSOBJECTMAKE_FROM_ARGV(callfunc, argc, argv, varout)\
const JSValueRef* js_argv = NULL;\
if (argc > 0) js_argv = (*env)->GetDirectBufferAddress(env, argv);\
JSObjectRef varout = callfunc(ctx, argc, js_argv, &exceptionStore);\

#ifdef __cplusplus
extern "C" {
#endif

static JavaVM* jvm;

static jmethodID jmethodId_JSObjectInitializeCallback = NULL;
static jmethodID jmethodId_JSObjectFinalizeCallback = NULL;
static jmethodID jmethodId_JSObjectGetStaticValueCallback = NULL;
static jmethodID jmethodId_JSObjectSetStaticValueCallback = NULL;
static jmethodID jmethodId_JSObjectGetPropertyCallback = NULL;
static jmethodID jmethodId_JSObjectSetPropertyCallback = NULL;
static jmethodID jmethodId_JSObjectCallAsConstructorCallback = NULL;
static jmethodID jmethodId_JSObjectMakeConstructorCallback = NULL;
static jmethodID jmethodId_JSObjectMakeFunctionCallback = NULL;
static jmethodID jmethodId_JSObjectCallAsFunctionCallback = NULL;
static jmethodID jmethodId_JSObjectConvertToTypeCallback = NULL;
static jmethodID jmethodId_JSObjectDeletePropertyCallback = NULL;
static jmethodID jmethodId_JSObjectGetPropertyNamesCallback = NULL;
static jmethodID jmethodId_JSObjectHasInstanceCallback = NULL;
static jmethodID jmethodId_JSObjectHasPropertyCallback = NULL;
static jmethodID jmethodId_JSValueRefUpdatePointerCallback = NULL;
static jmethodID jmethodId_JSObjectStaticFunctionCallback = NULL;

static JSClassDefinition jsClassDefinitionTemplate;
static JSStaticValue     jsStaticValueTemplate;
static JSStaticFunction  jsStaticFunctionTemplate;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    jvm = vm;
    return JNI_VERSION_1_6;
}

/*
 * Common functions
 */
static JSObjectPrivateData* NewJSObjectPrivateData()
{
    JSObjectPrivateData* prv = (JSObjectPrivateData*)malloc(sizeof(JSObjectPrivateData));
    prv->callback = NULL;
    prv->callbackClass = NULL;
    prv->object     = NULL;
    prv->initialized = false;
    return prv;
}

static bool CacheClassDefinitionCallbackMethods(JNIEnv* env, jclass callbackClass) {
    if (jmethodId_JSObjectInitializeCallback == NULL) {
        jmethodId_JSObjectInitializeCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectInitializeCallback", "(JJ)V");
        jmethodId_JSObjectFinalizeCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectFinalizeCallback", "(J)V");
        jmethodId_JSObjectGetStaticValueCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectGetStaticValueCallback", "(JJLjava/lang/String;J)J");
        jmethodId_JSObjectSetStaticValueCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectSetStaticValueCallback", "(JJLjava/lang/String;JJ)Z");
        jmethodId_JSObjectGetPropertyCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectGetPropertyCallback", "(JJLjava/lang/String;J)J");
        jmethodId_JSObjectSetPropertyCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectSetPropertyCallback", "(JJLjava/lang/String;JJ)Z");
        jmethodId_JSObjectCallAsConstructorCallback = (*env)->GetMethodID(
                    env, callbackClass, "JSObjectCallAsConstructorCallback", "(JJILjava/nio/ByteBuffer;J)J");
        jmethodId_JSObjectCallAsFunctionCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectCallAsFunctionCallback", "(JJJILjava/nio/ByteBuffer;J)J");
        jmethodId_JSObjectStaticFunctionCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectStaticFunctionCallback", "(JJJILjava/nio/ByteBuffer;J)J");
        jmethodId_JSObjectConvertToTypeCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectConvertToTypeCallback", "(JJIJ)J");
        jmethodId_JSObjectDeletePropertyCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectDeletePropertyCallback", "(JJLjava/lang/String;J)Z");
        jmethodId_JSObjectGetPropertyNamesCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectGetPropertyNamesCallback", "(JJJ)V");
        jmethodId_JSObjectHasInstanceCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectHasInstanceCallback", "(JJJJ)Z");
        jmethodId_JSObjectHasPropertyCallback = (*env)->GetMethodID(
                env, callbackClass, "JSObjectHasPropertyCallback", "(JJLjava/lang/String;)Z");
        jmethodId_JSObjectMakeConstructorCallback = (*env)->GetStaticMethodID(
                    env, callbackClass, "JSObjectMakeConstructorCallback", "(JJILjava/nio/ByteBuffer;J)J");
        jmethodId_JSObjectMakeFunctionCallback = (*env)->GetStaticMethodID(
                    env, callbackClass, "JSObjectMakeFunctionCallback", "(JJJILjava/nio/ByteBuffer;J)J");
    }
    return true;
}

/*
 * Update JSValueRef exception
 */
static void UpdateJSValueExceptionPointer(JNIEnv* env, JSContextRef ctx, JSValueRef exceptionValue, jobject exceptionObj) {
    // if Java object equals null, it's completely ok to ignore it
    if (exceptionObj == NULL) return;
    
    jclass clazz = (*env)->FindClass(env, "com/appcelerator/javascriptcore/opaquetypes/JSValueRef");
    
    if (jmethodId_JSValueRefUpdatePointerCallback == NULL) {
        jmethodId_JSValueRefUpdatePointerCallback = (*env)->GetMethodID(env, clazz, "UpdateJSValueRef", "(JJ)V");
    }
    if (jmethodId_JSValueRefUpdatePointerCallback != NULL) {
        (*env)->CallVoidMethod(env, exceptionObj,
                               jmethodId_JSValueRefUpdatePointerCallback, (jlong)ctx, (jlong)exceptionValue);
    }
}

static bool RegisterJSObjectCallback(JNIEnv* env, JSObjectRef object, jobject callback) {
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (callback != NULL && prv == NULL)
    {
        prv = NewJSObjectPrivateData();
        prv->callback = (*env)->NewGlobalRef(env, callback);
        prv->callbackClass = (*env)->NewGlobalRef(env, (*env)->GetObjectClass(env, callback));
        return JSObjectSetPrivate(object, prv);
    }
    return false;
}

/*
 * JavaScriptCore Callbacks
 */
/*
 * Fire initialize callback. This function may be called more than twice
 * for the one object because of the JS prototype chain, but by design actuall Java callback
 * should be fired only once. Java callback object should take care of the initializer callback chain.
 */
static void NativeCallback_JSObjectInitializeCallback(JSContextRef ctx, JSObjectRef object)
{
    LOGD("JSObjectInitializeCallback");
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback && prv->initialized == false)
    {
        (*env)->CallVoidMethod(env, prv->callback,
                               jmethodId_JSObjectInitializeCallback, (jlong)ctx, (jlong)object);
        prv->initialized = true;
    }
    JNI_ENV_EXIT
}

/*
 * Fire finalize callback. This function may be called more than twice
 * for the one object because of the JS prototype chain, but by design we release
 * private object for the first time here, actuall Java callback should be fired only once.
 * Java callback object should take care of the finalizer callback chain.
 * Do not set finalize callback on global object otherwise JSGlobalContextRelease will crash.
 */
static void NativeCallback_JSObjectFinalizeCallback(JSObjectRef object)
{
    LOGD("JSObjectFinalizeCallback");
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv)
    {
        if (prv->callback)
        {
            (*env)->CallVoidMethod(env, prv->callback,
                                   jmethodId_JSObjectFinalizeCallback, (jlong)object);
            (*env)->DeleteGlobalRef(env, prv->callback);
            (*env)->DeleteGlobalRef(env, prv->callbackClass);
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

static JSValueRef NativeCallback_JSObjectGetStaticValueCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef* exception)
{
    LOGD("JSObjectGetStaticValueCallback");
    JSValueRef value = NULL;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback,
                               jmethodId_JSObjectGetStaticValueCallback,
                               (jlong)ctx, (jlong)object, jname, (jlong)exception);
    }
    JNI_ENV_EXIT
    
    return value;
}

static bool NativeCallback_JSObjectSetStaticValueCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* exception)
{
    LOGD("JSObjectSetStaticValueCallback");
    bool result = false;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        result = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectSetStaticValueCallback,
            (jlong)ctx, (jlong)object, jname, (jlong)value, (jlong)exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT

    return result;
}
    
static JSValueRef NativeCallback_JSObjectGetPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef* exception)
{
    LOGD("JSObjectGetPropertyCallback");
    JSValueRef value = NULL;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback,
                               jmethodId_JSObjectGetPropertyCallback,
                               (jlong)ctx, (jlong)object, jname, (jlong)exception);
    }
    JNI_ENV_EXIT
    
    return value;
}

static bool NativeCallback_JSObjectSetPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* exception)
{
    LOGD("JSObjectSetPropertyCallback");
    bool result = false;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        result = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectSetPropertyCallback,
            (jlong)ctx, (jlong)object, jname, (jlong)value, (jlong)exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT

    return result;
}

static JSObjectRef NativeCallback_JSObjectCallAsConstructorCallback(
    JSContextRef ctx, JSObjectRef constructor,
    size_t argc, const JSValueRef argv[], JSValueRef *exception)
{
    LOGD("JSObjectCallAsConstructorCallback");
    JSObjectRef object = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(constructor);
    if (prv && prv->callback)
    {
        jobject argvbuffer = argc > 0 ? (*env)->NewDirectByteBuffer(env, (void*)&argv[0], sizeof(long) * argc) : NULL;
        object = (JSObjectRef)(*env)->CallLongMethod(env, prv->callback,
                            jmethodId_JSObjectCallAsConstructorCallback,
                            (jlong)ctx, (jlong)constructor, (jint)argc, argvbuffer, (jlong)exception);
    } else {
        LOGD("Constructor callback does not found for %lu", (long)constructor);
    }
    JNI_ENV_EXIT

    return object;
}

static JSObjectRef NativeCallback_JSObjectMakeConstructorCallback(
    JSContextRef ctx, JSObjectRef constructor,
    size_t argc, const JSValueRef argv[], JSValueRef *exception)
{
    LOGD("JSObjectMakeConstructorCallback");
    JSObjectRef object = NULL;
    JNI_ENV_ENTER
    jclass clazz = (*env)->FindClass(env, "com/appcelerator/javascriptcore/opaquetypes/JSClassDefinition");
    jobject argvbuffer = argc > 0 ? (*env)->NewDirectByteBuffer(env, (void*)&argv[0], sizeof(long) * argc) : NULL;
    object = (JSObjectRef)(*env)->CallStaticLongMethod(env, clazz,
                                        jmethodId_JSObjectMakeConstructorCallback,
                                        (jlong)ctx, (jlong)constructor, (jint)argc, argvbuffer, (jlong)exception);
    JNI_ENV_EXIT
    return object;
}

static JSValueRef NativeCallback_JSObjectMakeFunctionCallback(
    JSContextRef ctx, JSObjectRef func, JSObjectRef thisObject,
    size_t argc, const JSValueRef argv[], JSValueRef *exception)
{
    LOGD("JSObjectMakeFunctionCallback");
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    jclass clazz = (*env)->FindClass(env, "com/appcelerator/javascriptcore/opaquetypes/JSClassDefinition");
    jobject argvbuffer = argc > 0 ? (*env)->NewDirectByteBuffer(env, (void*)&argv[0], sizeof(long) * argc) : NULL;
    value = (JSValueRef)(*env)->CallStaticLongMethod(env, clazz,
                                        jmethodId_JSObjectMakeFunctionCallback,
                                        (jlong)ctx, (jlong)func, (jlong)thisObject,
                                        (jint)argc, argvbuffer, (jlong)exception);
    JNI_ENV_EXIT
    return value;
}

static JSValueRef NativeCallback_JSObjectCallAsFunctionCallback(
    JSContextRef ctx, JSObjectRef func, JSObjectRef thisObject,
    size_t argc, const JSValueRef argv[], JSValueRef* exception) {
    LOGD("JSObjectCallAsFunctionCallback");
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(func);
    if (prv == NULL) prv = (JSObjectPrivateData*)JSObjectGetPrivate(thisObject);
    if (prv && prv->callback)
    {
        jobject argvbuffer = argc > 0 ? (*env)->NewDirectByteBuffer(env, (void*)&argv[0], sizeof(long) * argc) : NULL;
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback, jmethodId_JSObjectCallAsFunctionCallback,
                               (jlong)ctx, (jlong)func, (jlong)thisObject, (jint)argc, argvbuffer, (jlong)exception);
    }
    JNI_ENV_EXIT
    return value;
}

static JSValueRef NativeCallback_JSObjectStaticFunctionCallback(
    JSContextRef ctx, JSObjectRef func, JSObjectRef thisObject,
    size_t argc, const JSValueRef argv[], JSValueRef* exception) {
    LOGD("JSObjectStaticFunctionCallback");
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(func);
    if (prv == NULL) prv = (JSObjectPrivateData*)JSObjectGetPrivate(thisObject);
    if (prv && prv->callback)
    {
        jobject argvbuffer = argc > 0 ? (*env)->NewDirectByteBuffer(env, (void*)&argv[0], sizeof(long) * argc) : NULL;
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback, jmethodId_JSObjectStaticFunctionCallback,
                               (jlong)ctx, (jlong)func, (jlong)thisObject, (jint)argc, argvbuffer, (jlong)exception);
    }
    JNI_ENV_EXIT
    return value;

}

static JSValueRef NativeCallback_JSObjectConvertToTypeCallback(
   JSContextRef ctx,JSObjectRef object,
   JSType type, JSValueRef *exception)
{
    LOGD("JSObjectConvertToTypeCallback");
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback, jmethodId_JSObjectConvertToTypeCallback,
                                                   (jlong)ctx, (jlong)object, (jint)type, (jlong)exception);
    }
    JNI_ENV_EXIT
    return value;
}
    
static bool NativeCallback_JSObjectDeletePropertyCallback(
    JSContextRef ctx, JSObjectRef object,
    JSStringRef name, JSValueRef *exception)
{
    LOGD("JSObjectDeletePropertyCallback");
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectDeletePropertyCallback,
                    (jlong)ctx, (jlong)object, jname, (jlong)exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT
    return value;
}

static void NativeCallback_JSObjectGetPropertyNamesCallback(
    JSContextRef ctx, JSObjectRef object,
    JSPropertyNameAccumulatorRef propertyNames)
{
    LOGD("JSObjectGetPropertyNamesCallback");
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        (*env)->CallVoidMethod(env, prv->callback, jmethodId_JSObjectGetPropertyNamesCallback,
                               (jlong)ctx, (jlong)object, (jlong)propertyNames);
    }
    JNI_ENV_EXIT
}

static bool NativeCallback_JSObjectHasInstanceCallback(
    JSContextRef ctx, JSObjectRef constructor,
    JSValueRef instance, JSValueRef *exception)
{
    LOGD("JSObjectHasInstanceCallback");
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(constructor);
    if (prv && prv->callback)
    {
        value = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectHasInstanceCallback,
                    (jlong)ctx, (jlong)constructor, (jlong)instance, (jlong)exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT
    return value;
}

static bool NativeCallback_JSObjectHasPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name)
{
    LOGD("JSObjectHasPropertyCallback");
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectHasPropertyCallback,
                                (jlong)ctx, (jlong)object, jname) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT
    return value;
}

/*
 * JNI methods
 */
/**
 * Return JSClassDefinition struct that contains default attributes and pointer to callback functions
 */
JNIEXPORT jobject JNICALL
Java_com_appcelerator_javascriptcore_opaquetypes_JSClassDefinition_NativeGetClassDefinitionTemplate
    (JNIEnv *env, jclass clazz)
{
    jsClassDefinitionTemplate = kJSClassDefinitionEmpty;
    jsClassDefinitionTemplate.version = 0;
    jsClassDefinitionTemplate.attributes = kJSClassAttributeNone;
    
    jsClassDefinitionTemplate.initialize = NativeCallback_JSObjectInitializeCallback;
    jsClassDefinitionTemplate.finalize   = NativeCallback_JSObjectFinalizeCallback;
    jsClassDefinitionTemplate.hasProperty = NativeCallback_JSObjectHasPropertyCallback;
    jsClassDefinitionTemplate.getProperty = NativeCallback_JSObjectGetPropertyCallback;
    jsClassDefinitionTemplate.setProperty = NativeCallback_JSObjectSetPropertyCallback;
    jsClassDefinitionTemplate.deleteProperty = NativeCallback_JSObjectDeletePropertyCallback;
    jsClassDefinitionTemplate.getPropertyNames = NativeCallback_JSObjectGetPropertyNamesCallback;
    jsClassDefinitionTemplate.callAsFunction   = NativeCallback_JSObjectCallAsFunctionCallback;
    jsClassDefinitionTemplate.callAsConstructor = NativeCallback_JSObjectCallAsConstructorCallback;
    jsClassDefinitionTemplate.hasInstance   = NativeCallback_JSObjectHasInstanceCallback;
    jsClassDefinitionTemplate.convertToType = NativeCallback_JSObjectConvertToTypeCallback;
    
    return (*env)->NewDirectByteBuffer(env, &jsClassDefinitionTemplate, sizeof(JSClassDefinition));
}

/**
 * Return JSStaticValue struct that contains default attributes and pointer to callback functions
 */
JNIEXPORT jobject JNICALL
Java_com_appcelerator_javascriptcore_opaquetypes_JSStaticValues_NativeGetStaticValueTemplate
    (JNIEnv *env, jclass clazz)
{
    jsStaticValueTemplate.attributes = kJSPropertyAttributeNone;
    jsStaticValueTemplate.getProperty = NativeCallback_JSObjectGetStaticValueCallback;
    jsStaticValueTemplate.setProperty = NativeCallback_JSObjectSetStaticValueCallback;
    
    return (*env)->NewDirectByteBuffer(env, &jsStaticValueTemplate, sizeof(JSStaticValue));
}

/**
 * Return JSStaticFunction struct that contains default attributes and pointer to callback functions
 */
JNIEXPORT jobject JNICALL
Java_com_appcelerator_javascriptcore_opaquetypes_JSStaticFunctions_NativeGetStaticFunctionTemplate
    (JNIEnv *env, jclass clazz)
{
    jsStaticFunctionTemplate.attributes = kJSPropertyAttributeNone;
    jsStaticFunctionTemplate.callAsFunction = NativeCallback_JSObjectStaticFunctionCallback;
    
    return (*env)->NewDirectByteBuffer(env, &jsStaticFunctionTemplate, sizeof(JSStaticFunction));
}

/**
 * Allocate char arrays from Java String
 */
JNIEXPORT jlongArray JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeAllocateCharacterBuffer
    (JNIEnv *env, jclass clazz, jobjectArray invalues)
{
    int valueCount = (*env)->GetArrayLength(env, invalues);
    jlongArray outvalues = (*env)->NewLongArray(env, valueCount);
    jlong* p_outvalues = (*env)->GetLongArrayElements(env, outvalues, NULL);
    for (int i = 0; i < valueCount; i++) {
        jobject invalue = (*env)->GetObjectArrayElement(env, invalues, i);
        const char* inCvalue = (*env)->GetStringUTFChars(env, invalue, NULL);
        p_outvalues[i] = (jlong)strdup(inCvalue);
        (*env)->ReleaseStringUTFChars(env, invalue, inCvalue);
        (*env)->DeleteLocalRef(env, invalue);
    }
    (*env)->ReleaseLongArrayElements(env, outvalues, p_outvalues, 0);
    
    return outvalues;
}

/*
 * Release char arrays from pointers
 */
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeReleasePointers
    (JNIEnv *env, jclass clazz, jlongArray invalues)
{
    int valueCount = (*env)->GetArrayLength(env, invalues);
    jlong* p_invalues = (*env)->GetLongArrayElements(env, invalues, NULL);
    for (int i = 0; i < valueCount; i++) {
        free((void*)p_invalues[i]);
    }
    (*env)->ReleaseLongArrayElements(env, invalues, p_invalues, 0);
}

/*!
@function
@abstract Creates a JavaScript class suitable for use with JSObjectMake.
@param definition A JSClassDefinition that defines the class.
@result A JSClass with the given definition. Ownership follows the Create Rule.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassCreate
    (JNIEnv *env, jobject thiz, jobject definitionBuffer, jstring className, jobject staticValuesBuffer, jobject staticFunctionsBuffer)
{
    LOGD("JSClassCreate");
    JSClassDefinition* definition = (*env)->GetDirectBufferAddress(env, definitionBuffer);
    
    JSStaticValue* staticValues = NULL;
    if (staticValuesBuffer != NULL) staticValues = (*env)->GetDirectBufferAddress(env, staticValuesBuffer);
    
    JSStaticFunction* staticFunctions = NULL;
    if (staticFunctionsBuffer != NULL) staticFunctions = (*env)->GetDirectBufferAddress(env, staticFunctionsBuffer);
    
    definition->staticValues    = staticValues;
    definition->staticFunctions = staticFunctions;
    
    const char* classNameAsChars = NULL;
    if (className != NULL)
    {
        classNameAsChars = (*env)->GetStringUTFChars(env, className, NULL);
        definition->className = classNameAsChars;
    }
    
    JSClassRef jsClass = JSClassCreate(definition);
    
    if (classNameAsChars != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, className, classNameAsChars);
    }
    
    return (jlong)jsClass;
}

/*!
@function
@abstract Creates a JavaScript object.
@param ctx The execution context to use.
@param jsClass The JSClass to assign to the object. Pass NULL to use the default object class.
@param data A void* to set as the object's private data. Pass NULL to specify no private data.
@result A JSObject with the given class and private data.
@discussion The default object class does not allocate storage for private data, so you must provide a non-NULL jsClass to JSObjectMake if you want your object to be able to store private data.

data is set on the created object before the intialize methods in its class chain are called. This enables the initialize methods to retrieve and manipulate data through JSObjectGetPrivate.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMake
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsClassRef,
     jobject callback, jobject staticFunctionsBuffer, jint staticFunctionCount, jobject object)
{
    LOGD("JSObjectMake");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    
    JSObjectPrivateData* prv = NewJSObjectPrivateData();
    
    if (callback != NULL)
    {
        prv->callback = (*env)->NewGlobalRef(env, callback);
        prv->callbackClass = (*env)->NewGlobalRef(env, (*env)->GetObjectClass(env, callback));
    }
    if (object != NULL)
    {
        prv->object = (*env)->NewGlobalRef(env, object);
    }
    
    if (prv->callbackClass != NULL) {
        CacheClassDefinitionCallbackMethods(env, prv->callbackClass);
    }
    
    return (jlong)JSObjectMake(ctx, jsClass, prv);
}

/*!
@function
@abstract Gets an object's private data.
@param object A JSObject whose private data you want to get.
@result A void* that is the object's private data, if the object has private data, otherwise NULL.
*/
JNIEXPORT jobject JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPrivate
(JNIEnv *env, jobject thiz, jlong jsObjectRef)
{
    LOGD("JSObjectGetPrivate");
    JSObjectRef jsObject = (JSObjectRef)jsObjectRef;
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(jsObject);
    if (prv && prv->object) {
        return prv->object;
    }
    return NULL;
}

/*!
@function
@abstract Sets a pointer to private data on an object.
@param object The JSObject whose private data you want to set.
@param data A void* to set as the object's private data.
@result true if object can store private data, otherwise false.
@discussion The default object class does not allocate storage for private data. Only objects created with a non-NULL JSClass can store private data.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetPrivate
(JNIEnv *env, jobject thiz, jlong jsObjectRef, jobject object)
{
    LOGD("JSObjectSetPrivate");
    JSObjectRef jsObject = (JSObjectRef)jsObjectRef;
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(jsObject);
    if (prv == NULL)
    {
        prv = NewJSObjectPrivateData();
    }
    if (prv->object)
    {
        (*env)->DeleteGlobalRef(env, prv->object);
        prv->object = NULL;
    }
    if (object != NULL)
    {
        prv->object = (*env)->NewGlobalRef(env, object);
    }
    return JSObjectSetPrivate(jsObject, prv) ? JNI_TRUE : JNI_FALSE;
}
/*!
@function
@abstract Creates a JavaScript context group.
@discussion A JSContextGroup associates JavaScript contexts with one another.
 Contexts in the same group may share and exchange JavaScript objects. Sharing and/or exchanging
 JavaScript objects between contexts in different groups will produce undefined behavior.
 When objects from the same context group are used in multiple threads, explicit
 synchronization is required.
@result The created JSContextGroup.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupCreate
    (JNIEnv *env, jobject thiz)
{
    LOGD("JSContextGroupCreate");
    JSContextGroupRef group = JSContextGroupCreate();
    return (jlong)group;
}

/*!
@function
@abstract Releases a JavaScript context group.
@param group The JSContextGroup to release.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupRelease
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef)
{
    LOGD("JSContextGroupRelease");
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    JSContextGroupRelease(group);
}
/*!
@function
@abstract Retains a JavaScript context group.
@param group The JSContextGroup to retain.
@result A JSContextGroup that is the same as group.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupRetain
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef)
{
    LOGD("JSContextGroupRetain");
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    return (jlong)JSContextGroupRetain(group);
}

/*!
@function
@abstract Gets the global object of a JavaScript execution context.
@param ctx The JSContext whose global object you want to get.
@result ctx's global object.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGetGlobalObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSContextGetGlobalObject");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSContextGetGlobalObject(ctx);
}

/*!
@function
@abstract Gets the context group to which a JavaScript execution context belongs.
@param ctx The JSContext whose group you want to get.
@result ctx's group.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGetGroup
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSContextGetGroup");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSContextGetGroup(ctx);
}

/*!
@function
@abstract Creates a global JavaScript execution context.
@discussion JSGlobalContextCreate allocates a global object and populates it with all the
 built-in JavaScript objects, such as Object, Function, String, and Array.

 In WebKit version 4.0 and later, the context is created in a unique context group.
 Therefore, scripts may execute in it concurrently with scripts executing in other contexts.
 However, you may not use values created in the context in other contexts.
@param globalObjectClass The class to use when creating the global object. Pass 
 NULL to use the default object class.
@result A JSGlobalContext with a global object of class globalObjectClass.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextCreate
    (JNIEnv *env, jobject thiz, jlong jsClassRef, jobject callback)
{
    LOGD("JSGlobalContextCreate");
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    JSGlobalContextRef ctx = JSGlobalContextCreate(jsClass);
    JSObjectRef globalObject = JSContextGetGlobalObject(ctx);
    RegisterJSObjectCallback(env, globalObject, callback);
    NativeCallback_JSObjectInitializeCallback(ctx, globalObject);
    return (jlong)ctx;
}

/*!
@function
@abstract Creates a global JavaScript execution context in the context group provided.
@discussion JSGlobalContextCreateInGroup allocates a global object and populates it with
 all the built-in JavaScript objects, such as Object, Function, String, and Array.
@param globalObjectClass The class to use when creating the global object. Pass
 NULL to use the default object class.
@param group The context group to use. The created global context retains the group.
 Pass NULL to create a unique group for the context.
@result A JSGlobalContext with a global object of class globalObjectClass and a context
 group equal to group.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextCreateInGroup
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef, jlong jsClassRef, jobject callback)
{
    LOGD("JSGlobalContextCreateInGroup");
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    JSGlobalContextRef ctx = JSGlobalContextCreateInGroup(group, jsClass);
    JSObjectRef globalObject = JSContextGetGlobalObject(ctx);
    RegisterJSObjectCallback(env, globalObject, callback);
    NativeCallback_JSObjectInitializeCallback(ctx, globalObject);
    return (jlong)ctx;
}

/*!
@function
@abstract Releases a global JavaScript execution context.
@param ctx The JSGlobalContext to release.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextRelease
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSGlobalContextRelease");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSGlobalContextRelease(ctx);
}

/*!
@function
@abstract Retains a global JavaScript execution context.
@param ctx The JSGlobalContext to retain.
@result A JSGlobalContext that is the same as ctx.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextRetain
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSGlobalContextRetain");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSGlobalContextRetain(ctx);
}
/*!
@function JSEvaluateScript
@abstract Evaluates a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to evaluate.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSValue that results from evaluating script, or NULL if an exception is thrown.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSEvaluateScriptShort
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring script, jobject exceptionObj)
{
    LOGD("JSEvaluateScriptShort");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSSTRINGREF_FROM_JSTRING(script, scriptJS)
    
    JSValueRef result = JSEvaluateScript(ctx, scriptJS, NULL, NULL, 1, &exceptionStore);
    JSSTRING_RELEASE(scriptJS);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    
    return (jlong)result;
}

/*!
@function JSEvaluateScript
@abstract Evaluates a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to evaluate.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSValue that results from evaluating script, or NULL if an exception is thrown.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSEvaluateScriptFull
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring script,
     jlong jsObjectRef, jstring sourceURL, jint line, jobject exceptionObj)
{
    LOGD("JSEvaluateScriptFull");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSObjectRef object = (JSObjectRef)jsObjectRef;

    JSSTRINGREF_FROM_JSTRING(script, scriptJS)
    JSSTRINGREF_FROM_JSTRING(sourceURL, jsSourceURL)
    
    JSValueRef result = JSEvaluateScript(ctx, scriptJS, object, jsSourceURL, line, &exceptionStore);
    JSSTRING_RELEASE(scriptJS);
    JSSTRING_RELEASE(jsSourceURL);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }

    return (jlong)result;
}

/*!
@function JSCheckScriptSyntax
@abstract Checks for syntax errors in a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to check for syntax errors.
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions.
@param exception A pointer to a JSValueRef in which to store a syntax error exception, if any. Pass NULL if you do not care to store a syntax error exception.
@result true if the script is syntactically correct, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSCheckScriptSyntax
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring script, jobject exceptionObj)
{
    LOGD("JSCheckScriptSyntax");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSSTRINGREF_FROM_JSTRING(script, scriptJS)
    
    bool check = JSCheckScriptSyntax(ctx, scriptJS, NULL, 1, &exceptionStore);
    JSSTRING_RELEASE(scriptJS);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    
    return check ? JNI_TRUE : JNI_FALSE;
}

/*!
@function JSGarbageCollect
@abstract Performs a JavaScript garbage collection. 
@param ctx The execution context to use.
@discussion JavaScript values that are on the machine stack, in a register, 
 protected by JSValueProtect, set as the global object of an execution context, 
 or reachable from any such value will not be collected.

 During JavaScript execution, you are not required to call this function; the 
 JavaScript engine will garbage collect as needed. JavaScript values created
 within a context group are automatically destroyed when the last reference
 to the context group is released.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGarbageCollect
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSGarbageCollect");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSGarbageCollect(ctx);
}

/*!
@function
@abstract       Creates a JavaScript value of the undefined type.
@param ctx  The execution context to use.
@result         The unique undefined value.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeUndefined
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSValueMakeUndefined");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeUndefined(ctx);
}

/*!
@function
@abstract       Creates a JavaScript value of the null type.
@param ctx  The execution context to use.
@result         The unique null value.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeNull
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSValueMakeNull");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeNull(ctx);
}

/*!
@function
@abstract       Creates a JavaScript value of the number type.
@param ctx  The execution context to use.
@param number   The double to assign to the newly created JSValue.
@result         A JSValue of the number type, representing the value of number.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jdouble arg)
{
    LOGD("JSValueMakeNumber");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeNumber(ctx, (double)arg);
}

/*!
@function
@abstract       Creates a JavaScript value of the boolean type.
@param ctx  The execution context to use.
@param boolean  The bool to assign to the newly created JSValue.
@result         A JSValue of the boolean type, representing the value of boolean.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jboolean arg)
{
    LOGD("JSValueMakeBoolean");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeBoolean(ctx, arg == JNI_TRUE ? true : false);
}

/*!
@function
@abstract Tests whether a JavaScript value is an object with a given class in its class chain.
@param ctx The execution context to use.
@param value The JSValue to test.
@param jsClass The JSClass to test against.
@result true if value is an object and has jsClass in its class chain, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsObjectOfClass
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsClassRef)
{
    LOGD("JSValueIsObjectOfClass");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSClassRef jsClass = (JSClassRef)jsClassRef;
    return JSValueIsObjectOfClass(ctx, value, jsClass) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract Tests whether a JavaScript value is an object constructed by a given constructor, as compared by the JS instanceof operator.
@param ctx The execution context to use.
@param value The JSValue to test.
@param constructor The constructor to test against.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result true if value is an object constructed by constructor, as compared by the JS instanceof operator, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsInstanceOfConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsObjectRef, jobject exceptionObj)
{
    LOGD("JSValueIsInstanceOfConstructor");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSObjectRef constructor = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    jboolean result = JSValueIsInstanceOfConstructor(ctx, value, constructor, &exceptionStore) ? JNI_TRUE : JNI_FALSE;
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return result;
}

/*!
@function
@abstract       Tests whether a JavaScript value's type is the undefined type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the undefined type, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsUndefined
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsUndefined");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsUndefined(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract       Tests whether a JavaScript value's type is the null type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the null type, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsNull
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsNull");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsNull(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract       Tests whether a JavaScript value's type is the number type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the number type, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsNumber");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsNumber(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract       Tests whether a JavaScript value's type is the boolean type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the boolean type, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsBoolean");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsBoolean(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract       Tests whether a JavaScript value's type is the string type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the string type, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsString");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsString(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract       Tests whether a JavaScript value's type is the object type.
@param ctx  The execution context to use.
@param value    The JSValue to test.
@result         true if value's type is the object type, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsObject");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsObject(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract       Converts a JavaScript value to boolean and returns the resulting boolean.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@result         The boolean result of conversion.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueToBoolean");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    return JSValueToBoolean(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract       Converts a JavaScript value to number and returns the resulting number.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         The numeric result of conversion, or NaN if an exception is thrown.
*/
JNIEXPORT jdouble JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jobject exceptionObj)
{
    LOGD("JSValueToNumber");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    double result = JSValueToNumber(ctx, value, &exceptionStore);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return (jdouble)result;
}

/*!
@function
@abstract Converts a JavaScript value to object and returns the resulting object.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         The JSObject result of conversion, or NULL if an exception is thrown.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jobject exceptionObj)
{
    LOGD("JSValueToObject");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSObjectRef result = JSValueToObject(ctx, value, &exceptionStore);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return (jlong)result;
}

/*!
 @function
 @abstract       Creates a JavaScript value from a JSON formatted string.
 @param ctx      The execution context to use.
 @param string   The JSString containing the JSON string to be parsed.
 @result         A JSValue containing the parsed value, or NULL if the input is invalid.
 */
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeFromJSONString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring jjson)
{
    LOGD("JSValueMakeFromJSONString");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    
    JSSTRINGREF_FROM_JSTRING(jjson, json)
    JSValueRef value = JSValueMakeFromJSONString(ctx, json);
    JSSTRING_RELEASE(json);
    
    return (jlong)value;
}

/*!
@function
@abstract       Converts a JavaScript value to string and copies the result into a JavaScript string.
@param ctx  The execution context to use.
@param value    The JSValue to convert.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result         A JSString with the result of conversion, or NULL if an exception is thrown. Ownership follows the Create Rule.
*/
JNIEXPORT jstring JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToStringCopy
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jobject exceptionObj)
{
    LOGD("JSValueToStringCopy");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSStringRef jsstring = JSValueToStringCopy(ctx, value, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
        return NULL;
    }

    size_t length = JSStringGetMaximumUTF8CStringSize(jsstring);
    char* aschars = (char*)malloc(length);
    JSStringGetUTF8CString(jsstring, aschars, length);
    JSSTRING_RELEASE(jsstring);
    
    jstring copy = (*env)->NewStringUTF(env, aschars);
    free(aschars);
    return copy;
}

/*!
@function
@abstract Tests whether two JavaScript values are equal, as compared by the JS == operator.
@param ctx The execution context to use.
@param a The first value to test.
@param b The second value to test.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result true if the two values are equal, false if they are not equal or an exception is thrown.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsEqual
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRefA, jlong jsValueRefB, jobject exceptionObj)
{
    LOGD("JSValueIsEqual");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef a = (JSValueRef)jsValueRefA;
    JSValueRef b = (JSValueRef)jsValueRefB;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    jboolean result = JSValueIsEqual(ctx, a, b, &exceptionStore) ? JNI_TRUE : JNI_FALSE;
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return result;
}

/*!
@function
@abstract       Tests whether two JavaScript values are strict equal, as compared by the JS === operator.
@param ctx  The execution context to use.
@param a        The first value to test.
@param b        The second value to test.
@result         true if the two values are strict equal, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsStrictEqual
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRefA, jlong jsValueRefB)
{
    LOGD("JSValueIsStrictEqual");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef a = (JSValueRef)jsValueRefA;
    JSValueRef b = (JSValueRef)jsValueRefB;
    
    return JSValueIsStrictEqual(ctx, a, b) ? JNI_TRUE : JNI_FALSE;
}
/*!
@function
@abstract Protects a JavaScript value from garbage collection.
@param ctx The execution context to use.
@param value The JSValue to protect.
@discussion Use this method when you want to store a JSValue in a global or on the heap, where the garbage collector will not be able to discover your reference to it.
 
A value may be protected multiple times and must be unprotected an equal number of times before becoming eligible for garbage collection.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueProtect
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueProtect");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueProtect(ctx, value);
}

/*!
@function
@abstract       Unprotects a JavaScript value from garbage collection.
@param ctx      The execution context to use.
@param value    The JSValue to unprotect.
@discussion     A value may be protected multiple times and must be unprotected an 
 equal number of times before becoming eligible for garbage collection.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueUnprotect
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueUnprotect");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueUnprotect(ctx, value);
}

/*!
 @function
 @abstract       Creates a JavaScript string containing the JSON serialized representation of a JS value.
 @param ctx      The execution context to use.
 @param value    The value to serialize.
 @param indent   The number of spaces to indent when nesting.  If 0, the resulting JSON will not contains newlines.  The size of the indent is clamped to 10 spaces.
 @param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result         A JSString with the result of serialization, or NULL if an exception is thrown.
 */
JNIEXPORT jstring JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueCreateJSONString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jint indent, jobject exceptionObj)
{
    LOGD("JSValueCreateJSONString");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSStringRef jsstring = JSValueCreateJSONString(ctx, value, indent, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
        return NULL;
    }
    
    size_t length = JSStringGetMaximumUTF8CStringSize(jsstring);
    char* aschars = (char*)malloc(length);
    JSStringGetUTF8CString(jsstring, aschars, length);
    JSSTRING_RELEASE(jsstring);
    
    jstring copy = (*env)->NewStringUTF(env, aschars);
    free(aschars);
    return copy;
}

/*!
@function
@abstract       Creates a JavaScript value of the string type.
@param ctx  The execution context to use.
@param string   The JSString to assign to the newly created JSValue. The
 newly created JSValue retains string, and releases it upon garbage collection.
@result         A JSValue of the string type, representing the value of string.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring value)
{
    LOGD("JSValueMakeString");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSSTRINGREF_FROM_JSTRING(value, jsvalue)
    JSValueRef string = JSValueMakeString(ctx, jsvalue);
    JSSTRING_RELEASE(jsvalue);
    return (jlong)string;
}

/*!
@function
@abstract       Returns a JavaScript value's type.
@param ctx  The execution context to use.
@param value    The JSValue whose type you want to obtain.
@result         A value of type JSType that identifies value's type.
*/
JNIEXPORT jint JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueGetType
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueGetType");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    return (jint)JSValueGetType(ctx, value);
}

/*!
@function
@abstract Calls an object as a constructor.
@param ctx The execution context to use.
@param object The JSObject to call as a constructor.
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of arguments to pass to the constructor. Pass NULL if argumentCount is 0.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSObject that results from calling object as a constructor, or NULL if an exception is thrown or object is not a constructor.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCallAsConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint argc, jobject argv, jobject exceptionObj)
{
    LOGD("JSObjectCallAsConstructor");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    const JSValueRef* js_argv = NULL;
    if (argc > 0) js_argv = (*env)->GetDirectBufferAddress(env, argv);
    JSObjectRef value = JSObjectCallAsConstructor(ctx, object, argc, js_argv, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
   
    return (jlong)value;
}

/*!
@function
@abstract Calls an object as a function.
@param ctx The execution context to use.
@param object The JSObject to call as a function.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param argumentCount An integer count of the number of arguments in arguments.
@param arguments A JSValue array of arguments to pass to the function. Pass NULL if argumentCount is 0.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSValue that results from calling object as a function, or NULL if an exception is thrown or object is not a function.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCallAsFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jlong jsThisObjectRef,
     jint argc, jobject argv, jobject exceptionObj)
{
    LOGD("JSObjectCallAsFunction");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSObjectRef thisObject = (JSObjectRef)jsThisObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    const JSValueRef* js_argv = NULL;
    if (argc > 0) js_argv = (*env)->GetDirectBufferAddress(env, argv);
    JSValueRef value = JSObjectCallAsFunction(ctx, object, thisObject, argc, js_argv, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    
    return (jlong)value;
}

/*!
@function
@abstract Sets a property on an object.
@param ctx The execution context to use.
@param object The JSObject whose property you want to set.
@param propertyName A JSString containing the property's name.
@param value A JSValue to use as the property's value.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@param attributes A logically ORed set of JSPropertyAttributes to give to the property.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring name,
     jlong jsValueRef, jint attributes, jobject exceptionObj)
{
    LOGD("JSObjectSetProperty");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    JSObjectSetProperty(ctx, object, jsname, value, attributes, &exceptionStore);
    JSSTRING_RELEASE(jsname);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
}

/*!
@function
@abstract Gets a property from an object.
@param ctx The execution context to use.
@param object The JSObject whose property you want to get.
@param propertyName A JSString containing the property's name.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The property's value if object has the property, otherwise the undefined value.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef,
     jstring name, jobject exceptionObj)
{
    LOGD("JSObjectGetProperty");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSSTRINGREF_FROM_JSTRING(name, jsname)

    JSValueRef value = JSObjectGetProperty(ctx, object, jsname, &exceptionStore);
    JSSTRING_RELEASE(jsname);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    
    return (jlong)value;
}

/*!
@function
@abstract Releases a JavaScript class.
@param jsClass The JSClass to release.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassRelease
    (JNIEnv *env, jobject thiz, jlong jsClassRef)
{
    LOGD("JSClassRelease");
    JSClassRelease((JSClassRef)jsClassRef);
}

/*!
@function
@abstract Retains a JavaScript class.
@param jsClass The JSClass to retain.
@result A JSClass that is the same as jsClass.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassRetain
    (JNIEnv *env, jobject thiz, jlong jsClassRef)
{
    LOGD("JSClassRetain");
    return (jlong)JSClassRetain((JSClassRef)jsClassRef);
}

/*!
@function
@abstract Gets a count of the number of items in a JavaScript property name array.
@param array The array from which to retrieve the count.
@result An integer count of the number of names in array.
*/
JNIEXPORT jint JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayGetCount
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    LOGD("JSPropertyNameArrayGetCount");
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    return (jint)JSPropertyNameArrayGetCount(array);
}

/*!
@function
@abstract Gets a property name at a given index in a JavaScript property name array.
@param array The array from which to retrieve the property name.
@param index The index of the property name to retrieve.
@result A JSStringRef containing the property name.
*/
JNIEXPORT jstring JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayGetNameAtIndex
    (JNIEnv *env, jobject thiz, jlong namesRef, jint index)
{
    LOGD("JSPropertyNameArrayGetNameAtIndex");
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    JSStringRef name = JSPropertyNameArrayGetNameAtIndex(array, index);
    JSTRING_FROM_JSSTRINGREF(name, cname, jname)
    return jname;
}

/*!
@function
@abstract Releases a JavaScript property name array.
@param array The JSPropetyNameArray to release.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayRelease
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    LOGD("JSPropertyNameArrayRelease");
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    JSPropertyNameArrayRelease(array);
}

/*!
@function
@abstract Retains a JavaScript property name array.
@param array The JSPropertyNameArray to retain.
@result A JSPropertyNameArray that is the same as array.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayRetain
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    LOGD("JSPropertyNameArrayRetain");
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    return (jlong)JSPropertyNameArrayRetain(array);
}

/*!
@function
@abstract Adds a property name to a JavaScript property name accumulator.
@param accumulator The accumulator object to which to add the property name.
@param propertyName The property name to add.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameAccumulatorAddName
    (JNIEnv *env, jobject thiz, jlong accumulatorRef, jstring name)
{
    LOGD("JSPropertyNameAccumulatorAddName");
    JSPropertyNameAccumulatorRef accumulator = (JSPropertyNameAccumulatorRef)accumulatorRef;
    JSSTRINGREF_FROM_JSTRING(name, jsname)

    JSPropertyNameAccumulatorAddName(accumulator, jsname);
    JSSTRING_RELEASE(jsname);
}

/*!
@function
@abstract Sets a property on an object by numeric index.
@param ctx The execution context to use.
@param object The JSObject whose property you want to set.
@param propertyIndex The property's name as a number.
@param value A JSValue to use as the property's value.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@discussion Calling JSObjectSetPropertyAtIndex is equivalent to calling JSObjectSetProperty with a string containing propertyIndex, but JSObjectSetPropertyAtIndex provides optimized access to numeric properties.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetPropertyAtIndex
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint propertyIndex,
     jlong jsValueRef, jobject exceptionObj)
{
    LOGD("JSObjectSetPropertyAtIndex");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSObjectSetPropertyAtIndex(ctx, object, propertyIndex, value, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
}

/*!
@function
@abstract Sets an object's prototype.
@param ctx  The execution context to use.
@param object The JSObject whose prototype you want to set.
@param value A JSValue to set as the object's prototype.
*/
JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetPrototype
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jlong jsValueRef)
{
    LOGD("JSObjectSetPrototype");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    JSObjectSetPrototype(ctx, object, value);
}

/*!
@function
@abstract Tests whether an object can be called as a constructor.
@param ctx  The execution context to use.
@param object The JSObject to test.
@result true if the object can be called as a constructor, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectIsConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectIsConstructor");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return JSObjectIsConstructor(ctx, object) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract Tests whether an object can be called as a function.
@param ctx  The execution context to use.
@param object The JSObject to test.
@result true if the object can be called as a function, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectIsFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectIsFunction");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return JSObjectIsFunction(ctx, object) ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract Gets the names of an object's enumerable properties.
@param ctx The execution context to use.
@param object The object whose property names you want to get.
@result A JSPropertyNameArray containing the names object's enumerable properties. Ownership follows the Create Rule.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCopyPropertyNames
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectCopyPropertyNames");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return (jlong)JSObjectCopyPropertyNames(ctx, object);
}

/*!
@function
@abstract Deletes a property from an object.
@param ctx The execution context to use.
@param object The JSObject whose property you want to delete.
@param propertyName A JSString containing the property's name.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result true if the delete operation succeeds, otherwise false (for example, if the property has the kJSPropertyAttributeDontDelete attribute set).
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectDeleteProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring name, jobject exceptionObj)
{
    LOGD("JSObjectDeleteProperty");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    bool value = JSObjectDeleteProperty(ctx, object, jsname, &exceptionStore);
    JSSTRING_RELEASE(jsname);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return value ? JNI_TRUE : JNI_FALSE;
}

/*!
@function
@abstract Gets a property from an object by numeric index.
@param ctx The execution context to use.
@param object The JSObject whose property you want to get.
@param propertyIndex An integer value that is the property's name.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The property's value if object has the property, otherwise the undefined value.
@discussion Calling JSObjectGetPropertyAtIndex is equivalent to calling JSObjectGetProperty with a string containing propertyIndex, but JSObjectGetPropertyAtIndex provides optimized access to numeric properties.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPropertyAtIndex
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint index, jobject exceptionObj)
{
    LOGD("JSObjectGetPropertyAtIndex");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSValueRef result = JSObjectGetPropertyAtIndex(ctx, object, index, &exceptionStore);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return (jlong)result;
}

/*!
@function
@abstract Gets an object's prototype.
@param ctx  The execution context to use.
@param object A JSObject whose prototype you want to get.
@result A JSValue that is the object's prototype.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPrototype
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectGetPrototype");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return (jlong)JSObjectGetPrototype(ctx, object);
}

/*!
@function
@abstract Tests whether an object has a given property.
@param object The JSObject to test.
@param propertyName A JSString containing the property's name.
@result true if the object has a property whose name matches propertyName, otherwise false.
*/
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectHasProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring name)
{
    LOGD("JSObjectHasProperty");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    bool value = JSObjectHasProperty(ctx, object, jsname);
    JSSTRING_RELEASE(jsname);
    return value ? JNI_TRUE : JNI_FALSE;
}

/*!
 @function
 @abstract Creates a JavaScript Array object.
 @param ctx The execution context to use.
 @param argumentCount An integer count of the number of arguments in arguments.
 @param arguments A JSValue array of data to populate the Array with. Pass NULL if argumentCount is 0.
 @param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result A JSObject that is an Array.
 @discussion The behavior of this function does not exactly match the behavior of the built-in Array constructor. Specifically, if one argument 
 is supplied, this function returns an array with one element.
 */
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeArray
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv, jobject exceptionObj)
{
    LOGD("JSObjectMakeArray");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeArray, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return (jlong)value;
}

/*!
 @function
 @abstract Creates a JavaScript Date object, as if by invoking the built-in Date constructor.
 @param ctx The execution context to use.
 @param argumentCount An integer count of the number of arguments in arguments.
 @param arguments A JSValue array of arguments to pass to the Date Constructor. Pass NULL if argumentCount is 0.
 @param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result A JSObject that is a Date.
 */
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeDate
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv, jobject exceptionObj)
{
    LOGD("JSObjectMakeDate");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeDate, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return (jlong)value;
}

/*!
 @function
 @abstract Creates a JavaScript Error object, as if by invoking the built-in Error constructor.
 @param ctx The execution context to use.
 @param argumentCount An integer count of the number of arguments in arguments.
 @param arguments A JSValue array of arguments to pass to the Error Constructor. Pass NULL if argumentCount is 0.
 @param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result A JSObject that is a Error.
 */
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeError
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv, jobject exceptionObj)
{
    LOGD("JSObjectMakeError");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeError, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return (jlong)value;
}

/*!
 @function
 @abstract Creates a JavaScript RegExp object, as if by invoking the built-in RegExp constructor.
 @param ctx The execution context to use.
 @param argumentCount An integer count of the number of arguments in arguments.
 @param arguments A JSValue array of arguments to pass to the RegExp Constructor. Pass NULL if argumentCount is 0.
 @param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result A JSObject that is a RegExp.
 */
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeRegExp
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv, jobject exceptionObj)
{
    LOGD("JSObjectMakeRegExp");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeRegExp, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }
    return (jlong)value;
}

JNIEXPORT jshort JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeSizeOfLong
    (JNIEnv* env, jclass clazz)
{
    return (jshort)sizeof(long);
}

JNIEXPORT jshort JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeSizeOfInt
    (JNIEnv* env, jclass clazz)
{
    return (jshort)sizeof(int);
}

JNIEXPORT jshort JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeSizeOfUnsigned
    (JNIEnv* env, jclass clazz)
{
    return (jshort)sizeof(unsigned);
}

JNIEXPORT jshort JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeSizeOfJSClassDefinition
    (JNIEnv* env, jclass clazz)
{
    return (jshort)sizeof(JSClassDefinition);
}

JNIEXPORT jshort JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeSizeOfJSStaticValue
    (JNIEnv* env, jclass clazz)
{
    return (jshort)sizeof(JSStaticValue);
}

JNIEXPORT jshort JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeSizeOfJSStaticFunction
    (JNIEnv* env, jclass clazz)
{
    return (jshort)sizeof(JSStaticFunction);
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_opaquetypes_Pointer_NativeUpdatePointer
    (JNIEnv* env, jobject thiz, jlong toJPointer, jlong fromJValue)
{
    void* fromValue = (void*)fromJValue;
    void** toPointer = (void**)toJPointer;
    
    *toPointer = fromValue;
}

/*!
@function
@abstract Creates a function with a given script as its body.
@param ctx The execution context to use.
@param name A JSString containing the function's name. This will be used when converting the function to string. Pass NULL to create an anonymous function.
@param parameterCount An integer count of the number of parameter names in parameterNames.
@param parameterNames A JSString array containing the names of the function's parameters. Pass NULL if parameterCount is 0.
@param body A JSString containing the script to use as the function's body.
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions.
@param exception A pointer to a JSValueRef in which to store a syntax error exception, if any. Pass NULL if you do not care to store a syntax error exception.
@result A JSObject that is a function, or NULL if either body or parameterNames contains a syntax error. The object's prototype will be the default function prototype.
@discussion Use this method when you want to execute a script repeatedly, to avoid the cost of re-parsing the script before each execution.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring name, jint paramCount,
     jobjectArray paramNames, jstring body, jstring sourceURL, jint line, jobject exceptionObj)
{
    LOGD("JSObjectMakeFunction");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    JSSTRINGREF_FROM_JSTRING(body, jsbody)
    JSSTRINGREF_FROM_JSTRING(sourceURL, jsSourceURL)
    
    JSStringRef* jsParamNames = NULL;
    if (paramCount > 0) {
        jsParamNames = (JSStringRef*)malloc(sizeof(JSStringRef) * paramCount);
        for (int i = 0; i < paramCount; i++) {
            jobject paramName = (*env)->GetObjectArrayElement(env, paramNames, i);
            JSSTRINGREF_FROM_JSTRING(paramName, jsParamName);
            jsParamNames[i] = jsParamName;
            (*env)->DeleteLocalRef(env, paramName);
        }
    }
    
    JSObjectRef value = JSObjectMakeFunction(ctx, jsname, paramCount, jsParamNames, jsbody, jsSourceURL, line, &exceptionStore);
    
    JSSTRING_RELEASE(jsname);
    JSSTRING_RELEASE(jsbody);
    JSSTRING_RELEASE(jsSourceURL);
    for (int i = 0; i < paramCount; i++) {
        JSSTRING_RELEASE(jsParamNames[i]);
    }
    free((void*)jsParamNames);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        UpdateJSValueExceptionPointer(env, ctx, exceptionStore, exceptionObj);
    }

    return (jlong)value;
}

/*!
@function
@abstract Convenience method for creating a JavaScript constructor.
@param ctx The execution context to use.
@param jsClass A JSClass that is the class your constructor will assign to the objects its constructs. jsClass will be used to set the constructor's .prototype property, and to evaluate 'instanceof' expressions. Pass NULL to use the default object class.
@param callAsConstructor A JSObjectCallAsConstructorCallback to invoke when your constructor is used in a 'new' expression. Pass NULL to use the default object constructor.
@result A JSObject that is a constructor. The object's prototype will be the default object prototype.
@discussion The default object constructor takes no arguments and constructs an object of class jsClass with no private data.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsClassRef, jobject callback, jboolean useCallback)
{
    LOGD("JSObjectMakeConstructor");
    return (jlong)JSObjectMakeConstructor((JSContextRef)jsContextRef, (JSClassRef)jsClassRef,
                                          NativeCallback_JSObjectMakeConstructorCallback);
}

/*!
@function
@abstract Convenience method for creating a JavaScript function with a given callback as its implementation.
@param ctx The execution context to use.
@param name A JSString containing the function's name. This will be used when converting the function to string. Pass NULL to create an anonymous function.
@param callAsFunction The JSObjectCallAsFunctionCallback to invoke when the function is called.
@result A JSObject that is a function. The object's prototype will be the default function prototype.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeFunctionWithCallback
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring name)
{
    LOGD("JSObjectMakeFunctionWithCallback");
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    JSObjectRef function = JSObjectMakeFunctionWithCallback((JSContextRef)jsContextRef, jsname,
                                          NativeCallback_JSObjectMakeFunctionCallback);
    JSSTRING_RELEASE(jsname);
    return (jlong)function;
}

/*
 * Return pointers to JavaScript static functions for specific object
 */
JNIEXPORT jlongArray JNICALL
Java_com_appcelerator_javascriptcore_opaquetypes_JSClassDefinition_NativeGetStaticFunctions
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint staticFunctionCount, jobject staticFunctionsBuffer)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    jlongArray outValues = (*env)->NewLongArray(env, staticFunctionCount);
    jlong* p_outValues = (*env)->GetLongArrayElements(env, outValues, NULL);
    if (staticFunctionCount > 0) {
        JSStaticFunction* staticFunctions = (*env)->GetDirectBufferAddress(env, staticFunctionsBuffer);
        int i = 0;
        while(staticFunctions->name) {
            JSStringRef funcname = JSStringCreateWithUTF8CString(staticFunctions->name);
            JSValueRef  funcval  = JSObjectGetProperty(ctx, object, funcname, NULL);
            if (!JSValueIsUndefined(ctx, funcval)) {
                p_outValues[i] = (jlong)JSValueToObject(ctx, funcval, NULL);
            }
            JSSTRING_RELEASE(funcname);
            ++staticFunctions;
            ++i;
        }
    }
    (*env)->ReleaseLongArrayElements(env, outValues, p_outValues, 0);
    
    return outValues;
}

#ifdef ENABLE_JAVASCRIPTCORE_PRIVATE_API
/*
 * Private APIs
 */
/*!
@function
@abstract Gets the global context of a JavaScript execution context.
@param ctx The JSContext whose global context you want to get.
@result ctx's global context.
*/
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGetGlobalContext
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSContextGetGlobalContext");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSContextGetGlobalContext(ctx);
}

/*!
@function
@abstract Gets a Backtrace for the existing context
@param ctx The JSContext whose backtrace you want to get
@result A string containing the backtrace
*/
JNIEXPORT jstring JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextCreateBacktrace
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint maxStackSize)
{
    LOGD("JSContextCreateBacktrace");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSStringRef trace = JSContextCreateBacktrace(ctx, (unsigned)maxStackSize);
    JSTRING_FROM_JSSTRINGREF(trace, ctrace, jtrace);
    JSSTRING_RELEASE(trace);
    return jtrace;
}

/*!
 @function
 @abstract Sets a private property on an object.  This private property cannot be accessed from within JavaScript.
 @param ctx The execution context to use.
 @param object The JSObject whose private property you want to set.
 @param propertyName A JSString containing the property's name.
 @param value A JSValue to use as the property's value.  This may be NULL.
 @result true if object can store private data, otherwise false.
 @discussion This API allows you to store JS values directly an object in a way that will be ensure that they are kept alive without exposing them to JavaScript code and without introducing the reference cycles that may occur when using JSValueProtect.

 The default object class does not allocate storage for private data. Only objects created with a non-NULL JSClass can store private properties.
 */
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetPrivateProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring propertyName, jlong jsValueRef)
{
    LOGD("JSObjectSetPrivateProperty");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    JSSTRINGREF_FROM_JSTRING(propertyName, jsname)
    jboolean result = JSObjectSetPrivateProperty(ctx, object, jsname, value) ? JNI_TRUE : JNI_FALSE;
    JSSTRING_RELEASE(jsname);
    return result;
}

/*!
 @function
 @abstract Gets a private property from an object.
 @param ctx The execution context to use.
 @param object The JSObject whose private property you want to get.
 @param propertyName A JSString containing the property's name.
 @result The property's value if object has the property, otherwise NULL.
 */
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPrivateProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring propertyName)
{
    LOGD("JSObjectGetPrivateProperty");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    JSSTRINGREF_FROM_JSTRING(propertyName, jsname)
    JSValueRef value = JSObjectGetPrivateProperty(ctx, object, jsname);
    JSSTRING_RELEASE(jsname);
    return (jlong)value;
}

/*!
 @function
 @abstract Deletes a private property from an object.
 @param ctx The execution context to use.
 @param object The JSObject whose private property you want to delete.
 @param propertyName A JSString containing the property's name.
 @result true if object can store private data, otherwise false.
 @discussion The default object class does not allocate storage for private data. Only objects created with a non-NULL JSClass can store private data.
 */
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectDeletePrivateProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring propertyName)
{
    LOGD("JSObjectDeletePrivateProperty");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    JSSTRINGREF_FROM_JSTRING(propertyName, jsname)
    jboolean result = JSObjectDeletePrivateProperty(ctx, object, jsname) ? JNI_TRUE : JNI_FALSE;
    JSSTRING_RELEASE(jsname);
    return result;
}
#endif

#ifdef __cplusplus
}
#endif