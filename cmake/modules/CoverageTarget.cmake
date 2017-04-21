
set(GENERATE_COVERAGE TRUE)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
        set(GENERATE_COVERAGE FALSE)
        message(STATUS "Skipping Coverage target (clang version must be 3.0.0 or greater)")
    endif()
elseif(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(GENERATE_COVERAGE FALSE)
    message(STATUS "Skipping Coverage target (compiler is not GNU gcc)")
endif()

if(GENERATE_COVERAGE)
    find_program(GCOV_PATH gcov PARENT_SCOPE)
    if(NOT GCOV_PATH)
        set(GENERATE_COVERAGE FALSE)
        message(STATUS "Skipping Coverage target (could not find gcov)")
    endif()
endif()

if(GENERATE_COVERAGE)
    message(STATUS "Loading Coverage target")

    #
    # Quote from cmake.org:
    #   | There are many per-config properties and variables (usually following clean
    #   | SOME_VAR_<CONFIG> order conventions), such as CMAKE_C_FLAGS_<CONFIG>, specified
    #   | as uppercase: CMAKE_C_FLAGS_[DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL].
    #   | For example, in a build tree configured to build type Debug, CMake will see to
    #   | having CMAKE_C_FLAGS_DEBUG settings get added to the CMAKE_C_FLAGS settings.
    # This will mean that if you choose 'Coverage' as build target
    #   $ cmake <src_root> -DCMAKE_BUILD_TYPE=Coverage
    # the following build flags are automatically used when building chameleon.
    #
    set(CMAKE_CXX_FLAGS_COVERAGE
        "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C++ compiler during coverage builds."
        FORCE )
    set(CMAKE_C_FLAGS_COVERAGE
        "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C compiler during coverage builds."
        FORCE )
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used for linking binaries during coverage builds."
        FORCE )
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used by the shared libraries linker during coverage builds."
        FORCE )
    mark_as_advanced(
        CMAKE_CXX_FLAGS_COVERAGE
        CMAKE_C_FLAGS_COVERAGE
        CMAKE_EXE_LINKER_FLAGS_COVERAGE
        CMAKE_SHARED_LINKER_FLAGS_COVERAGE )
else()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
    if(BUILD_TYPE_LOWER STREQUAL "coverage")
        message(FATAL_ERROR "Build type is ${CMAKE_BUILD_TYPE}, but this is not supported on this platform")
    endif()
endif()





set(GENERATE_COVERAGE_HTML GENERATE_COVERAGE)

if(GENERATE_COVERAGE_HTML)
    find_program(LCOV_PATH lcov PARENT_SCOPE)
    if(NOT LCOV_PATH)
        set(GENERATE_COVERAGE_HTML FALSE)
        message(STATUS "Skipping Coverage analyzing (could not find lcov)")
    endif()
endif()

if(GENERATE_COVERAGE_HTML)
    find_program(GENHTML_PATH genhtml PARENT_SCOPE)
    if(NOT GENHTML_PATH)
        set(GENERATE_COVERAGE_HTML FALSE)
        message(STATUS "Skipping Coverage analyzing (could not find genhtml)")
    endif()
endif()




# Param RUNNER        The name of the target which runs the tests.
#                              No coverage will be reported if RUNNER does
#                              not return ZERO.
# Param OUTPUT_NAME   Output directory (and file(s) prefix).
# Param ARGV2         Optional. Will be passes to RUNNER ("-x;2" for -x 2)
function(setup_target_coverage RUNNER OUTPUT_NAME)

    if(GENERATE_COVERAGE)
        set(TARGET_NAME Coverage)


        separate_arguments(test_command UNIX_COMMAND "${RUNNER}")

        if(GENERATE_COVERAGE_HTML)
            set(COVERAGE_INFO "${CMAKE_BINARY_DIR}/${OUTPUT_NAME}.info")
            set(COVERAGE_CLEANED "${COVERAGE_INFO}.cleaned")

            # Setup lcov target
            add_custom_target(${TARGET_NAME}
                # Cleanup lcov
                ${LCOV_PATH} --directory . --zerocounters

                # Run tests
                COMMAND ${test_command} ${ARGV2}

                # Capturing lcov counters and generating report
                COMMAND ${LCOV_PATH} --directory . --capture --output-file ${COVERAGE_INFO}
                COMMAND ${LCOV_PATH} --remove ${COVERAGE_INFO} 'tests/*' '/usr/*' ${LCOV_REMOVE_EXTRA} --output-file ${COVERAGE_CLEANED}
                COMMAND ${GENHTML_PATH} -o ${OUTPUT_NAME} ${COVERAGE_CLEANED}
                COMMAND ${CMAKE_COMMAND} -E remove ${COVERAGE_INFO} ${COVERAGE_CLEANED}

                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Running ${test_command} ${ARGV2}\nResetting code coverage counters to zero.\nProcessing code coverage counters and generating report."
            )

            # Show info where to find the report
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ;
                COMMENT "Open ${CMAKE_BINARY_DIR}/${OUTPUT_NAME}/index.html in your browser to view the coverage report."
            )
        else(GENERATE_COVERAGE_HTML)
            set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/${OUTPUT_NAME}")

            # Setup gcov target
            add_custom_target(${TARGET_NAME}
                # Run tests
                COMMAND ${test_command} ${ARGV2}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Running ${test_command} ${ARGV2}"
            )

            # Show info where to find the report
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND rm -rf "${COVERAGE_DIR}"
                COMMAND mkdir -p "${COVERAGE_DIR}"
                COMMAND find "${CMAKE_BINARY_DIR}" -path "${COVERAGE_DIR}" -prune -o -type f -name *.gcda -exec cp -f {} "${COVERAGE_DIR}" \\\\;
                COMMAND find "${CMAKE_BINARY_DIR}" -path "${COVERAGE_DIR}" -prune -o -type f -name *.gcno -exec cp -f {} "${COVERAGE_DIR}" \\\\;
                COMMAND cd "${COVERAGE_DIR}" \\; gcov *
                COMMENT "Collect data files and execute gcof"
            )
        endif(GENERATE_COVERAGE_HTML)
    endif(GENERATE_COVERAGE)

endfunction() # SETUP_TARGET_FOR_COVERAGE




