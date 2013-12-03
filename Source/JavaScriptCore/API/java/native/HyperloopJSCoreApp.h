//
//  HyperloopJSCoreApp.h
//  JSCoreFoundation
//
//  Created by Kota Iguchi on 11/30/13.
//  Copyright (c) 2013 Appcelerator, Inc. All rights reserved.
//

#ifndef JSCoreFoundation_JSCoreApp_h
#define JSCoreFoundation_JSCoreApp_h

#include <jni.h>
#include <JavaScriptCore/JavaScriptCore.h>

#define JSCORE_LOG_TAG "JSCoreFoundation"

#ifdef __ANDROID__
#include <stdarg.h>
#include <android/log.h>
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  JSCORE_LOG_TAG, __VA_ARGS__));
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, JSCORE_LOG_TAG, __VA_ARGS__));
#else
#define LOGD(...) ((void)fprintf(stdout, "[DEBUG] %s\n", __VA_ARGS__));fflush(stdout);
#define LOGI(...) ((void)fprintf(stdout, "[INFO] %s\n", __VA_ARGS__));fflush(stdout);
#define LOGW(...) ((void)fprintf(stdout, "[WARN] %s\n", __VA_ARGS__));fflush(stdout);
#define LOGE(...) ((void)fprintf(stdout, "[ERROR] %s\n", __VA_ARGS__));fflush(stdout);
#endif

#define CONSOLE_LOG_FROM_ARG(index, arg, func) \
CCHAR_FROM_ARG(index, arg)\
func(arg);\
free(arg);\
return JSValueMakeUndefined(context);

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

/* Create jstring from JSValueRef[] argv */
#define JSTRING_FROM_ARG(index, varname)\
JSStringRef jsstring##index = JSValueToStringCopy(context, argv[index], exception);\
size_t jsstringlength##index = JSStringGetMaximumUTF8CStringSize(jsstring##index);\
char* jsstringlengthaschars##index = (char*)malloc(jsstringlength##index);\
JSStringGetUTF8CString(jsstring##index, jsstringlengthaschars##index, jsstringlength##index);\
JSStringRelease(jsstring##index);\
jstring varname = (*env)->NewStringUTF(env, jsstringlengthaschars##index);\
free(jsstringlengthaschars##index);

/* Create char* from JSValueRef[] argv */
#define CCHAR_FROM_ARG(index, varname)\
JSStringRef jsstring##index = JSValueToStringCopy(context, argv[index], exception);\
size_t jsstringlength##index = JSStringGetMaximumUTF8CStringSize(jsstring##index);\
char* varname = (char*)malloc(jsstringlength##index);\
JSStringGetUTF8CString(jsstring##index, varname, jsstringlength##index);\
JSStringRelease(jsstring##index);

/* Create JSStringRef from jstring */
#define JSSTRINGREF_FROM_JSTRING(inname, outname) \
const char* chars_from_jstring = (*env)->GetStringUTFChars(env, inname, 0);\
JSStringRef outname = JSStringCreateWithUTF8CString(chars_from_jstring);\
(*env)->ReleaseStringUTFChars(env, inname, chars_from_jstring);

typedef struct {
    jobject object;
} Hyperloop_JSObjectPrivateData;

bool JSCoreApp_Load(JavaVM* vm, JNIEnv* env, jclass invokerClass, jobject invokerObj,
                    JSGlobalContextRef context, JSObjectRef global_object);

#endif
