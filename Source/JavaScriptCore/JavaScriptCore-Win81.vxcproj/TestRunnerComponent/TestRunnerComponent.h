#pragma once

namespace TestRunnerComponent
{
    public ref class ScriptResult sealed
    {
    public:
        ScriptResult(bool success, Platform::String^ testPath, Platform::String^ exceptionString);
        bool Success() { return m_success; }
        Platform::String^ ExceptionString() { return m_exceptionString; }
        Platform::String^ TestPath() { return m_testPath; }
        Platform::String^ ToString() { return TestPath(); }

    private:
        bool m_success;
        Platform::String^ m_testPath;
        Platform::String^ m_exceptionString;
    }; 

    public ref class ScriptRunner sealed
    {
    public:
        ScriptRunner();
        ScriptResult^ RunScript(Platform::String^ script, Platform::String^ fileName);
    };
}