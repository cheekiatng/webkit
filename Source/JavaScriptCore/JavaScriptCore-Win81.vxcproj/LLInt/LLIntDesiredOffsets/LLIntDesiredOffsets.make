all:
    touch "%ConfigurationBuildDir%\buildfailed"
    bash build-LLIntDesiredOffsets.sh "%ConfigurationBuildDir%"

    -del "%ConfigurationBuildDir%\buildfailed"

clean:
    -del "%ConfigurationBuildDir%\buildfailed"
    -del /s /q "%ConfigurationBuildDir%\DerivedSources\LLIntDesiredOffsets.h"
