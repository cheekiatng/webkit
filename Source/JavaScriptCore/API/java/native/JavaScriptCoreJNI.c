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

#ifdef __ANDROID__
#include <stdarg.h>
#include <android/log.h>
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, JSCORE_LOG_TAG, __VA_ARGS__));
#else
#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__));fflush(stdout);
#define LOGI(...) ((void)fprintf(stdout, __VA_ARGS__));fflush(stdout);
#define LOGW(...) ((void)fprintf(stdout, __VA_ARGS__));fflush(stdout);
#define LOGE(...) ((void)fprintf(stdout, __VA_ARGS__));fflush(stdout);
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
const char* aschars##varin = (*env)->GetStringUTFChars(env, varin, NULL);\
JSStringRef varout = JSStringCreateWithUTF8CString(aschars##varin);\
(*env)->ReleaseStringUTFChars(env, varin, aschars##varin);

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
static JSStaticValue JSStaticEmptyValue = {0,0,0,0};
static JSStaticFunction JSStaticEmptyFunction = {0,0,0};

jmethodID jmethodId_JSObjectInitializeCallback = NULL;
jmethodID jmethodId_JSObjectFinalizeCallback = NULL;
jmethodID jmethodId_JSObjectGetStaticValueCallback = NULL;
jmethodID jmethodId_JSObjectSetStaticValueCallback = NULL;
jmethodID jmethodId_JSObjectGetPropertyCallback = NULL;
jmethodID jmethodId_JSObjectSetPropertyCallback = NULL;
jmethodID jmethodId_JSObjectCallAsConstructorCallback = NULL;
jmethodID jmethodId_JSObjectCallAsFunctionCallback = NULL;
jmethodID jmethodId_JSObjectConvertToTypeCallback = NULL;
jmethodID jmethodId_JSObjectDeletePropertyCallback = NULL;
jmethodID jmethodId_JSObjectGetPropertyNamesCallback = NULL;
jmethodID jmethodId_JSObjectHasInstanceCallback = NULL;
jmethodID jmethodId_JSObjectHasPropertyCallback = NULL;

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
    }
    return true;
}

/*
 * Throw JavaScriptCore exception from JSValueRef exception
 */
static void ThrowJavaScriptCoreException(JNIEnv* env, JSContextRef ctx, JSValueRef exValue, jobject thisObject) {
    // if Java object equals null, it's completely ok to ignore it
    if (thisObject == NULL) return;
    
    jclass javaExceptionClass = (*env)->FindClass(env, "com/appcelerator/javascriptcore/JavaScriptException");
    if (javaExceptionClass == NULL) return;
    
    JSStringRef exString = JSValueToStringCopy(ctx, exValue, NULL);
    
    if (exString == NULL) {
        (*env)->ThrowNew(env, javaExceptionClass, "Unknown JavaScriptCore Exception");
    } else {
        CCHAR_FROM_JSSTRINGREF(exString, exChars)
        (*env)->ThrowNew(env, javaExceptionClass, exChars);
        JSStringRelease(exString);
        free(exChars);
    }
    
    return;
}

/*
 * JavaScriptCore Callbacks
 */
static void NativeCallback_JSObjectInitializeCallback(JSContextRef ctx, JSObjectRef object)
{
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        (*env)->CallVoidMethod(env, prv->callback,
                               jmethodId_JSObjectInitializeCallback, (jlong)ctx, (jlong)object);
    }
    JNI_ENV_EXIT
}

static void NativeCallback_JSObjectFinalizeCallback(JSObjectRef object)
{
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
    JSValueRef value = NULL;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback,
                               jmethodId_JSObjectGetStaticValueCallback,
                               (jlong)ctx, (jlong)object, jname, (jlong)*exception);
    }
    JNI_ENV_EXIT
    
    return value;
}

static bool NativeCallback_JSObjectSetStaticValueCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* exception)
{
    bool result = false;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        result = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectSetStaticValueCallback,
            (jlong)ctx, (jlong)object, jname, (jlong)value, (jlong)*exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT

    return result;
}
    
static JSValueRef NativeCallback_JSObjectGetPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef* exception)
{
    JSValueRef value = NULL;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback,
                               jmethodId_JSObjectGetPropertyCallback,
                               (jlong)ctx, (jlong)object, jname, (jlong)*exception);
    }
    JNI_ENV_EXIT
    
    return value;
}

static bool NativeCallback_JSObjectSetPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* exception)
{
    bool result = false;
    
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        result = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectSetPropertyCallback,
            (jlong)ctx, (jlong)object, jname, (jlong)value, (jlong)*exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT

    return result;
}

static JSObjectRef NativeCallback_JSObjectCallAsConstructorCallback(
    JSContextRef ctx, JSObjectRef constructor,
    size_t argc, const JSValueRef argv[], JSValueRef *exception)
{
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(constructor);
    if (prv && prv->callback)
    {
        jobject argvbuffer = argc > 0 ? (*env)->NewDirectByteBuffer(env, (void*)&argv[0], sizeof(long) * argc) : NULL;
        constructor = (JSObjectRef)(*env)->CallLongMethod(env, prv->callback,
                            jmethodId_JSObjectCallAsConstructorCallback,
                            (jlong)ctx, (jlong)constructor, (jint)argc, argvbuffer, (jlong)*exception);
    }
    JNI_ENV_EXIT

    return constructor;
}

static JSValueRef NativeCallback_JSObjectCallAsFunctionCallback(
    JSContextRef ctx, JSObjectRef func, JSObjectRef thisObject,
    size_t argc, const JSValueRef argv[], JSValueRef* exception) {
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(func);
    if (prv == NULL) prv = (JSObjectPrivateData*)JSObjectGetPrivate(thisObject);
    if (prv && prv->callback)
    {
        jobject argvbuffer = argc > 0 ? (*env)->NewDirectByteBuffer(env, (void*)&argv[0], sizeof(long) * argc) : NULL;
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback, jmethodId_JSObjectCallAsFunctionCallback,
                               (jlong)ctx, (jlong)func, (jlong)thisObject, (jint)argc, argvbuffer, (jlong)*exception);
    }
    JNI_ENV_EXIT
    return value;
}

static JSValueRef NativeCallback_JSObjectConvertToTypeCallback(
   JSContextRef ctx,JSObjectRef object,
   JSType type, JSValueRef *exception)
{
    JSValueRef value = NULL;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        value = (JSValueRef)(*env)->CallLongMethod(env, prv->callback, jmethodId_JSObjectConvertToTypeCallback,
                                                   (jlong)ctx, (jlong)object, (jint)type, (jlong)*exception);
    }
    JNI_ENV_EXIT
    return value;
}
    
static bool NativeCallback_JSObjectDeletePropertyCallback(
    JSContextRef ctx, JSObjectRef object,
    JSStringRef name, JSValueRef *exception)
{
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(object);
    if (prv && prv->callback)
    {
        JSTRING_FROM_JSSTRINGREF(name, cname, jname)
        value = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectDeletePropertyCallback,
                    (jlong)ctx, (jlong)object, jname, (jlong)*exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT
    return value;
}

static void NativeCallback_JSObjectGetPropertyNamesCallback(
    JSContextRef ctx, JSObjectRef object,
    JSPropertyNameAccumulatorRef propertyNames)
{
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
    bool value = false;
    JNI_ENV_ENTER
    JSObjectPrivateData* prv = (JSObjectPrivateData*)JSObjectGetPrivate(constructor);
    if (prv && prv->callback)
    {
        value = (*env)->CallBooleanMethod(env, prv->callback, jmethodId_JSObjectHasInstanceCallback,
                    (jlong)ctx, (jlong)constructor, (jlong)instance, (jlong)*exception) == JNI_TRUE ? true : false;
    }
    JNI_ENV_EXIT
    return value;
}

static bool NativeCallback_JSObjectHasPropertyCallback(
    JSContextRef ctx, JSObjectRef object, JSStringRef name)
{
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
static JSClassDefinition jsClassDefinitionTemplate;
static JSStaticValue     jsStaticValueTemplate;
static JSStaticFunction  jsStaticFunctionTemplate;

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
    jsStaticFunctionTemplate.callAsFunction = NativeCallback_JSObjectCallAsFunctionCallback;
    
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
        const char* classNameAsChars = (*env)->GetStringUTFChars(env, className, NULL);
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
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassCreate_
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

JNIEXPORT jlongArray JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMake
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsClassRef,
     jobject callback, jobject staticFunctionsBuffer, jint staticFunctionCount, jobject object)
{
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
    
    JSObjectRef newObject = JSObjectMake(ctx, jsClass, prv);
    
    jlongArray outValues = (*env)->NewLongArray(env, staticFunctionCount + 1);
    jlong* p_outValues = (*env)->GetLongArrayElements(env, outValues, NULL);
    
    // First one is the new object pointer, static function pointers to follow
    p_outValues[0] = (jlong)newObject;

    // Attach callback to function object
    if (staticFunctionCount > 0) {
        JSStaticFunction* staticFunctions = (*env)->GetDirectBufferAddress(env, staticFunctionsBuffer);
        int i = 1;
        while(staticFunctions->name) {
            JSStringRef funcname = JSStringCreateWithUTF8CString(staticFunctions->name);
            JSValueRef  funcval  = JSObjectGetProperty(ctx, newObject, funcname, NULL);
            if (!JSValueIsUndefined(ctx, funcval)) {
                JSObjectRef funcObj = JSValueToObject(ctx, funcval, NULL);
                JSObjectSetPrivate(funcObj, prv);
                p_outValues[i] = (jlong)funcObj;
            }
            JSStringRelease(funcname);
            ++staticFunctions;
            ++i;
        }
    }
    (*env)->ReleaseLongArrayElements(env, outValues, p_outValues, 0);
    
    return outValues;
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
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring script)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    const char* scriptchars = (*env)->GetStringUTFChars(env, script, NULL);
    JSStringRef scriptJS = JSStringCreateWithUTF8CString(scriptchars);
    (*env)->ReleaseStringUTFChars(env, script, scriptchars);
    
    JSValueRef result = JSEvaluateScript(ctx, scriptJS, NULL, NULL, 1, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    
    return (jlong)result;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSCheckScriptSyntax
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring script)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    const char* scriptchars = (*env)->GetStringUTFChars(env, script, NULL);
    JSStringRef scriptJS = JSStringCreateWithUTF8CString(scriptchars);
    (*env)->ReleaseStringUTFChars(env, script, scriptchars);
    
    if (JSCheckScriptSyntax(ctx, scriptJS, NULL, 1, &exceptionStore)) {
        return (jlong)JSValueMakeBoolean(ctx, true);
    } else {
        if (!JSValueIsNull(ctx, exceptionStore)) {
            return (jlong)exceptionStore;
        } else {
            return (jlong)JSValueMakeBoolean(ctx, false);
        }
    }
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
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jlong jsObjectRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSObjectRef constructor = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    jboolean result = JSValueIsInstanceOfConstructor(ctx, value, constructor, &exceptionStore) ? JNI_TRUE : JNI_FALSE;
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return result;
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
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    double result = JSValueToNumber(ctx, value, &exceptionStore);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return (jdouble)result;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSValueToObject
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSObjectRef result = JSValueToObject(ctx, value, &exceptionStore);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return (jlong)result;
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
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSStringRef jsstring = JSValueToStringCopy(ctx, value, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
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
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRefA, jlong jsValueRefB)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef a = (JSValueRef)jsValueRefA;
    JSValueRef b = (JSValueRef)jsValueRefB;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    jboolean result = JSValueIsEqual(ctx, a, b, &exceptionStore) ? JNI_TRUE : JNI_FALSE;
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return result;
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
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsValueRef, jint indent)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSStringRef jsstring = JSValueCreateJSONString(ctx, value, indent, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
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
    JSValueRef string = JSValueMakeString(ctx, jsvalue);
    JSStringRelease(jsvalue);
    return (jlong)string;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCallAsConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint argc, jobject argv)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    const JSValueRef* js_argv = NULL;
    if (argc > 0) js_argv = (*env)->GetDirectBufferAddress(env, argv);
    JSObjectRef value = JSObjectCallAsConstructor(ctx, object, argc, js_argv, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
   
    return (jlong)value;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCallAsFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jlong jsThisObjectRef,
     jint argc, jobject argv)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSObjectRef thisObject = (JSObjectRef)jsThisObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    const JSValueRef* js_argv = NULL;
    if (argc > 0) js_argv = (*env)->GetDirectBufferAddress(env, argv);
    JSValueRef value = JSObjectCallAsFunction(ctx, object, thisObject, argc, js_argv, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    
    return (jlong)value;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring name,
     jlong jsValueRef, jint attributes)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    JSObjectSetProperty(ctx, object, jsname, value, attributes, &exceptionStore);
    JSStringRelease(jsname);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef,
     jstring name)
{
    JSGlobalContextRef ctx = (JSGlobalContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSSTRINGREF_FROM_JSTRING(name, jsname)

    JSValueRef value = JSObjectGetProperty(ctx, object, jsname, &exceptionStore);
    JSStringRelease(jsname);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    
    return (jlong)value;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassRelease
    (JNIEnv *env, jobject thiz, jlong jsClassRef)
{
    JSClassRelease((JSClassRef)jsClassRef);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSClassRetain
    (JNIEnv *env, jobject thiz, jlong jsClassRef)
{
    return (jlong)JSClassRetain((JSClassRef)jsClassRef);
}

JNIEXPORT jint JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayGetCount
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    return (jint)JSPropertyNameArrayGetCount(array);
}

JNIEXPORT jstring JNICALL
NativeJSPropertyNameArrayGetNameAtIndex
    (JNIEnv *env, jobject thiz, jlong namesRef, jint index)
{
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    JSStringRef name = JSPropertyNameArrayGetNameAtIndex(array, index);
    JSTRING_FROM_JSSTRINGREF(name, cname, jname)
    return jname;
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayRelease
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    JSPropertyNameArrayRelease(array);
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameArrayRetain
    (JNIEnv *env, jobject thiz, jlong namesRef)
{
    JSPropertyNameArrayRef array = (JSPropertyNameArrayRef)namesRef;
    return (jlong)JSPropertyNameArrayRetain(array);
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSPropertyNameAccumulatorAddName
    (JNIEnv *env, jobject thiz, jlong accumulatorRef, jstring name)
{
    JSPropertyNameAccumulatorRef accumulator = (JSPropertyNameAccumulatorRef)accumulatorRef;
    JSSTRINGREF_FROM_JSTRING(name, jsname)

    JSPropertyNameAccumulatorAddName(accumulator, jsname);
    JSStringRelease(jsname);
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetPropertyAtIndex
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint propertyIndex, jlong jsValueRef)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSObjectSetPropertyAtIndex(ctx, object, propertyIndex, value, &exceptionStore);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
}

JNIEXPORT void JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectSetPrototype
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jlong jsValueRef)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef value = (JSValueRef)jsValueRef;
    
    JSObjectSetPrototype(ctx, object, value);
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectIsConstructor
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return JSObjectIsConstructor(ctx, object) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectIsFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return JSObjectIsFunction(ctx, object) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectCopyPropertyNames
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return (jlong)JSObjectCopyPropertyNames(ctx, object);
}

JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectDeleteProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring name)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    bool value = JSObjectDeleteProperty(ctx, object, jsname, &exceptionStore);
    JSStringRelease(jsname);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return value ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPropertyAtIndex
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jint index)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSValueRef result = JSObjectGetPropertyAtIndex(ctx, object, index, &exceptionStore);
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return (jlong)result;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectGetPrototype
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    
    return (jlong)JSObjectGetPrototype(ctx, object);
}
    
JNIEXPORT jboolean JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectHasProperty
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jlong jsObjectRef, jstring name)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSObjectRef object = (JSObjectRef)jsObjectRef;
    JSSTRINGREF_FROM_JSTRING(name, jsname)
    bool value = JSObjectHasProperty(ctx, object, jsname);
    JSStringRelease(jsname);
    return value ? JNI_TRUE : JNI_FALSE;
}
    
JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeArray
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeArray, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return (jlong)value;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeDate
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeDate, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return (jlong)value;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeError
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeError, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }
    return (jlong)value;
}

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeRegExp
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jint argc, jobject argv)
{
    JSContextRef ctx = (JSContextRef)jsContextRef;
    JSValueRef exceptionStore = JSValueMakeNull(ctx);
    
    JSOBJECTMAKE_FROM_ARGV(JSObjectMakeRegExp, argc, argv, value)
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
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

JNIEXPORT jlong JNICALL
Java_com_appcelerator_javascriptcore_JavaScriptCoreLibrary_NativeJSObjectMakeFunction
    (JNIEnv *env, jobject thiz, jlong jsContextRef, jstring name, jint paramCount,
     jobjectArray paramNames, jstring body, jstring sourceURL, jint line)
{
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
    
    JSStringRelease(jsname);
    JSStringRelease(jsbody);
    JSStringRelease(jsSourceURL);
    for (int i = 0; i < paramCount; i++) {
        JSStringRelease(jsParamNames[i]);
    }
    free((void*)jsParamNames);
    
    if (!JSValueIsNull(ctx, exceptionStore)) {
        ThrowJavaScriptCoreException(env, ctx, exceptionStore, thiz);
    }

    return (jlong)value;
}

#ifdef __cplusplus
}
#endif