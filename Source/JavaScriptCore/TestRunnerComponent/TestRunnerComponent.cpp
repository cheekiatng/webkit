// TestRunnerComponent.cpp

#include "TestRunnerComponent.h"
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <string>
#include <windows.h>

using namespace TestRunnerComponent;
using namespace Platform;

extern int jscmainRepeatable(const std::string& script, const std::string& fileName, std::string& exceptionString);

ScriptRunner::ScriptRunner()
{
}

static std::string convertPlatformStringtoUTF8(Platform::String^ in)
{
    size_t sizeRequired = WideCharToMultiByte(CP_UTF8, 0, in->Data(), in->Length(), NULL, 0, NULL, NULL);
    std::string result(sizeRequired, 0);
    WideCharToMultiByte(CP_UTF8, 0, in->Data(), in->Length(), &result[0], sizeRequired, NULL, NULL);
    return result;
}

static Platform::String^ convertUTF8ToPlatformString(std::string utf8)
{
    size_t sizeRequired = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    std::wstring result(sizeRequired, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], sizeRequired);
    return ref new Platform::String(result.c_str());
}

static std::string convertJSStringToUTF8(JSStringRef jsString)
{
    size_t bytesNeeded = JSStringGetMaximumUTF8CStringSize(jsString);
 
    std::string result;
    result.resize(bytesNeeded, '\0');
    JSStringGetUTF8CString(jsString, &result[0], bytesNeeded);

    return result;
}

static std::string jsValueGetString(JSContextRef context, JSValueRef value)
{
    JSStringRef jsString = JSValueToStringCopy(context, value, NULL);
    std::string result = convertJSStringToUTF8(jsString);
    JSStringRelease(jsString);
    return result;
}

ScriptResult^ ScriptRunner::RunScript(Platform::String^ script, Platform::String^ fileName)
{
    std::string scriptString = convertPlatformStringtoUTF8(script);
    std::string fileNameString = convertPlatformStringtoUTF8(fileName);
    std::string exceptionString;

    JSGlobalContextRef context = JSGlobalContextCreate(NULL);
    JSStringRef jsScriptString = JSStringCreateWithUTF8CString(scriptString.c_str());
    JSStringRef jsFilenameString = JSStringCreateWithUTF8CString(fileNameString.c_str());
    JSValueRef exception = NULL;

    JSValueRef result = JSEvaluateScript(context, jsScriptString, NULL, jsFilenameString, 0, &exception);
    bool success = result && !exception;
    JSStringRelease(jsScriptString);
    JSStringRelease(jsFilenameString);

    if (result)
        JSValueUnprotect(context, result);

    if (exception) {
        JSStringRef jsStackString = JSStringCreateWithUTF8CString("stack");
        JSValueRef stack = JSObjectGetProperty(context, JSValueToObject(context, exception, NULL), jsStackString, NULL);
        JSStringRelease(jsStackString);

        exceptionString = jsValueGetString(context, exception) + "\n" + jsValueGetString(context, stack);

        JSValueUnprotect(context, stack);
        JSValueUnprotect(context, exception);
    }

    JSGarbageCollect(context);
    JSGlobalContextRelease(context);
    return ref new ScriptResult(success, fileName, convertUTF8ToPlatformString(exceptionString));
}

ScriptResult::ScriptResult(bool success, Platform::String^ testPath, Platform::String^ exceptionString)
    : m_success(success)
    , m_testPath(testPath)
    , m_exceptionString(exceptionString)
{
}