CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
# Simple CMake example to build the Logger library and a test program.
# This will build logger into a library and build a program 
# that links against it.

# First you must define a project name. (This probably makes more sense to 
# you if you have Visual Studio experience.)
PROJECT(Logger)

# This will find the Doxygen stuff on your system if you want it.
#INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
# I need to use my version because the official version doesn't 
# understand the OS X version of Doxygen. It also doesn't define
# DOT_PATH which is a problem if dot is not in the path.
#INCLUDE(${PROJECT_SOURCE_DIR}/CMakeModules/Documentation.cmake OPTIONAL)
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

# Allow the user to specify if Logger should be compiled with or without
# thread/file locking.
OPTION(LOGGER_ENABLE_LOCKING "Compiles Logger with mutexes and file locking" ON)

# By default, BUILD_DOCUMENTATION and CMAKE_VERBOSE_MAKEFILE
# are marked as advanced (hidden in the advanced menu).
# I want these options to be seen in the basic menu, so I will
# call the function to toggle them back.
MARK_AS_ADVANCED(CLEAR
	BUILD_DOCUMENTATION
	CMAKE_VERBOSE_MAKEFILE
)

# If the user wants to compile the library with locking, add the 
# appropriate -D flags.
IF(LOGGER_ENABLE_LOCKING)
	ADD_DEFINITIONS(-DLOGGER_ENABLE_LOCKING)
ENDIF(LOGGER_ENABLE_LOCKING)

# Go through the subdirectories that contain source.
SUBDIRS(clogger cpplogger socket test)

IF(APPLE)
	ADD_SUBDIRECTORY(objclogger)
ENDIF(APPLE)

FIND_PACKAGE(Lua51)
IF(LUA51_FOUND)
	ADD_SUBDIRECTORY(lualogger)
ENDIF()

# To build the documention, you will have to enable it
# and then do the equivalent of "make doc".
IF (BUILD_DOCUMENTATION)
	IF (DOT)
		SET(HAVE_DOT YES)
	ELSE (DOT)
		SET(HAVE_DOT NO)
	ENDIF (DOT)
	# This processes our Doxyfile.in and substitutes paths to generate
	# a final Doxyfile
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in 
		${PROJECT_BINARY_DIR}/doc/Doxyfile)
	# This creates a new target to build documentation.
	# It runs ${DOXYGEN} which is the full path and executable to
	# Doxygen on your system, set by the FindDoxygen.cmake module
	# (called by FindDocumentation.cmake).
	# It runs the final generated Doxyfile against it.
	# The DOT_PATH is substituted into the Doxyfile.
	ADD_CUSTOM_TARGET(DoxygenDoc ${DOXYGEN}
		${PROJECT_BINARY_DIR}/doc/Doxyfile)
ENDIF (BUILD_DOCUMENTATION)


