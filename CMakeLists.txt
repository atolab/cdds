cmake_minimum_required(VERSION 3.5)

# Update the git submodules
execute_process(COMMAND git submodule init
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
execute_process(COMMAND git submodule update
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Set module path before defining project so platform files will work.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_PROJECT_NAME_FULL "Vortex DDS")
string(REPLACE " " "" PROJECT_NAME "${CMAKE_PROJECT_NAME_FULL}")
project(${PROJECT_NAME} VERSION 0.1.0)

# Set some convenience variants of the project-name
string(REPLACE " " "-" CMAKE_PROJECT_NAME_DASHED "${CMAKE_PROJECT_NAME_FULL}")
string(TOUPPER ${CMAKE_PROJECT_NAME} CMAKE_PROJECT_NAME_CAPS)
string(TOLOWER ${CMAKE_PROJECT_NAME} CMAKE_PROJECT_NAME_SMALL)

set(CMAKE_C_STANDARD 99)
if(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
  add_definitions(-std=c99)
endif()

include(FileIDs)
include(GNUInstallDirs)
include(AnalyzeBuild)
# Include Coverage before CTest so that COVERAGE_COMMAND can be modified
# in the Coverage module should that ever be necessary.
include(Coverage)
set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --trace-children=yes --child-silent-after-fork=yes --xml=yes --xml-file=TestResultValgrind_%p.xml --tool=memcheck --show-reachable=yes --leak-resolution=high")
set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/valgrind.supp" CACHE FILEPATH "suppression file")

# By default building the testing tree is enabled by including CTest, but
# since not everybody has CUnit and/or Criterion installed, and because it is
# not strictly required to build the product itself, switch to off by default.
option(BUILD_TESTING "Build the testing tree." OFF)
include(CTest)

# Build all executables and libraries into the top-level /bin and /lib folders.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(etc)
add_subdirectory(idlc)
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tools)
add_subdirectory(scripts)
add_subdirectory(docs)

# Package the JRE for install
install(
  DIRECTORY "$ENV{JAVA_HOME}/jre"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}"
  USE_SOURCE_PERMISSIONS
  COMPONENT dev
  OPTIONAL)


# Pull-in CPack and support for generating <Package>Config.cmake and packages.
include(Packaging)
