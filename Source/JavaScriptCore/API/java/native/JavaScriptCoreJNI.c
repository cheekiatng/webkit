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

JNIEXPORT jobject JNICALL
Java_com_appcelerator_javascriptcore_opaquetypes_JSStaticValues_NativeGetStaticValueTemplate
    (JNIEnv *env, jclass clazz)
{
    jsStaticValueTemplate.attributes = kJSPropertyAttributeNone;
    jsStaticValueTemplate.getProperty = NativeCallback_JSObjectGetStaticValueCallback;
    jsStaticValueTemplate.setProperty = NativeCallback_JSObjectSetStaticValueCallback;
    
    return (*env)->NewDirectByteBuffer(env, &jsStaticValueTemplate, sizeof(JSStaticValue));
}

JNIEXPORT jobject JNICALL
Java_com_appcelerator_javascriptcore_opaquetypes_JSStaticFunctions_NativeGetStaticFunctionTemplate
    (JNIEnv *env, jclass clazz)
{
    jsStaticFunctionTemplate.attributes = kJSPropertyAttributeNone;
    jsStaticFunctionTemplate.callAsFunction = NativeCallback_JSObjectStaticFunctionCallback;
    
    return (*env)->NewDirectByteBuffer(env, &jsStaticFunctionTemplate, sizeof(JSStaticFunction));
}

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

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupCreate
    (JNIEnv *env, jobject thiz)
{
    LOGD("JSContextGroupCreate");
    JSContextGroupRef group = JSContextGroupCreate();
    return (jlong)group;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupRelease
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef)
{
    LOGD("JSContextGroupRelease");
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    JSContextGroupRelease(group);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGroupRetain
    (JNIEnv *env, jobject thiz, jlong jsContextGroupRef)
{
    LOGD("JSContextGroupRetain");
    JSContextGroupRef group = (JSContextGroupRef)jsContextGroupRef;
    return (jlong)JSContextGroupRetain(group);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGetGlobalObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSContextGetGlobalObject");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSContextGetGlobalObject(ctx);
}
    
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSContextGetGroup
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSContextGetGroup");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSContextGetGroup(ctx);
}

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

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextRelease
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSGlobalContextRelease");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSGlobalContextRelease(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGlobalContextRetain
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSGlobalContextRetain");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSGlobalContextRetain(ctx);
}

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

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSGarbageCollect
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSGarbageCollect");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSGarbageCollect(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeUndefined
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSValueMakeUndefined");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeUndefined(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeNull
    (JNIEnv *env, jobject thiz, jlong jsContextRef)
{
    LOGD("JSValueMakeNull");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeNull(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jdouble arg)
{
    LOGD("JSValueMakeNumber");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeNumber(ctx, (double)arg);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueMakeBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jboolean arg)
{
    LOGD("JSValueMakeBoolean");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    return (jlong)JSValueMakeBoolean(ctx, arg == JNI_TRUE ? true : false);
}

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

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsUndefined
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsUndefined");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsUndefined(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsNull
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsNull");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsNull(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsNumber
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsNumber");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsNumber(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsBoolean");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsBoolean(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsString
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsString");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsString(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueIsObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueIsObject");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    return JSValueIsObject(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToBoolean
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueToBoolean");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    return JSValueToBoolean(ctx, value) ? JNI_TRUE : JNI_FALSE;
}

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

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueProtect
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueProtect");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueProtect(ctx, value);
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueUnprotect
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueUnprotect");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueUnprotect(ctx, value);
}

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

JNIEXPORT jint JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueGetType
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    LOGD("JSValueGetType");
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    return (jint)JSValueGetType(ctx, value);
}

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

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassRelease
    (JNIEnv *env, jobject thiz, jlong jsClassRef)
{
    LOGD("JSClassRelease");
    JSClassRelease((JSClassRef)jsClassRef);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassRetain
    (JNIEnv *env, jobject thiz, jlong jsClassRef)
{
    LOGD("JSClassRetain");
    return (jlong)JSClassRetain((JSClassRef)jsClassRef);
}

JNIEXPORT jint JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayGetCount
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    LOGD("JSPropertyNameArrayGetCount");
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    return (jint)JSPropertyNameArrayGetCount(array);
}

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

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayRelease
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    LOGD("JSPropertyNameArrayRelease");
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    JSPropertyNameArrayRelease(array);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayRetain
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    LOGD("JSPropertyNameArrayRetain");
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    return (jlong)JSPropertyNameArrayRetain(array);
}

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

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectIsConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectIsConstructor");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return JSObjectIsConstructor(ctx, object) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectIsFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectIsFunction");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return JSObjectIsFunction(ctx, object) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCopyPropertyNames
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectCopyPropertyNames");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return (jlong)JSObjectCopyPropertyNames(ctx, object);
}

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

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPrototype
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    LOGD("JSObjectGetPrototype");
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return (jlong)JSObjectGetPrototype(ctx, object);
}
    
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

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsClassRef, jobject callback, jboolean useCallback)
{
    LOGD("JSObjectMakeConstructor");
    return (jlong)JSObjectMakeConstructor((JSContextRef)jsContextRef, (JSClassRef)jsClassRef,
                                          NativeCallback_JSObjectMakeConstructorCallback);
}

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
#ifdef __cplusplus
}
#endif