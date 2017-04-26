#
# This script will run all tests and generates various coverage reports.
#
# Example usage
#
#cmake -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoverageGeneration.cmake

cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
message(STATUS "Source directory: ${SOURCE_DIR}")
message(STATUS "Test directory:   ${TEST_DIR}")
message(STATUS "Output directory: ${OUTPUT_DIR}")

# Do not include the test and example directories.
set(EXAMPLES_DIR "examples")
set(TESTS_DIR    "examples")

# Add this flag when you want to suppress LCOV and ctest output.
#set(QUIET_FLAG "--quiet")



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
file(MAKE_DIRECTORY ${OUTPUT_DIR})
if(GENERATE_COVERAGE_HTML)
    set(COVERAGE_HTML_OUTPUT  "${OUTPUT_DIR}/html")
    file(MAKE_DIRECTORY ${COVERAGE_HTML_OUTPUT})
    execute_process(COMMAND ${LCOV_PATH}  ${QUIET_FLAG} --directory . --zerocounters
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



###############################################################################
#
# Generate HTML report when possible
#
###############################################################################
if(GENERATE_COVERAGE_HTML)
    message(STATUS "Generating HTML report")

    set(COVERAGE_INFO    "${COVERAGE_HTML_OUTPUT}/coverage_html.info")
    set(COVERAGE_CLEANED "${COVERAGE_INFO}.cleaned")

    execute_process(COMMAND ${LCOV_PATH} ${QUIET_FLAG} --directory . --capture --output-file ${COVERAGE_INFO}
                    WORKING_DIRECTORY ${TEST_DIR})
    execute_process(COMMAND ${LCOV_PATH} ${QUIET_FLAG} --remove ${COVERAGE_INFO} "${EXAMPLES_DIR}/*" "${TESTS_DIR}/*" "/usr/*" --output-file ${COVERAGE_CLEANED}
                    WORKING_DIRECTORY ${TEST_DIR})
    execute_process(COMMAND ${GENHTML_PATH}  ${QUIET_FLAG} -o ${COVERAGE_HTML_OUTPUT} ${COVERAGE_CLEANED}
                    WORKING_DIRECTORY ${TEST_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${COVERAGE_INFO} ${COVERAGE_CLEANED}
                    WORKING_DIRECTORY ${TEST_DIR})
    message(STATUS "The HTML report can be found here: ${COVERAGE_HTML_OUTPUT}/index.html")
endif()



###############################################################################
#
# Generate Cobertura report when possible
#
###############################################################################
if(GENERATE_COVERAGE_COBERTURA)
    message(STATUS "Generating Cobertura report")
    execute_process(COMMAND ${GCOVR_PATH} -x -r ${SOURCE_DIR} -e "${SOURCE_DIR}/${EXAMPLES_DIR}/" -e "${SOURCE_DIR}/${TESTS_DIR}/" -o ${OUTPUT_DIR}/cobertura.xml
                    WORKING_DIRECTORY ${TEST_DIR})
    message(STATUS "The Cobertura report can be found here: ${OUTPUT_DIR}/cobertura.xml")
endif()

