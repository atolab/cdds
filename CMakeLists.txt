cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

find_package(Abstraction QUIET REQUIRED)

enable_testing()

include_directories(include)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)
