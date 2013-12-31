
# Add the logger (source) directory as part of the header search path so 
# the test program can find the Logger headers.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/clogger ${PROJECT_SOURCE_DIR}/cpplogger)


# Add the Logger (binary) directory as part of the link search path so
# the test program can find the Logger library to link against.
LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/clogger ${PROJECT_BINARY_DIR}/cpplogger)

# Create an executable program called "testlogger".
# As with the ADD_LIBRARY command, you should list all the files
# that are needed to build the executable.
ADD_EXECUTABLE(testloggerc
	testloggerc.c
)
ADD_EXECUTABLE(testloggercpp
	testloggercpp.cpp
)

# This tells the build system that the executable "testlogger" must
# be linked against the "logger" library.
TARGET_LINK_LIBRARIES(testloggerc
	clogger
)
TARGET_LINK_LIBRARIES(testloggercpp
	cpplogger
)

# The thread test only works with posix (pthreads) right now because my
# test directly uses pthreads to setup the tests.
IF(UNIX)

# Use the standard supplied CMake module to figure out what dependencies
# we need for threads on this system.
# It will define the variable ${CMAKE_THREAD_LIBS_INIT} for the 
# libraries you need to link against (e.g. -pthread).
FIND_PACKAGE(Threads)
	
ADD_EXECUTABLE(testthreadscpp
	testthreadscpp.cpp
)
TARGET_LINK_LIBRARIES(testthreadscpp
	cpplogger
	${CMAKE_THREAD_LIBS_INIT}
)
ENDIF(UNIX)
