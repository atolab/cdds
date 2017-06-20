find_path(CRITERION_INC criterion/criterion.h PATH_SUFFIXES criterion)
find_library(CRITERION_LIB criterion)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Criterion DEFAULT_MSG CRITERION_LIB CRITERION_INC)

if (CRITERION_FOUND AND NOT TARGET Criterion)
  add_library(Criterion INTERFACE IMPORTED)

  set_property(TARGET Criterion PROPERTY INTERFACE_LINK_LIBRARIES "${CRITERION_LIB}")
  set_property(TARGET Criterion PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CRITERION_INC}")

  if (DEFINED ENV{WORKSPACE})
    # Jenkins doesn't like colorized output
    set(CRITERION_OUTPUT_MODE "--ascii")
  endif()
endif()
