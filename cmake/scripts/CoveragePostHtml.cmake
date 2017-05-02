#
# This script assumes that all test have been run and gcov results are available.
# It will generate the HTML output from the gcov results.
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

# Create location to put the result file.
set(COVERAGE_HTML_OUTPUT  "${OUTPUT_DIR}/html")
file(MAKE_DIRECTORY ${COVERAGE_HTML_OUTPUT})

# Setup tmp analysis files
set(COVERAGE_INFO    "${COVERAGE_HTML_OUTPUT}/coverage_html.info")
set(COVERAGE_CLEANED "${COVERAGE_INFO}.cleaned")

# Do not include the test and example directories.
set(EXAMPLES_DIR "examples")
set(TESTS_DIR    "examples")

# Execute lcov and genhtml commands to get HTML results
execute_process(COMMAND ${LCOV_PATH} ${QUIET_FLAG} --directory . --capture --output-file ${COVERAGE_INFO}
                WORKING_DIRECTORY ${TEST_DIR})
execute_process(COMMAND ${LCOV_PATH} ${QUIET_FLAG} --remove ${COVERAGE_INFO} "${EXAMPLES_DIR}/*" "${TESTS_DIR}/*" "/usr/*" --output-file ${COVERAGE_CLEANED}
                WORKING_DIRECTORY ${TEST_DIR})
execute_process(COMMAND ${GENHTML_PATH}  ${QUIET_FLAG} -o ${COVERAGE_HTML_OUTPUT} ${COVERAGE_CLEANED}
                WORKING_DIRECTORY ${TEST_DIR})

# Remove tmp analysis files
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${COVERAGE_INFO} ${COVERAGE_CLEANED}
                WORKING_DIRECTORY ${TEST_DIR})


message(STATUS "The HTML coverage report can be found here: ${COVERAGE_HTML_OUTPUT}/index.html")

