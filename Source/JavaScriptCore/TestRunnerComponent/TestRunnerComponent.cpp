// TestRunnerComponent.cpp

#include "TestRunnerComponent.h"
#include <string>
#include <windows.h>

using namespace TestRunnerComponent;
using namespace Platform;

extern int jscmainRepeatable(const std::string& script, const std::string& fileName, std::string& exceptionString);

ScriptRunner::ScriptRunner()
{
}

std::string convertPlatformStringtoUTF8(Platform::String^ in)
{
    size_t sizeRequired = WideCharToMultiByte(CP_UTF8, 0, in->Data(), in->Length(), NULL, 0, NULL, NULL);
    std::string result(sizeRequired, 0);
    WideCharToMultiByte(CP_UTF8, 0, in->Data(), in->Length(), &result[0], sizeRequired, NULL, NULL);
    return result;
}

Platform::String^ convertUTF8ToPlatformString(std::string utf8)
{
    size_t sizeRequired = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    std::wstring result(sizeRequired, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], sizeRequired);
    return ref new Platform::String(result.c_str());
}

ScriptResult^ ScriptRunner::RunScript(Platform::String^ script, Platform::String^ fileName)
{
    std::string scriptString = convertPlatformStringtoUTF8(script);
    std::string fileNameString = convertPlatformStringtoUTF8(fileName);
    std::string exceptionString;
    bool success = !jscmainRepeatable(scriptString.c_str(), fileNameString.c_str(), exceptionString);
    return ref new ScriptResult(success, fileName, convertUTF8ToPlatformString(exceptionString));
}

ScriptResult::ScriptResult(bool success, Platform::String^ testPath, Platform::String^ exceptionString)
    : m_success(success)
    , m_testPath(testPath)
    , m_exceptionString(exceptionString)
{
}