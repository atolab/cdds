#
# This script will run all tests and generates various coverage reports.
#
# Example usage
#
#cmake -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoverageConvenience.cmake

cmake_minimum_required(VERSION 3.5)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
message(STATUS "Source directory: ${SOURCE_DIR}")
message(STATUS "Test directory:   ${TEST_DIR}")
message(STATUS "Output directory: ${OUTPUT_DIR}")

# Do not include the test and example directories.
set(EXAMPLES_DIR "examples")
set(TESTS_DIR    "examples")


###############################################################################
#
# Detect generators
#
###############################################################################
set(GENERATE_COVERAGE TRUE)
if(GENERATE_COVERAGE)
    find_program(GCOV_PATH gcov PARENT_SCOPE)
    if(NOT GCOV_PATH)
        set(GENERATE_COVERAGE FALSE)
        message(STATUS "[SKIP] Coverage generators - gcov (could not find gcov)")
    endif()
endif()
if(GENERATE_COVERAGE)
    message(STATUS "[ OK ] Coverage generators - gcov")
endif()

set(GENERATE_COVERAGE_HTML TRUE)
if(GENERATE_COVERAGE_HTML)
    find_program(LCOV_PATH lcov PARENT_SCOPE)
    if(NOT LCOV_PATH)
        set(GENERATE_COVERAGE_HTML FALSE)
        message(STATUS "[SKIP] Coverage generators - HTML (could not find lcov)")
    endif()
endif()
if(GENERATE_COVERAGE_HTML)
    find_program(GENHTML_PATH genhtml PARENT_SCOPE)
    if(NOT GENHTML_PATH)
        set(GENERATE_COVERAGE_HTML FALSE)
        message(STATUS "[SKIP] Coverage generators - HTML (could not find genhtml)")
    endif()
endif()
if(GENERATE_COVERAGE_HTML)
    message(STATUS "[ OK ] Coverage generators - HTML (lcov and genhtml)")
endif()

set(GENERATE_COVERAGE_COBERTURA TRUE)
if(GENERATE_COVERAGE_COBERTURA)
    find_program(GCOVR_PATH gcovr PARENT_SCOPE)
    if(NOT GCOVR_PATH)
        set(GENERATE_COVERAGE_COBERTURA FALSE)
        message(STATUS "[SKIP] Coverage generators - Cobertura (could not find gcovr)")
    endif()
endif()
if(GENERATE_COVERAGE_COBERTURA)
    message(STATUS "[ OK ] Coverage generators - Cobertura (gcovr)")
endif()

if(NOT GENERATE_COVERAGE)
    message(FATAL_ERROR "Could not find the main coverage generator 'gcov'")
elseif(NOT GENERATE_COVERAGE_HTML AND NOT GENERATE_COVERAGE_COBERTURA)
    message(FATAL_ERROR "Could not find either of the two coverage report generators")
endif()



###############################################################################
#
# Setup environment
#
###############################################################################
message(STATUS "Setup environment")
if(GENERATE_COVERAGE_HTML)
    execute_process(COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${SOURCE_DIR} -DTEST_DIR=${TEST_DIR} -DOUTPUT_DIR=${OUTPUT_DIR} -P ${SOURCE_DIR}/cmake/scripts/CoveragePreHtml.cmake
                    WORKING_DIRECTORY ${TEST_DIR})
endif()
if(GENERATE_COVERAGE_COBERTURA)
    execute_process(COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${SOURCE_DIR} -DTEST_DIR=${TEST_DIR} -DOUTPUT_DIR=${OUTPUT_DIR} -P ${SOURCE_DIR}/cmake/scripts/CoveragePreCobertura.cmake
                    WORKING_DIRECTORY ${TEST_DIR})
endif()


###############################################################################
#
# Generate coverage results by running all the tests
#
###############################################################################
message(STATUS "Run all test to get coverage")
execute_process(COMMAND ctest ${QUIET_FLAG} -T test
                WORKING_DIRECTORY ${TEST_DIR})
execute_process(COMMAND ctest ${QUIET_FLAG} -T coverage
                WORKING_DIRECTORY ${TEST_DIR})



###############################################################################
#
# Generate coverage reports
#
###############################################################################
if(GENERATE_COVERAGE_HTML)
    execute_process(COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${SOURCE_DIR} -DTEST_DIR=${TEST_DIR} -DOUTPUT_DIR=${OUTPUT_DIR} -P ${SOURCE_DIR}/cmake/scripts/CoveragePostHtml.cmake
                    WORKING_DIRECTORY ${TEST_DIR})
endif()
if(GENERATE_COVERAGE_COBERTURA)
    execute_process(COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${SOURCE_DIR} -DTEST_DIR=${TEST_DIR} -DOUTPUT_DIR=${OUTPUT_DIR} -P ${SOURCE_DIR}/cmake/scripts/CoveragePostCobertura.cmake
                    WORKING_DIRECTORY ${TEST_DIR})
endif()

