# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/onter/software/clion-2017.3.4/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/onter/software/clion-2017.3.4/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/onter/project/hub/minloadblance

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/onter/project/hub/minloadblance/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/minloadblance.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/minloadblance.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/minloadblance.dir/flags.make

CMakeFiles/minloadblance.dir/main.cpp.o: CMakeFiles/minloadblance.dir/flags.make
CMakeFiles/minloadblance.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/onter/project/hub/minloadblance/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/minloadblance.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/minloadblance.dir/main.cpp.o -c /home/onter/project/hub/minloadblance/main.cpp

CMakeFiles/minloadblance.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/minloadblance.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/onter/project/hub/minloadblance/main.cpp > CMakeFiles/minloadblance.dir/main.cpp.i

CMakeFiles/minloadblance.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/minloadblance.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/onter/project/hub/minloadblance/main.cpp -o CMakeFiles/minloadblance.dir/main.cpp.s

CMakeFiles/minloadblance.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/minloadblance.dir/main.cpp.o.requires

CMakeFiles/minloadblance.dir/main.cpp.o.provides: CMakeFiles/minloadblance.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/minloadblance.dir/build.make CMakeFiles/minloadblance.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/minloadblance.dir/main.cpp.o.provides

CMakeFiles/minloadblance.dir/main.cpp.o.provides.build: CMakeFiles/minloadblance.dir/main.cpp.o


CMakeFiles/minloadblance.dir/log.cpp.o: CMakeFiles/minloadblance.dir/flags.make
CMakeFiles/minloadblance.dir/log.cpp.o: ../log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/onter/project/hub/minloadblance/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/minloadblance.dir/log.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/minloadblance.dir/log.cpp.o -c /home/onter/project/hub/minloadblance/log.cpp

CMakeFiles/minloadblance.dir/log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/minloadblance.dir/log.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/onter/project/hub/minloadblance/log.cpp > CMakeFiles/minloadblance.dir/log.cpp.i

CMakeFiles/minloadblance.dir/log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/minloadblance.dir/log.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/onter/project/hub/minloadblance/log.cpp -o CMakeFiles/minloadblance.dir/log.cpp.s

CMakeFiles/minloadblance.dir/log.cpp.o.requires:

.PHONY : CMakeFiles/minloadblance.dir/log.cpp.o.requires

CMakeFiles/minloadblance.dir/log.cpp.o.provides: CMakeFiles/minloadblance.dir/log.cpp.o.requires
	$(MAKE) -f CMakeFiles/minloadblance.dir/build.make CMakeFiles/minloadblance.dir/log.cpp.o.provides.build
.PHONY : CMakeFiles/minloadblance.dir/log.cpp.o.provides

CMakeFiles/minloadblance.dir/log.cpp.o.provides.build: CMakeFiles/minloadblance.dir/log.cpp.o


CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o: CMakeFiles/minloadblance.dir/flags.make
CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o: ../xml/tinyxml2.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/onter/project/hub/minloadblance/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o -c /home/onter/project/hub/minloadblance/xml/tinyxml2.cpp

CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/onter/project/hub/minloadblance/xml/tinyxml2.cpp > CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.i

CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/onter/project/hub/minloadblance/xml/tinyxml2.cpp -o CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.s

CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.requires:

.PHONY : CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.requires

CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.provides: CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.requires
	$(MAKE) -f CMakeFiles/minloadblance.dir/build.make CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.provides.build
.PHONY : CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.provides

CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.provides.build: CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o


# Object files for target minloadblance
minloadblance_OBJECTS = \
"CMakeFiles/minloadblance.dir/main.cpp.o" \
"CMakeFiles/minloadblance.dir/log.cpp.o" \
"CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o"

# External object files for target minloadblance
minloadblance_EXTERNAL_OBJECTS =

minloadblance: CMakeFiles/minloadblance.dir/main.cpp.o
minloadblance: CMakeFiles/minloadblance.dir/log.cpp.o
minloadblance: CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o
minloadblance: CMakeFiles/minloadblance.dir/build.make
minloadblance: CMakeFiles/minloadblance.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/onter/project/hub/minloadblance/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable minloadblance"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/minloadblance.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/minloadblance.dir/build: minloadblance

.PHONY : CMakeFiles/minloadblance.dir/build

CMakeFiles/minloadblance.dir/requires: CMakeFiles/minloadblance.dir/main.cpp.o.requires
CMakeFiles/minloadblance.dir/requires: CMakeFiles/minloadblance.dir/log.cpp.o.requires
CMakeFiles/minloadblance.dir/requires: CMakeFiles/minloadblance.dir/xml/tinyxml2.cpp.o.requires

.PHONY : CMakeFiles/minloadblance.dir/requires

CMakeFiles/minloadblance.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/minloadblance.dir/cmake_clean.cmake
.PHONY : CMakeFiles/minloadblance.dir/clean

CMakeFiles/minloadblance.dir/depend:
	cd /home/onter/project/hub/minloadblance/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/onter/project/hub/minloadblance /home/onter/project/hub/minloadblance /home/onter/project/hub/minloadblance/cmake-build-debug /home/onter/project/hub/minloadblance/cmake-build-debug /home/onter/project/hub/minloadblance/cmake-build-debug/CMakeFiles/minloadblance.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/minloadblance.dir/depend

