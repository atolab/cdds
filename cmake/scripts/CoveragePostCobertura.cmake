#
# This script assumes that all test have been run and gcov results are available.
# It will generate the Cobertura output from the gcov results.
#
# Example usage:
# $ cmake -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoveragePreCobertura.cmake
# $ ctest -T test
# $ ctest -T coverage
# $ ctest -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoveragePostCobertura.cmake
cmake_minimum_required(VERSION 3.5)

# Some debug
#message(STATUS "Source directory: ${SOURCE_DIR}")
#message(STATUS "Test directory:   ${TEST_DIR}")
#message(STATUS "Output directory: ${OUTPUT_DIR}")


# Find gcovr to generate Cobertura results
find_program(GCOVR_PATH gcovr PARENT_SCOPE)
if(NOT GCOVR_PATH)
    message(FATAL_ERROR "Could not find gcovr to generate Cobertura coverage.")
endif()

# Create location to put the result file.
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Do not include the test and example directories.
#set(EXCLUDE_DIRS -e "${SOURCE_DIR}/${EXAMPLES_DIR}/" -e "${SOURCE_DIR}/${TESTS_DIR}/")

#execute_process(COMMAND ${GCOVR_PATH} -x -r ${SOURCE_DIR} ${EXCLUDE_DIRS} -o ${OUTPUT_DIR}/cobertura.xml
#                WORKING_DIRECTORY ${TEST_DIR})

# Do not include the test and example directories.
set(EXAMPLES_DIR "examples")
set(TESTS_DIR    "examples")

execute_process(COMMAND ${GCOVR_PATH} -x -r ${SOURCE_DIR} -e "${SOURCE_DIR}/${EXAMPLES_DIR}/" -e "${SOURCE_DIR}/${TESTS_DIR}/" -o ${OUTPUT_DIR}/cobertura.xml
                WORKING_DIRECTORY ${TEST_DIR})


message(STATUS "The Cobertura report can be found here: ${OUTPUT_DIR}/cobertura.xml")

