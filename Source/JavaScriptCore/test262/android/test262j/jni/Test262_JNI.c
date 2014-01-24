/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string.h>
#include <jni.h>

#include <sys/types.h> // need for off_t for asset_manager because it looks like r9b broke something.
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_INFO,  "Test262", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR,  "Test262", __VA_ARGS__))

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */

// Declare here from test262.c because I didn't make a header file.
extern _Bool Test262_EvaluateStringScript(JSStringRef js_script_string, JSStringRef js_file_name, JSStringRef* out_error_string, JSStringRef* out_stack_string);


//static jobject s_proxyObject;
static JavaVM* s_javaVm;

jint JNI_OnLoad(JavaVM* java_vm, void* reserved)
{
	s_javaVm = java_vm;
	return JNI_VERSION_1_6;
}


/* Returns a const char* which must be freed by you when done */
static const char* jstringToC(JNIEnv* jni_env, jstring original_jstring)
{
	const char* c_string = (*jni_env)->GetStringUTFChars(jni_env, original_jstring, 0);
	const char* duplicated_string = strdup(c_string);
	(*jni_env)->ReleaseStringUTFChars(jni_env, original_jstring, c_string);
	return duplicated_string;
}

/* Returns a JSStringRef which must be released by you when done */
static JSStringRef jstringToJSStringRef(JNIEnv* jni_env, jstring original_jstring)
{
	JSStringRef return_jsstringref;
	const char* c_string = (*jni_env)->GetStringUTFChars(jni_env, original_jstring, 0);
	return_jsstringref = JSStringCreateWithUTF8CString(c_string);
	(*jni_env)->ReleaseStringUTFChars(jni_env, original_jstring, c_string);
	return return_jsstringref;
}

static jstring jstringFromJSStringRef(JNIEnv* jni_env, JSStringRef original_jsstringref)
{
	jstring return_jstring;
	size_t bytes_needed = JSStringGetMaximumUTF8CStringSize(original_jsstringref);

//	LOGD("result bytes_needed: %d\n", bytes_needed);

	char* c_result_string = (char*)calloc(bytes_needed, sizeof(char));
	JSStringGetUTF8CString(original_jsstringref, c_result_string, bytes_needed);

//	LOGD("c_result_string: %s\n", c_result_string);

    return_jstring =  (*jni_env)->NewStringUTF(jni_env, c_result_string);
	free(c_result_string);
	return return_jstring;
}

static jstring jstringFromC(JNIEnv* jni_env, const char* original_c_string)
{
	jstring return_jstring;
    return_jstring =  (*jni_env)->NewStringUTF(jni_env, original_c_string);
	return return_jstring;
}

JNIEXPORT jboolean JNICALL Java_org_webkit_javascriptcore_test262_Test262_doInit(JNIEnv* env, jobject thiz, jobject java_asset_manager)
{
	LOGD("Java_org_webkit_javascriptcore_test262_Test262_doInit");

	// I'm not sure if I need to
	AAssetManager* ndk_asset_manager = AAssetManager_fromJava(env, java_asset_manager);
	/* Saving the object instance so it can be used for calling back into Java. 
	 * I think I need to call NewGlobalRef on it because it must live past this function,
	 * otherwise I get the error: 
	 * JNI ERROR (app bug): accessed stale local reference
	 */
//	s_proxyObject = (*env)->NewGlobalRef(env, thiz);


	return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_org_webkit_javascriptcore_test262_Test262_doPause(JNIEnv* env, jobject thiz)
{
	LOGD("Java_org_webkit_javascriptcore_test262_Test262_doPause");
}

JNIEXPORT void JNICALL Java_org_webkit_javascriptcore_test262_Test262_doResume(JNIEnv* env, jobject thiz)
{
	LOGD("Java_org_webkit_javascriptcore_test262_Test262_doResume");
}

JNIEXPORT void JNICALL Java_org_webkit_javascriptcore_test262_Test262_doDestroy(JNIEnv* env, jobject thiz)
{
	LOGD("Java_org_webkit_javascriptcore_test262_Test262_doDestroy");

	/* Release the proxy object. */
//	(*env)->DeleteGlobalRef(env, s_proxyObject);
//	s_proxyObject = NULL;

	LOGD("Java_org_webkit_javascriptcore_test262_Test262_doDestroy end");

}


JNIEXPORT jboolean JNICALL Java_org_webkit_javascriptcore_test262_Test262_evaluateScript(JNIEnv* jni_env, jobject thiz, jstring java_string_script, jstring java_file_name, jobject java_return_data_object)
{
	/*
const char* c_str = jstringToC(jni_env, java_string_script);
LOGD("going to run script:\n%s\n<end>\n", c_str);
free((void*)c_str);
c_str = NULL;
*/

//	JSStringRef override_script = JSStringCreateWithUTF8CString("function(){ do{ function __func(){}; }while(0);};");


	JSStringRef js_string_script = jstringToJSStringRef(jni_env, java_string_script);
	JSStringRef js_file_name = jstringToJSStringRef(jni_env, java_file_name);
	JSStringRef js_exception_string = NULL;
	JSStringRef js_stack_string = NULL;
	_Bool is_success;

	is_success = Test262_EvaluateStringScript(js_string_script, js_file_name, &js_exception_string, &js_stack_string);
//	is_success = Test262_EvaluateStringScript(override_script, js_file_name, &js_exception_string, &js_stack_string);

	if(!is_success)
	{
		// javap -classpath bin/classes -s -p org.webkit.javascriptcore.test262.ReturnDataObject
	    jclass return_data_object_class = (*jni_env)->GetObjectClass(jni_env, java_return_data_object);
		
		if(NULL != js_exception_string)
		{
			// Because we don't have multiple return values for JNI,
			// we will save any additional information in the ReturnDataObject instance 
			// by calling back into Java to set values which can be read after this function returns.
			jmethodID method_id = (*jni_env)->GetMethodID(jni_env, return_data_object_class, "setExceptionString", "(Ljava/lang/String;)V");
			jstring java_exception_string = jstringFromJSStringRef(jni_env, js_exception_string);
			(*jni_env)->CallVoidMethod(jni_env, java_return_data_object, method_id, java_exception_string);
		}

		if(NULL != js_stack_string)
		{
			// Because we don't have multiple return values for JNI,
			// we will save any additional information in the ReturnDataObject instance 
			// by calling back into Java to set values which can be read after this function returns.
			jmethodID method_id = (*jni_env)->GetMethodID(jni_env, return_data_object_class, "setStackString", "(Ljava/lang/String;)V");
			jstring java_stack_string = jstringFromJSStringRef(jni_env, js_stack_string);
			(*jni_env)->CallVoidMethod(jni_env, java_return_data_object, method_id, java_stack_string);
		}
	}
	
	if(NULL != js_stack_string)
	{
		JSStringRelease(js_stack_string);
	}
	if(NULL != js_exception_string)
	{
		JSStringRelease(js_exception_string);
	}
	JSStringRelease(js_file_name);
	JSStringRelease(js_string_script);
	
	return (jboolean)is_success;
}

