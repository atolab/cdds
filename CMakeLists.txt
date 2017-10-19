cmake_minimum_required(VERSION 3.5)

message("Using deprecated CMakeLists.txt")

# Just a temporary CMakeLists.txt to allow Jenkins builds on both this ticket and
# master during the directory structure transition.
# TODO: Remove this CMakeLists.txt when the rearranged direcories are on master and
#       all Jenkins jobs expect the root CMakeLists.txt on the new location.

# The CMAKE_PROJECT_NAME is dependant on the project in the root CMakeLists.txt.
# This means we have to declare the project here as well to be able to use that
# specific variable (which we do).
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/src/cmake/modules")
set(CMAKE_PROJECT_NAME_FULL "Vortex DDS")
string(REPLACE " " "" PROJECT_NAME "${CMAKE_PROJECT_NAME_FULL}")
project(${PROJECT_NAME} VERSION 0.1.0)

# We also need to line up test related directories by including CTest in the
# top level CMakeLists.txt
option(BUILD_TESTING "Build the testing tree." OFF)
set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --trace-children=yes --child-silent-after-fork=yes --xml=yes --xml-file=TestResultValgrind_%p.xml --tool=memcheck --show-reachable=yes --leak-resolution=high")
set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_CURRENT_LIST_DIR}/src/valgrind.supp" CACHE FILEPATH "suppression file")
include(CTest)

# Now continue doing the actual work.
add_subdirectory(src)

