cmake_minimum_required(VERSION 3.5)

# Determine build type before including other modules or detect compilers
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug
       CACHE STRING "Choose the type of build : Debug Release RelWithDebInfo MinSizeRel Coverage."
       FORCE)
endif()
message("Build type - ${CMAKE_BUILD_TYPE}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    include(CoverageTarget)
endif()

find_package(Abstraction QUIET REQUIRED)
find_package(CUnit QUIET)

enable_testing()

include_directories(include)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)
