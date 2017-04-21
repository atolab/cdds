cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

find_package(Abstraction QUIET REQUIRED)

# toggle to include tests for examples; set to ON/OFF to enable/disable example testing
set(CMAKE_TEST_EXAMPLES ON)

# Add tests for this example; ctest requires this commend to be in the root
if (CMAKE_TEST_EXAMPLES)
    message(STATUS "Include example testing")
    enable_testing()
endif()

include_directories(include)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)
