cmake_minimum_required(VERSION 3.5)

# Set module path before defining project for platform files to work
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

option(ANALYZE_BUILD "Enable static analysis during build" OFF)
if(ANALYZE_BUILD)
  include(AnalyzeBuild)
endif()

# Include Coverage before CTest so that COVERAGE_COMMAND can be modified
# in the Coverage module should that ever be necessary.
include(Coverage)
include(CTest)
enable_testing()

include_directories(include)
#add_subdirectory(idlc)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
add_subdirectory(cunitrunner)
#add_subdirectory(rpc-examples)
