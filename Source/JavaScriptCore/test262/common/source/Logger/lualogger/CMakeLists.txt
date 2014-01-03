

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/clogger ${PROJECT_SOURCE_DIR}/timestamp ${LUA_INCLUDE_DIR})


# Add the Logger (binary) directory as part of the link search path so
# the test program can find the Logger library to link against.
LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/clogger)


# Create a module called "logger" which includes all the source files
# for the library.
# The name "logger" will automatically be named "liblogger.*" for Unix,
# so don't explicitly put the prefix "lib" in the name.
# For the source files, you may omit the extension and all 
# implementation files will be added, but header files will not.
# Instead I explicitly list all files. This is not important for Makefiles,
# but is nicer for IDEs like Visual Studio, Xcode, etc, because the header 
# files will be included in the IDE file browsers.
ADD_LIBRARY( loggerlua MODULE
	#	LoggerLua.h
	LoggerLua.c
)

# Lua modules don't usually have the "lib" prefix in the file name, so get rid of it.
SET_TARGET_PROPERTIES(loggerlua PROPERTIES PREFIX "")

# This tells the build system that the library "objclogger" must
# be linked against the "logger" library.
TARGET_LINK_LIBRARIES(loggerlua
	loggerc
	${LUA_LIBRARIES}
)
