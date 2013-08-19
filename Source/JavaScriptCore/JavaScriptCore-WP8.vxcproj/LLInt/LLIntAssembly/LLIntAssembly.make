all:
    touch "%ConfigurationBuildDir%\buildfailed"
    bash build-LLIntAssembly.sh "%ConfigurationBuildDir%"
    -del "%ConfigurationBuildDir%\buildfailed"

clean:
    -del "%ConfigurationBuildDir%\buildfailed"
    -del /s /q "%ConfigurationBuildDir%\obj32\JavaScriptCore\DerivedSources\LLIntAssembly.h"
