cmake_minimum_required(VERSION 3.5)

# Set module path before defining project for platform files to work
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

# Include Coverage before CTest so that COVERAGE_COMMAND can be modified
# in the Coverage module should that ever be necessary.
include(Coverage)
include(CTest)

find_package(Abstraction QUIET REQUIRED)
find_package(CUnit QUIET)

enable_testing()

include_directories(include)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)
