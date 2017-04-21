cmake_minimum_required(VERSION 3.5)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug
       CACHE STRING "Choose the type of build : Debug Release RelWithDebInfo MinSizeRel Coverage."
       FORCE)
endif()
message("Build type - ${CMAKE_BUILD_TYPE}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

include(CoverageTarget)

enable_testing()

find_package(Abstraction QUIET REQUIRED)

include_directories(include)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)
