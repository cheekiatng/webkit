#include <stdio.h>

#ifdef __ANDROID__
	#include <sys/types.h> // need for off_t for asset_manager because it looks like r9b broke something.
	#include <android/asset_manager.h>
	#include <android/asset_manager_jni.h>

	#undef fprintf
	#undef printf
	#include <android/log.h>
	#define printf(...) ((void)__android_log_print(ANDROID_LOG_INFO,  "Test262", __VA_ARGS__))
	#define fprintf(stderr, ...) ((void)__android_log_print(ANDROID_LOG_INFO,  "Test262", __VA_ARGS__))
#endif

#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>

/* Returns a JSStringRef which must be released by you when done */
_Bool Test262_EvaluateStringScript(JSStringRef js_script_string, JSStringRef js_file_name, JSStringRef* out_error_string, JSStringRef* out_stack_string)
{
//__android_log_print(ANDROID_LOG_INFO,  "Test262", "In Test262_EvaluateStringScript\n");
//fprintf(stderr, "fprintf in Test262\n");
	JSGlobalContextRef js_context = JSGlobalContextCreate(NULL);
    JSValueRef js_exception = NULL;
			
    JSValueRef js_result = JSEvaluateScript(js_context, js_script_string, NULL, js_file_name, 0, &js_exception);

	_Bool is_success = js_result && !js_exception;
//__android_log_print(ANDROID_LOG_INFO,  "Test262", "is_success: %d", is_success);

	if(!is_success)
	{
//			fprintf(stderr, "in not success\n");
		
		if(js_exception)
		{

//			fprintf(stderr, "in exception\n");
			if(NULL != out_stack_string)
			{
//			fprintf(stderr, "in out_stack_string\n");

				/* Create a string with the contents "stack" which will be used as a property name to retrieve the value from an object. */
				JSStringRef js_literal_stack_string = JSStringCreateWithUTF8CString("stack");
				/* Convert the JSValueRef to JSObjectRef. I don't think any memory gets retained or copied so we don't need to release. */
				JSObjectRef js_exception_object = JSValueToObject(js_context, js_exception, NULL);
				/* Get exception_object.stack */
				JSValueRef stack_object = JSObjectGetProperty(js_context, js_exception_object, js_literal_stack_string, NULL);
				/* Don't need the "stack" string any more */
				JSStringRelease(js_literal_stack_string);

				*out_stack_string = JSValueToStringCopy(js_context, stack_object, NULL);

				/* Release the stack_object */
				JSValueUnprotect(js_context, stack_object);
			}


			if(NULL != out_error_string)
			{
//			fprintf(stderr, "in out_error_string\n");
				
				*out_error_string = JSValueToStringCopy(js_context, js_exception, NULL);
			}

			JSValueUnprotect(js_context, js_exception);
		}
		else
		{
			*out_error_string = JSStringCreateWithUTF8CString("JSEvaluateScript() failed, but didn't get an exception???");
			fprintf(stderr, "JSEvaluateScript() failed, but didn't get an exception???\n");
		}
	}
	else
	{
//__android_log_print(ANDROID_LOG_INFO,  "Test262", "was success");

//			fprintf(stderr, "in  success\n");
		
	}

	/* clean up result because we don't actually need it */
	if(js_result)
	{
		JSValueUnprotect(js_context, js_result);
	}

	JSGarbageCollect(js_context);
	JSGlobalContextRelease(js_context);

	return is_success;
}

