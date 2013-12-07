//
//  JavaScriptCoreForJNI.h
//  JavaScriptCoreJNI
//
//  Created by Kota Iguchi on 12/5/13.
//  Copyright (c) 2013 Appcelerator, Inc. All rights reserved.
//

#ifndef JavaScriptCoreJNI_JavaScriptCoreForJNI_h
#define JavaScriptCoreJNI_JavaScriptCoreForJNI_h

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

/* Create char* from JSStringRef */
#define CCHAR_FROM_JSSTRINGREF(varin, varout)\
size_t length##varin = JSStringGetMaximumUTF8CStringSize(varin);\
char* varout = (char*)malloc(length##varin);\
JSStringGetUTF8CString(varin, varout, length##varin);\

/* Create jstring from JSStringRef */
#define JSTRING_FROM_JSSTRINGREF(varin, varcout, varjout)\
CCHAR_FROM_JSSTRINGREF(varin, varcout);\
jstring varjout = (*env)->NewStringUTF(env, varcout);

#define JSSTRINGREF_FROM_JSTRING(varin, varout)\
const char* aschars##varin = (*env)->GetStringUTFChars(env, varin, NULL);\
JSStringRef varout = JSStringCreateWithUTF8CString(aschars##varin);\
(*env)->ReleaseStringUTFChars(env, varin, aschars##varin);

/* Private object for JSObjectRef */
typedef struct {
    // JSClassDefinition which holds callbacks
    jobject definition;
    jclass  definitionClass;
    // Java Object associated with jsobject
    jobject object;
} JSObjectPrivateData;

#endif
