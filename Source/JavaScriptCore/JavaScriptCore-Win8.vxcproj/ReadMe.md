TestRunner Application
----------------------

 This sample contains a Windows Store application that will run the ECMAScript test262 tests. The tests are designed to ensure compliance with the current ECMAScript (ECMA-262) specifications. This application is a port of the Windows Phone "TestRunner" application with the addition of using WinUtils library to check System Process Memory while running the tests.

Building and Running Sample
---------------------------

Included in the sample is JavaScriptCore.sln Visual Studio Solution file. The solution builds the TestRunner Applcation and the TestRunnerComponenet Windows Runtime Component. TestRunnerComponent is a C++/CX WinRT object that hooks up TestRunner app with JavaScriptCore static library. Do the following steps before running TestRunner.

- Make sure WinUtils.lib and JavaScript.lib are in c:/<CONFIGURATION>/bin32. Where <CONFIGURATION> is ether Debug or Release. You can put the libraries elsewhere but you will need to edit TestRunnerComponent project settings. See the references below to accesses the WinUtil and JavaScriptCore libraries.

- Make sure the path to Counter.h for WinUtils.lib and the headers files for JavaSriptCore.lib are listed in TestRunnerComponent project include path. The default is c:/<CONFIGURATION>/include. 

 References
 ----------
 
 WinUtil Library - https://github.com/appcelerator/hyperloop/tree/master/examples/native/windows/WinUtils

 JavaScriptCore Library - https://github.com/appcelerator/webkit/tree/javascriptcore-wp8-test262/Source/JavaScriptCore/JavaScriptCore-WP8.vxcproj

 How To Build JavaScriptCore - https://github.com/appcelerator/webkit/blob/javascriptcore-wp8-test262/BUILDING.txt

 ECMAScript Language Tests - http://test262.ecmascript.org/
