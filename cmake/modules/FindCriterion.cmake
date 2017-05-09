# This file is licensed under the WTFPL version 2 -- you can see the full
# license over at http://www.wtfpl.net/txt/copying/
#
# - Try to find Criterion
#
# Once done this will define
#  CRITERION_FOUND - System has Criterion
#  CRITERION_INC - The Criterion include directories
#  CRITERION_LIB - The libraries needed to use Criterion
#
# This file is inspired by https://github.com/Snaipe/Criterion/blob/bleeding/dev/FindCriterion.cmake

find_package(PkgConfig)

find_path(CRITERION_INC criterion/criterion.h PATH_SUFFIXES criterion)
find_library(CRITERION_LIB NAMES criterion libcriterion)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Criterion DEFAULT_MSG CRITERION_LIB CRITERION_INC)

if (CRITERION_FOUND AND NOT TARGET Criterion)
  add_library(Criterion INTERFACE IMPORTED)

  set_property(TARGET Criterion PROPERTY INTERFACE_LINK_LIBRARIES "${CRITERION_LIB}")
  set_property(TARGET Criterion PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CRITERION_INC}")
endif()

