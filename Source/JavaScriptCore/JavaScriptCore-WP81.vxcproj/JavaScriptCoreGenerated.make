all:
    bash build-generated-files.sh "%ConfigurationBuildDir%"
    copy-files.cmd

clean:
    -del "%ConfigurationBuildDir%\buildfailed"
    copy-files.cmd clean
    -del /s /q "%ConfigurationBuildDir%\obj32\JavaScriptCore\DerivedSources"
