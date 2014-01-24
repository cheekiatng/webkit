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

// for close()
#include <unistd.h>


#include "Test262HelperAndroid.h"
#include "LogWrapper.h"
#include "SocketServer.h"
#include "SimpleThread.h"

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_INFO,  "Test262", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR,  "Test262", __VA_ARGS__))

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */

// used as the userdata for Test262Helper_RunTests callback functions
struct Test262AndroidUserDataForRunTests
{
	JNIEnv* jniEnv;
	jobject mainActivityInstance;
	jclass mainActivityClass;
	jmethodID methodIdCallbackForAllTestsStarting;
	jmethodID methodIdCallbackForAllBeginningTest;
	jmethodID methodIdCallbackForAllEndingTest;
	jmethodID methodIdCallbackForAllTestsFinished;
	jmethodID methodIdForGetShouldContinueRunning;
	int32_t javaScriptThreadShouldContinueRunning;
};

/*
struct Test262AndroidUserDataForRunTests* Test262JNI_CreateUserDataForRunTests(JNIEnv* jni_env, jlong main_activity_instance);
void Test262JNI_FreeUserDataForRunTests(struct Test262HelperAndroidUserDataForRunTests* user_data);

// callback functions for Test262Helper_RunTests
int32_t Test262JNI_CallbackForAllTestsStarting(uint32_t total_number_of_tests, void* user_data);
int32_t Test262JNI_CallbackForBeginningTest(const char* test_file, uint32_t total_number_of_tests, uint32_t current_test_number, void* user_data);
int32_t Test262JNI_CallbackForEndingTest(const char* test_file, uint32_t total_number_of_tests, uint32_t current_test_number, uint32_t current_test_number, uint32_t total_number_of_tests_failed, _Bool did_pass, const char* exception_string, const char* stack_string, void* user_data);
int32_t Test262JNI_allbackForAllTestsFinished(uint32_t total_number_of_tests, uint32_t number_of_tests_run, uint32_t total_number_of_tests_failed, void* user_data);
*/

// Declare here from test262.c because I didn't make a header file.
extern _Bool Test262_EvaluateStringScript(JSStringRef js_script_string, JSStringRef js_file_name, JSStringRef* out_error_string, JSStringRef* out_stack_string);



//static jobject s_proxyObject;
static JavaVM* s_javaVm;
static LogWrapper* s_logWrapper;

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


	s_logWrapper = LogWrapper_Create();

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

	LogWrapper_Free(s_logWrapper);
	s_logWrapper = NULL;

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

// Basically the JNI way to invoke:
// Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "test262_runlog.txt"; 
_Bool LogWrapperAndroid_OpenNewFile(JNIEnv* env, LogWrapper* log_wrapper)
{
	_Bool ret_val;
    jthrowable exception;	
    // Get File object for the external storage directory.
    jclass class_environment = (*env)->FindClass(env, "android/os/Environment");
    jmethodID method_getExternalStorageDirectory = (*env)->GetStaticMethodID(env, class_environment, "getExternalStorageDirectory", "()Ljava/io/File;"); // public static File getExternalStorageDirectory ()
    jobject object_file = (*env)->CallStaticObjectMethod(env, class_environment, method_getExternalStorageDirectory);
    exception = (*env)->ExceptionOccurred(env);
    if(exception)
	{
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }

    // Call method on File object to retrieve String object.
    jclass class_file = (*env)->GetObjectClass(env, object_file);
    jmethodID method_getAbsolutePath = (*env)->GetMethodID(env, class_file, "getAbsolutePath", "()Ljava/lang/String;");
    jstring j_string_path = (*env)->CallObjectMethod(env, object_file, method_getAbsolutePath);
    exception = (*env)->ExceptionOccurred(env);
    if (exception)
	{
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }
    // Extract a C string from the String object, and chdir() to it.
    const char* c_string_path = (*env)->GetStringUTFChars(env, j_string_path, NULL);

	const char* file_name = "test262_runlog.txt";

	// add 1 for separator and 1 for null terminator
	size_t total_string_length = strlen(c_string_path) + 1 + strlen(file_name) + 1;
	char* full_path_and_file = (char*)malloc(total_string_length*sizeof(char));
	strcpy(full_path_and_file, c_string_path);
	// I'm lazy. I probably should call File.separator, but I know '/' is fine.
	strcat(full_path_and_file, "/");
	strcat(full_path_and_file, file_name);

	ret_val = LogWrapper_OpenNewFileWithName(log_wrapper, full_path_and_file);

	free(full_path_and_file);

	(*env)->ReleaseStringUTFChars(env, j_string_path, c_string_path);
	return ret_val;

}


struct Test262AndroidUserDataForRunTests* Test262JNI_CreateUserDataForRunTests(JNIEnv* jni_env, jobject main_activity_instance)
{
	struct Test262AndroidUserDataForRunTests* user_data = (struct Test262AndroidUserDataForRunTests*)calloc(1, sizeof(struct Test262AndroidUserDataForRunTests));
	user_data->javaScriptThreadShouldContinueRunning = 1;
	// make sure this is the jnienv on the thread you are running in
	user_data->jniEnv = jni_env;
	// I don't know if NewLocalRef would work and be better than GlobalRef
	user_data->mainActivityInstance = (*jni_env)->NewGlobalRef(jni_env, main_activity_instance);
	jclass main_activity_class = (*jni_env)->GetObjectClass(jni_env, user_data->mainActivityInstance);
	user_data->mainActivityClass = (*jni_env)->NewGlobalRef(jni_env, main_activity_class);
			 	 
	// JNI doesn't have unsigned int. So promote everything to jlong?
	user_data->methodIdCallbackForAllTestsStarting = (*jni_env)->GetMethodID(jni_env, user_data->mainActivityClass, "callbackForAllTestsStarting", "(JJ)V");
	user_data->methodIdCallbackForAllBeginningTest = (*jni_env)->GetMethodID(jni_env, user_data->mainActivityClass, "callbackForBeginningTest", "(Ljava/lang/String;JJJ)V");
	user_data->methodIdCallbackForAllEndingTest = (*jni_env)->GetMethodID(jni_env, user_data->mainActivityClass, "callbackForEndingTest", "(Ljava/lang/String;JJJZLjava/lang/String;Ljava/lang/String;J)V");
	user_data->methodIdCallbackForAllTestsFinished = (*jni_env)->GetMethodID(jni_env, user_data->mainActivityClass, "callbackForAllTestsFinished", "(JJJDJ)V");
	user_data->methodIdForGetShouldContinueRunning = (*jni_env)->GetMethodID(jni_env, user_data->mainActivityClass, "getJavaScriptThreadShouldContinueRunning", "()Z");

	return user_data;
}

void Test262JNI_FreeUserDataForRunTests(struct Test262AndroidUserDataForRunTests* user_data)
{
	JNIEnv* jni_env = user_data->jniEnv;
	(*jni_env)->DeleteGlobalRef(jni_env, user_data->mainActivityClass);
	(*jni_env)->DeleteGlobalRef(jni_env, user_data->mainActivityInstance);
	free(user_data);
}


int32_t Test262JNI_CallbackForAllTestsStarting(uint32_t total_number_of_tests, void* user_data)
{
	LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Test262JNI_CallbackForAllTestsStarting", "Test262JNI_CallbackForAllTestsStarting");
	
	struct Test262AndroidUserDataForRunTests* android_user_data = (struct Test262AndroidUserDataForRunTests*)user_data;

	(*android_user_data->jniEnv)->CallVoidMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdCallbackForAllTestsStarting, (jlong)total_number_of_tests, (jlong)user_data);

	jboolean should_continue = (*android_user_data->jniEnv)->CallBooleanMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdForGetShouldContinueRunning);
	android_user_data->javaScriptThreadShouldContinueRunning = (int32_t)should_continue;
	return android_user_data->javaScriptThreadShouldContinueRunning;
}

int32_t Test262JNI_CallbackForBeginningTest(const char* test_file, uint32_t total_number_of_tests, uint32_t current_test_number, void* user_data)
{
	struct Test262AndroidUserDataForRunTests* android_user_data = (struct Test262AndroidUserDataForRunTests*)user_data;

	jstring java_test_file_string = jstringFromC(android_user_data->jniEnv, test_file);	
	(*android_user_data->jniEnv)->CallVoidMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdCallbackForAllBeginningTest, java_test_file_string, (jlong)total_number_of_tests, (jlong)current_test_number, (jlong)user_data);
	(*android_user_data->jniEnv)->DeleteLocalRef(android_user_data->jniEnv, java_test_file_string);
	
	jboolean should_continue = (*android_user_data->jniEnv)->CallBooleanMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdForGetShouldContinueRunning);
	android_user_data->javaScriptThreadShouldContinueRunning = (int32_t)should_continue;
	return android_user_data->javaScriptThreadShouldContinueRunning;
}

int32_t Test262JNI_CallbackForEndingTest(const char* test_file, uint32_t total_number_of_tests, uint32_t current_test_number, uint32_t total_number_of_tests_failed, _Bool did_pass, const char* exception_string, const char* stack_string, void* user_data)
{
	struct Test262AndroidUserDataForRunTests* android_user_data = (struct Test262AndroidUserDataForRunTests*)user_data;

	jstring java_test_file_string = jstringFromC(android_user_data->jniEnv, test_file);		
	jstring java_exception_string = jstringFromC(android_user_data->jniEnv, exception_string);		
	jstring java_stack_string = jstringFromC(android_user_data->jniEnv, stack_string);		
	(*android_user_data->jniEnv)->CallVoidMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdCallbackForAllEndingTest,
		java_test_file_string, (jlong)total_number_of_tests, (jlong)current_test_number, (jlong)total_number_of_tests_failed, (jboolean)did_pass, java_exception_string, java_stack_string, (jlong)user_data
	);
	(*android_user_data->jniEnv)->DeleteLocalRef(android_user_data->jniEnv, java_stack_string);
	(*android_user_data->jniEnv)->DeleteLocalRef(android_user_data->jniEnv, java_exception_string);
	(*android_user_data->jniEnv)->DeleteLocalRef(android_user_data->jniEnv, java_test_file_string);
	
	jboolean should_continue = (*android_user_data->jniEnv)->CallBooleanMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdForGetShouldContinueRunning);
	android_user_data->javaScriptThreadShouldContinueRunning = (int32_t)should_continue;
	return android_user_data->javaScriptThreadShouldContinueRunning;
}


int32_t Test262JNI_CallbackForAllTestsFinished(uint32_t total_number_of_tests, uint32_t number_of_tests_run, uint32_t total_number_of_tests_failed, double diff_time_in_double_seconds, void* user_data)
{
	struct Test262AndroidUserDataForRunTests* android_user_data = (struct Test262AndroidUserDataForRunTests*)user_data;

	(*android_user_data->jniEnv)->CallVoidMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdCallbackForAllTestsFinished, (jlong)total_number_of_tests, (jlong)number_of_tests_run, (jdouble)diff_time_in_double_seconds, (jlong)user_data);
	
	jboolean should_continue = (*android_user_data->jniEnv)->CallBooleanMethod(android_user_data->jniEnv, android_user_data->mainActivityInstance, android_user_data->methodIdForGetShouldContinueRunning);
	android_user_data->javaScriptThreadShouldContinueRunning = (int32_t)should_continue;
	return android_user_data->javaScriptThreadShouldContinueRunning;
}



JNIEXPORT void JNICALL Java_org_webkit_javascriptcore_test262_Test262_runTests(JNIEnv* env, jobject thiz, jobject java_asset_manager)
{
	LOGD("Java_org_webkit_javascriptcore_test262_Test262_runTests");

	// I'm not sure if I need to
	AAssetManager* ndk_asset_manager = AAssetManager_fromJava(env, java_asset_manager);
	/* Saving the object instance so it can be used for calling back into Java. 
	 * I think I need to call NewGlobalRef on it because it must live past this function,
	 * otherwise I get the error: 
	 * JNI ERROR (app bug): accessed stale local reference
	 */
//	s_proxyObject = (*env)->NewGlobalRef(env, thiz);

	LogWrapper* log_wrapper = s_logWrapper;
	LogWrapperAndroid_OpenNewFile(env, log_wrapper);

	LogWrapper_LogEvent(log_wrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Created LogWrapper", "Created logwrapper instance");
	
	struct Test262AndroidUserDataForRunTests* user_data = Test262JNI_CreateUserDataForRunTests(env, thiz);


	Test262Helper_RunTests(log_wrapper, ndk_asset_manager,
		Test262JNI_CallbackForAllTestsStarting,
		Test262JNI_CallbackForBeginningTest,
		Test262JNI_CallbackForEndingTest,
		Test262JNI_CallbackForAllTestsFinished,
		user_data
	);

	Test262JNI_FreeUserDataForRunTests(user_data);

	LogWrapper_Flush(log_wrapper);
	LogWrapper_CloseFile(log_wrapper);

//	LogWrapper_Free(log_wrapper);
}

jlong JNICALL Java_org_webkit_javascriptcore_test262_Test262_getNativeLoggerFile(JNIEnv* jenv, jclass jcls)
{
	jlong jresult = 0 ;
	Logger* result = NULL;

	if((NULL != s_logWrapper) && (NULL != s_logWrapper->loggerFile))
	{
		result = s_logWrapper->loggerFile;
		*(Logger **)&jresult = result; 
	}
	return jresult;
}

jlong JNICALL Java_org_webkit_javascriptcore_test262_Test262_getNativeLoggerNative(JNIEnv* jenv, jclass jcls)
{
	jlong jresult = 0 ;
	Logger* result = NULL;

	if((NULL != s_logWrapper) && (NULL != s_logWrapper->loggerNative))
	{
		result = s_logWrapper->loggerNative;
		*(Logger **)&jresult = result; 
	}
	return jresult;
}

jlong JNICALL Java_org_webkit_javascriptcore_test262_Test262_getNativeLoggerSocket(JNIEnv* jenv, jclass jcls)
{
	jlong jresult = 0 ;
	Logger* result = NULL;

	if((NULL != s_logWrapper) && (NULL != s_logWrapper->loggerSocket))
	{
		result = s_logWrapper->loggerFile;
		*(Logger **)&jresult = result; 
	}
	return jresult;
}



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
void NetworkHelperForAndroid_UploadFile(int accepted_socket, uint16_t http_server_port, void* user_data)
{
//	struct sockaddr_in addr;
	struct sockaddr_storage addr_storage; // connector's address information
	
	socklen_t addr_len = sizeof(addr_storage);
	const char* ip_address_string = NULL;
	char ipv6_address_string_storage_with_brackets[INET6_ADDRSTRLEN+2];
	
	int err = getpeername(accepted_socket, (struct sockaddr*)&addr_storage, &addr_len);
	if(err != 0)
	{
		fprintf(stderr, "getpeername failed for NetworkHelperForAndroid_UploadFile\n");
		LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "NetworkHelperForAndroid_UploadFile", "got error with getpeername %d ", err);
		return;
	}

	if(AF_INET == addr_storage.ss_family)
	{
		struct sockaddr_in* sin = (struct sockaddr_in*)&addr_storage;
		// This points to a statically stored char array within inet_ntoa() so that each time you call inet_ntoa() it will overwrite the last IP address you asked for.
//		ip_address_string = inet_ntoa(sin->sin_addr);
		ip_address_string = inet_ntop(AF_INET, &sin->sin_addr, ipv6_address_string_storage_with_brackets, sizeof(ipv6_address_string_storage_with_brackets));
//		ip_address_string = &ipv6_address_string_storage[0];
		
		LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "NetworkHelperForAndroid_UploadFile", "ip_address_string: %s, port:%d", ip_address_string, http_server_port);
	}
	else if(AF_INET6 == addr_storage.ss_family)
	{
		char ipv6_address_string_storage[INET6_ADDRSTRLEN];
		struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&addr_storage;
		const char* ipv6_address_string = inet_ntop(AF_INET6, &sin6->sin6_addr, ipv6_address_string_storage, sizeof(ipv6_address_string_storage));
		// We need to add brackets around the address "literal" for IPv6. I think this is part of the spec.
		// I'm assuming inet_ntop will always give a literal address.
		ipv6_address_string_storage_with_brackets[0] = '[';
		ipv6_address_string_storage_with_brackets[1] = '\0';
		strcat(ipv6_address_string_storage_with_brackets, ipv6_address_string);
		strcat(ipv6_address_string_storage_with_brackets, "]");
		LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "NetworkHelperForAndroid_UploadFile", "ipv6_address_string_storage_with_brackets: %s, port:%d", ipv6_address_string_storage_with_brackets, http_server_port);
		// for convenience, set ip_address_string so the following code can blindly use the same string pointer.
		ip_address_string = &ipv6_address_string_storage_with_brackets[0];
	}
	else
	{
		fprintf(stderr, "unexpected ss_family: %d\n", addr_storage.ss_family);
		LogWrapper_LogEvent(s_logWrapper, LOGWRAPPER_PRIORITY_CRITICAL, LOGWRAPPER_PRIMARY_KEYWORD, "NetworkHelperForAndroid_UploadFile", "unexpected sin_family: %d", addr_storage.ss_family);
		return;
	}


	// I don't know what thread things are happening on (I probably do, but I don't want to be locked in).
	// So we need to use Attach/DetachThread
	
	 /* There is a little JNI dance you can do to deal with this situation which is shown here.
	 */
	jobject network_helper = (jobject)user_data;	
    JNIEnv* jni_env;
    int get_env_stat = (*s_javaVm)->GetEnv(s_javaVm, (void**)&jni_env, JNI_VERSION_1_6);
    if(get_env_stat == JNI_EDETACHED)
	{
		jint attach_status = (*s_javaVm)->AttachCurrentThread(s_javaVm, &jni_env, NULL);
		if(0 != attach_status)
		{
			LOGE("AttachCurrentThread failed"); 
		}
    }
	else if(JNI_OK == get_env_stat)
	{
        // don't need to do anything
    }
	else if (get_env_stat == JNI_EVERSION)
	{
		LOGE("GetEnv: version not supported"); 
    }

	jclass network_helper_class = (*jni_env)->GetObjectClass(jni_env, network_helper);

	jmethodID method_id = (*jni_env)->GetMethodID(jni_env, network_helper_class, "uploadFileCallbackFromNative", "(Ljava/lang/String;S)V");
	jstring java_ipaddress_string = jstringFromC(jni_env, ip_address_string);
	(*jni_env)->CallVoidMethod(jni_env, network_helper, method_id, java_ipaddress_string, http_server_port);

	/* Clean up: If we Attached the thread, we need to Detach it */
    if(get_env_stat == JNI_EDETACHED)
	{
		(*s_javaVm)->DetachCurrentThread(s_javaVm);
	}
}

// Watch out: Signal handlers are global
// If you are automatically breaking on SIGPIPE due to writing on broken sockets, see this to make Xcode not do that:
// http://stackoverflow.com/questions/10431579/permanently-configuring-lldb-in-xcode-4-3-2-not-to-stop-on-signals
void NetworkHelperForAndroid_GlobalSignalHandlerForSIGPIPE(int sig)
{
	fprintf(stderr, "NetworkHelperForAndroid_GlobalSignalHandlerForSIGPIPE");
	LOGD("NetworkHelperForAndroid_GlobalSignalHandlerForSIGPIPE");
	
	LogWrapper* log_wrapper = s_logWrapper;
	Logger_Disable(log_wrapper->loggerSocket);
	// I think I need an atomic set on LoggerWrapper because there might be a theoretical race condition between freeing the Logger and setting the pointer to NULL in the wrapper.
	Logger* temp_logger = log_wrapper->loggerSocket;
	log_wrapper->loggerSocket = NULL;
	Logger_Free(temp_logger);
}

static void NetworkHelperForAndroid_OpenLogStream(int accepted_socket, void* opaque_log_wrapper, void* user_data)
{
	LogWrapper* log_wrapper = (LogWrapper*)opaque_log_wrapper;

	// disable SIGPIPE errors (crashes) on writing to a dead socket. I need this since I'm blindly throwing stuff at Logger.
	// decided to move to platform specific code:
	// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
	// If you are automatically breaking on SIGPIPE due to writing on broken sockets, see this to make Xcode not do that:
	// http://stackoverflow.com/questions/10431579/permanently-configuring-lldb-in-xcode-4-3-2-not-to-stop-on-signals
#if 0
	int set_value = 1;
	setsockopt(accepted_socket, SOL_SOCKET, SO_NOSIGPIPE, &set_value, sizeof(set_value));
#else
    struct sigaction sa;
    sa.sa_handler = NetworkHelperForAndroid_GlobalSignalHandlerForSIGPIPE;
    sa.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
	{
		perror("sigaction");
		fprintf(stderr, "can't setup signal handler");

		// Linux doesn't have SO_NOSIGPIPE
		// int set_value = 1;
		// setsockopt(accepted_socket, SOL_SOCKET, SO_NOSIGPIPE, &set_value, sizeof(set_value));
    }
#endif

	// Thanks to Unix, where everything is a file, I'll just convert a socket into a FILE* with fdopen.
	// int fd = socket(AF_INET, SOCK_STREAM, 0);
	// fdopen or fdreopen?
	FILE* file_pointer_from_socket = fdopen(accepted_socket, "a");
	// Only allow one right now. Need to figure out if it is worth supporting more.
	if(NULL != log_wrapper->loggerSocket)
	{
		Logger_Free(log_wrapper->loggerSocket);
	}
	// the "" is a hack to deal with that there are no file names in sockets, but Logger currently expects files except for stdout/stderr. Logger should be extended to handle this case.
	log_wrapper->loggerSocket = Logger_CreateWithHandle(file_pointer_from_socket, "");

	Logger_EnableAutoFlush(log_wrapper->loggerSocket);

	// Test/debug message. Sending it directly to the socketLogger instead of all 3 loggers.
	Logger_LogEventNoFormat(log_wrapper->loggerSocket, LOGWRAPPER_PRIORITY_DEBUG, LOGWRAPPER_PRIMARY_KEYWORD, "Debug", "Log Stream socket connected to Logger via socket->fdopen (this not echoed to other loggers)");

	
}

static jobject s_networkHelper = NULL;

JNIEXPORT jboolean JNICALL Java_org_webkit_javascriptcore_test262_NetworkHelper_startServerNative(JNIEnv* jni_env, jobject thiz, jobject java_return_data_object)
{
	// javap -classpath bin/classes -s -p org.webkit.javascriptcore.test262.NetworkHelperReturnDataObject
	jclass return_data_object_class = (*jni_env)->GetObjectClass(jni_env, java_return_data_object);
	
	int server_socket = 0;
	uint16_t server_port = 0;

	// need to retain the network helper instance for the upload file callback or it will get collected.
	s_networkHelper = (*jni_env)->NewGlobalRef(jni_env, thiz);
	LOGD("got s_networkHelper NewGlobalRef");
	SocketServer_SetUploadFileCallback(NetworkHelperForAndroid_UploadFile, s_networkHelper);
	SocketServer_SetOpenLogStreamCallback(NetworkHelperForAndroid_OpenLogStream, s_logWrapper, NULL);

	SocketServer_CreateSocketAndListen(&server_socket, &server_port);


	

	struct SocketServer_UserData* server_user_data = (struct SocketServer_UserData*)calloc(1, sizeof(struct SocketServer_UserData));
	server_user_data->serverSocket = server_socket;
	// This pointer is how we will signal the server thread to stop
	server_user_data->shouldKeepRunning = 1;
	SimpleThread* server_accept_thread = SocketServer_RunAcceptLoopInThread(server_user_data);


	jlong j_server_accept_thread = (jlong)server_accept_thread;
	LOGD("got server_accept_thread %llu", server_accept_thread);
	LOGD("got j_server_accept_thread %llu", j_server_accept_thread);


	// Because we don't have multiple return values for JNI,
	// we will save any additional information in the ReturnDataObject instance 
	// by calling back into Java to set values which can be read after this function returns.
	jmethodID method_id = (*jni_env)->GetMethodID(jni_env, return_data_object_class, "setFields", "(ISJJ)V");
//	(*jni_env)->CallVoidMethod(jni_env, java_return_data_object, method_id, server_socket, server_port, server_user_data, server_accept_thread);
	(*jni_env)->CallVoidMethod(jni_env, java_return_data_object, method_id, server_socket, server_port, (jlong)server_user_data, (jlong)server_accept_thread);

	return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_org_webkit_javascriptcore_test262_NetworkHelper_stopServerNative(JNIEnv* jni_env, jobject thiz, jint server_socket, jshort server_port, jlong j_server_user_data, jlong j_server_accept_thread)
{
	int thread_status = 0;
	LOGD("Java_org_webkit_javascriptcore_test262_NetworkHelper_stopServerNative");
	// a simple cast triggers the following warning:  cast to pointer from integer of different size [-Wint-to-pointer-cast]
	// I think it is bogus. SWIG does the following which seems to avoid the problem.
//	struct SocketServer_UserData* server_user_data = *(struct SocketServer_UserData**)&j_server_user_data;
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	struct SocketServer_UserData* server_user_data = (struct SocketServer_UserData*)j_server_user_data;
	LOGD("got server_user_data");

	LOGD("got j_server_accept_thread %llu", j_server_accept_thread);
	
//	SimpleThread* server_accept_thread = *(SimpleThread**)&j_server_accept_thread;
	SimpleThread* server_accept_thread = (SimpleThread*)j_server_accept_thread;
	LOGD("got server_accept_thread %llu", server_accept_thread);
	#pragma GCC diagnostic pop
	// This pointer is how we will signal the server thread to stop
	server_user_data->shouldKeepRunning = 0;
	LOGD("got shouldKeepRunning");
	// I think this will disrupt the accept() blocking forcing the loop to continue
	// Linux requires shutdown. Directly calling close results in undefined/broken behavior.
	shutdown(server_socket, SHUT_RDWR);
	close(server_socket);
	LOGD("got close");

	// wait for the thread to end
	SimpleThread_WaitThread(server_accept_thread, &thread_status);
	LOGD("got SimpleThread_WaitThread");

	free(server_user_data);
	LOGD("got free");

	/* Release the proxy object. */
	(*jni_env)->DeleteGlobalRef(jni_env, s_networkHelper);
	s_networkHelper = NULL;

	LOGD("got DeleteGlobalRef");
	
}

