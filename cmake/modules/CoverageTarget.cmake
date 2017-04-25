message(STATUS "Loading Coverage target")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
        message(FATAL_ERROR "Can not load Coverage target (clang version must be 3.0.0 or greater)")
    endif()
elseif(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(GENERATE_COVERAGE FALSE)
    message(FATAL_ERROR "Can not load Coverage target (compiler is not GNU gcc)")
endif()

find_program(GCOV_PATH gcov PARENT_SCOPE)
if(NOT GCOV_PATH)
    message(FATAL_ERROR "Can not load  Coverage target (could not find gcov)")
endif()


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

