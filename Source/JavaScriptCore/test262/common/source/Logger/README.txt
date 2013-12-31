This is a little sample program that demonstrates several key things:

1) Documentation style/guidelines using Doxygen

2) Coding style/guidelines via the Logger library.

3) How to use Logger (testprogram in test).

4) How to use CMake.


To build this project:

1) Make sure CMake is installed. You should also have Doxygen and (Graphviz) Dot installed to generate all the pretty diagrams.

2) Create a new directory somewhere for all the build files. (This is to keep build files separate from source files which makes source management easier to work with.) I recommend you make a directory called BUILD in ../
So if your Logger directory is in
/home/ewing/MyWork/Logger
Create a directory called
/home/ewing/MyWork/BUILD

3) Change directories to your build directory.

4) From your build directory, run:
ccmake <PATH_TO_SOURCE>
So in my example, the command would be
ccmake ../Logger
(Hint: You can also generate other projects besides Makefiles using 
cmake -G<type>, e.g. cmake -GXCode ../Logger)

5) This will bring you to a menu. Hit 'c' to go past the CMAKE_BACKWARDS_COMPATIBILITY option.

5b) You are now in the main menu. You can hit 't' to get more options. 

5c) If you hit 't' to see the advanced options, look for "BUILD_DOCUMENTATION". Select it and press ENTER. This will toggle documentation on. 

5d) You also can set the CMAKE_BUILD_TYPE to DEBUG, MINSIZEREL, RELEASE, RELWITHDEBINFO to pick the different compile options. There is also an option for verbose builds which shows you more detail during the build process.

6) After you finish selecting all your options, hit 'c' to configure. If there are no errors, the menu will update. Keep hitting 'c' until the 'g' option appears. Then hit 'g' to generate the Makefiles. (If there was an error, you may have to fill in the correct paths for the missing tools.)
Generally hitting 'c' again, and then hitting 'g' will configure everything and generate the Makefiles and you will be returned to the Unix prompt.

7) Type 'make' to build the code.
This will build the stuff in the logger directory as a library, and build the testprogram which links to the logger library.

8) Type 'make DoxygenDoc' to build the documentation.


Note: If you ever add new files or directories, you should rerun ccmake as you did before so it can update the Makefiles. Just modifying files will not require you to rerun CMake.




