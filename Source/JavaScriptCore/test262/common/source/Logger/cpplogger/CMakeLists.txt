
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/timestamp)

# Create a library called "logger" which includes all the source files
# for the library.
# The name "logger" will automatically be named "liblogger.*" for Unix,
# so don't explicitly put the prefix "lib" in the name.
# For the source files, you may omit the extension and all 
# implementation files will be added, but header files will not.
# Instead I explicitly list all files. This is not important for Makefiles,
# but is nicer for IDEs like Visual Studio, Xcode, etc, because the header 
# files will be included in the IDE file browsers.
ADD_LIBRARY( loggercpp SHARED
	${PROJECT_SOURCE_DIR}/timestamp/TimeStamp.h
	${PROJECT_SOURCE_DIR}/timestamp/TimeStamp.c
	Logger.hpp
	Logger.cpp
)

