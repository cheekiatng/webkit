JavaScriptCore-Win81.vxproj Folder
----------------------------------

This folder contains the Visual Studio 2013 solution and projects to build JavaScriptCore.lib for Windows 8.1 OS. It includes a Windows Store Application named TestRunner to test JavaScriptCore.lib for compliance with the current ECMAScript (ECMA-262) specifications.

Building and Running Tests
--------------------------

Included in the sample is JavaScriptCore.sln Visual Studio Solution file. The solution builds WTF, JavaScriptCore, TestRunner Windows Store App and the TestRunnerComponenet Windows Runtime Component. TestRunnerComponent is a C++/CX WinRT object that hooks up TestRunner C# app with JavaScriptCore static library. Do the following steps before running TestRunner.

- Cygwin: Building JavaScriptCore on Windows requires Cygwin as some of the projects are of type Makefile and they do make use of perl, ruby and python. The easiest way to get Cygwin setup is to follow the WebKit build instructions located here - http://www.webkit.org/building/tools.html.

- Output Directory: Make sure to set the WEBKIT_OUTPUTDIR environment variable. The final path to JavaScriptCore.lib is WEBKIT_OUTPUTDIR/<CONFIGURATION>/bin32. Where <CONFIGURATION> is ether Debug or Release. 

Windows Phone
-------------

When the Windows Phone 8.1 is released there will be major convergance with Windows Store 8.1 applications. It should be possible to use the projects in this folder and rather then in the C++ preprocessor settings using WINDOWS change to WINDOWS_PHONE. The target platform will also need to point to the Windows Phone 8.1 tool-set. Building for WINDOWS_PHONE is turns of the "Just In Time Compiler" and stops the uses of executable memory a requirement when running on Windows Phones.


 References
 ----------

 ECMAScript Language Tests - http://test262.ecmascript.org/
