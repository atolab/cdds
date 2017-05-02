#
# This script assumes that it is called before all tests are run and gcov results are available.
# It can be used to setup the environment needed to get proper HTML coverage results.
#
# Example usage:
# $ cmake -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoveragePreHtml.cmake
# $ ctest -T test
# $ ctest -T coverage
# $ ctest -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoveragePostHtml.cmake
cmake_minimum_required(VERSION 3.5)

# Some debug
message(STATUS "Source directory: ${SOURCE_DIR}")
message(STATUS "Test directory:   ${TEST_DIR}")
message(STATUS "Output directory: ${OUTPUT_DIR}")

# Add this flag when you want to suppress LCOV output.
#set(QUIET_FLAG "--quiet")


# Find tools to generate HTML coverage results
find_program(LCOV_PATH lcov PARENT_SCOPE)
if(NOT LCOV_PATH)
    message(FATAL_ERROR "Could not find lcov to generate HTML coverage.")
endif()
find_program(GENHTML_PATH genhtml PARENT_SCOPE)
if(NOT GENHTML_PATH)
    message(FATAL_ERROR "Could not find genhtml to generate HTML coverage.")
endif()

# Reset LCOV environment
execute_process(COMMAND ${LCOV_PATH}  ${QUIET_FLAG} --directory . --zerocounters
                WORKING_DIRECTORY ${TEST_DIR})


