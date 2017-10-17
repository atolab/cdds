if("${CMAKE_BUILD_TYPE}" STREQUAL "Coverage")
    set(BUILD_TYPE_SUPPORTED False)
    mark_as_advanced(BUILD_TYPE_SUPPORTED)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(BUILD_TYPE_SUPPORTED True)
    elseif(("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang") AND
           ("${CMAKE_C_COMPILER_VERSION}" VERSION_GREATER "3.0.0"))
        set(BUILD_TYPE_SUPPORTED True)
    endif()

    if(NOT BUILD_TYPE_SUPPORTED)
        message(FATAL_ERROR "Coverage build type not supported. (GCC or Clang "
                            ">3.0.0 required)")
    endif()

    # NOTE: Since either GCC or Clang is required for now, and the coverage
    #       flags are the same for both, there is no need for seperate branches
    #       to set compiler flags. That might change in the future.

    # CMake has per build type compiler and linker flags. If 'Coverage' is
    # chosen, the flags below are automatically inserted into CMAKE_C_FLAGS.
    #
    # Any optimizations are disabled to ensure coverage results are correct.
    # See https://gcc.gnu.org/onlinedocs/gcc/Gcov-and-Optimization.html.
    set(CMAKE_C_FLAGS_COVERAGE
        "-DNDEBUG -g -O0 --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS_COVERAGE
        "-DNDEBUG -g -O0 --coverage -fprofile-arcs -ftest-coverage")
    mark_as_advanced(
        CMAKE_C_FLAGS_COVERAGE
        CMAKE_CXX_FLAGS_COVERAGE
        CMAKE_EXE_LINKER_FLAGS_COVERAGE
        CMAKE_SHARED_LINKER_FLAGS_COVERAGE)

    configure_file(${CMAKE_MODULE_PATH}/../CoverageSettings.cmake.in CoverageSettings.cmake @ONLY)

    message(STATUS "Coverage build type available")
endif()

