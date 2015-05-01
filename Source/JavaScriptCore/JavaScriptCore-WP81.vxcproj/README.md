# JavaScriptCore for Windows Phone 8.1 ARM

This folder contains the Visual Studio 2013 solution and projects to build JavaScriptCore.lib for Windows Phone 8.1.
:warning: If you want to build JavaScriptCore for Windows 8.1 or Windows Phone 8.1(x86 or emulator), use the *JavaScriptCore-Win81.vxproj* project instead.

## Building JavaScriptCore

Included at the same level as this ReadMe is a JavaScriptCore.sln file. It is Visual Studio 2013 solution file. The extension maybe hidden. To see hidden files use Windows File Explorer and on the View menu select to show File name extensions. The solution builds WTF and JavaScriptCore. The place to start is to open JavaScriptCore.sln from within Visual Studio 2013 on a Windows 8.1 machine. You may need Visual Studio Pro or community edition to build it. It is important to do the following steps before building JavaScriptCore.

- Cygwin: Building JavaScriptCore on Windows requires Cygwin as some of the projects are of type Makefile and make use of perl, ruby and python. The easiest way to get Cygwin setup is to follow the WebKit build instructions located here - [http://www.webkit.org/building/tools.html](http://www.webkit.org/building/tools.html) Important: There are some hard coded things still in the build process that look for Cygwin in %SYSTEM_DRIVE%\cygwin so you must install to this directory. Also note that the Cygwin 64-bit likes to install to C:\cygwin64 instead of C:\cygwin (where C: represents the system drive). This will break the process which means you must change the installation path or you should stick with the 32-bit version of Cygwin.
  :warning: The cygwin-downloader utility provided by webkit has a bug. You'll need to place the setup.ini file it creates underneath the x86 directory it creates before proceeding through the setup installer. See http://stackoverflow.com/questions/27080690/build-or-compile-webkit-on-windows7-64bit

- System Path: Update the PATH environment variable to include where to find cygwin/bin. The WebKit projects use Unix commands to generate dependent files and to build the binaries. It is important that Visual Studio be able to find and run commands like bash, perl, ruby and make. First (install cygwin) as described above. Then make sure the Unix commands can run from a DOS command shell. To check open a Command Prompt window and type "ls" you should see a directory listing. If not something is wrong with the cygwin installation. With a default installation, this path will be: C:\cygwin\bin

- Output Directory: Set *WEBKIT_OUTPUTDIR* environment variable. This variable controls where the projects in the solution will put the intermediate files and the final binaries. To set environment variables in Windows 8.1 type "Control Panel" from the Windows Start Screen. The screen with the application tiles. Then launch the Control Panel application. On the left hand side click on the “System and Security” link. Then in the page that opens select the “System” link. In the page that opens on the left hand side select the “Advanced system settings” link. On the dialog that appears is a button labeled "Environment Variables..." select the button, this is where you set Windows environment variables. Set the WEBKIT_OUTPUTDIR environment variable ether as user or system. The final path to JavaScriptCore.lib will be WEBKIT_OUTPUTDIR/<CONFIGURATION>/bin32. Where <CONFIGURATION> is ether Debug or Release.  For example, I set my WEBKIT_OUTPUTDIR to c:\webkit_build. After building the solution using the “Debug” configuration JavaScriptCore.lib is located in c:\webkit_build\Debug\bin32. You only need to change your User environmental variable, not the system-wide one. In Win 8.1, you may hit WinKey-s and search for Environment Variable and select Edit Environment Variables for your account. You do not need to reboot after setting this.

- WebKitLibraries Directory: Set *WEBKIT_LIBRARIES* environment variable. This variable is an absolute Windows-style path pointing to the WebKitLibraries/win folder in your WebKit checkout. For example, if I checkout WebKit at c:\webkit, WEBKIT_LIBRARIES will be c:\webkit\WebKitLibraries\win.

- No DOS Line Endings: JavaScriptCoreGenerated, LLIntAssembly, and LLIntDesiredOffsets projects have bash shell scripts that are executed during project builds. The scripts end with ".sh" file extension. The scripts must not use DOS style line endings. These files are explicitly flagged in the .gitattributes so correct handling should be automatic. If this fails, you may also try when checking the files out on Windows, first execute "git config --system core.autocrlf false" so that DOS line endings are not automatically injected. If during building of the JavaScriptCore solution you see errors about "\r" then most likely the line ending are incorrect. Use the Unix command (available in cygwin) dos2unix to convert the line endings for the scripts. The Unix "file" command will tell you if the text file is using CRLF (DOS) or not.

If everything is configured correctly, you should be able to:

1. Switch to Release/Debug configuration
2. Make sure your "Active soluation platform" is ARM
3. Build "JavaScriptCore" project
4. Check if you have *WEBKIT_OUTPUTDIR*\<CONFIGURATION>\bin32\JavaScriptCore.lib

## References

- [http://www.webkit.org/building/tools.html](http://www.webkit.org/building/tools.html)
- [http://www.webkit.org/building/build.html](http://www.webkit.org/building/build.html)
 
