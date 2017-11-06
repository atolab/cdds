# This file enables warnings for all targets defined after this point.
# It would be more flexible for it to export (alias) interface targets,
# allowing a target to depend on it (and pass options potentially). 
# For now however, this suffices, meaning all code must adhere to the
# same warning-levels.

if(WARNINGS_INCLUDED)
  return()
endif()
set(WARNINGS_INCLUDED true)

add_compile_options(
    # CMake does /W3 for MSVC by default, which is what we want.
    $<$<C_COMPILER_ID:Clang>:-Wall>
    $<$<C_COMPILER_ID:Clang>:-Wextra>
    # We don't want initializers like {0} to cause a warning (and {} is C++ unfortunately, and not accepted by MSVC)
    $<$<C_COMPILER_ID:Clang>:-Wno-missing-field-initializers>
#   $<$<C_COMPILER_ID:Clang>:-Weverything> # If you're ready for the madhouse or want to become ready
    $<$<C_COMPILER_ID:GNU>:-Wall>
    $<$<C_COMPILER_ID:GNU>:-Wextra>
    $<$<C_COMPILER_ID:GNU>:-Wno-missing-field-initializers>
#   $<$<C_COMPILER_ID:GNU>:-Wpedantic>
)

add_library(Warnings::NoWall INTERFACE IMPORTED)
set_property(
    TARGET Warnings::NoWall
    PROPERTY INTERFACE_COMPILE_OPTIONS
        $<$<C_COMPILER_ID:Clang>:-Wno-all>
        $<$<C_COMPILER_ID:GNU>:-Wno-all>
)

add_library(Warnings::NoWextra INTERFACE IMPORTED)
set_property(
    TARGET Warnings::NoWextra
    PROPERTY INTERFACE_COMPILE_OPTIONS
        $<$<C_COMPILER_ID:Clang>:-Wno-extra>
        $<$<C_COMPILER_ID:GNU>:-Wno-extra>
)

add_library(Warnings::NoUnusedParameter INTERFACE IMPORTED)
set_property(
    TARGET Warnings::NoUnusedParameter
    PROPERTY INTERFACE_COMPILE_OPTIONS
        $<$<C_COMPILER_ID:Clang>:-Wno-unused-parameter>
        $<$<C_COMPILER_ID:GNU>:-Wno-unused-parameter>
)
