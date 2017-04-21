# Find the CUnit headers and libraries
#
#  CUNIT_INCLUDE_DIRS - The CUnit include directory (directory where CUnit/CUnit.h was found)
#  CUNIT_LIBRARIES    - The libraries needed to use CUnit
#  CUNIT_FOUND        - True if CUnit found in system
#
# This piece of code is inspired by https://gist.github.com/adobkin/1073354


message(STATUS "Lookup CUnit package")

find_path(CUNIT_INCLUDE_DIR NAMES CUnit/CUnit.h)
find_library(CUNIT_LIBRARY NAMES cunit libcunit cunitlib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CUNIT DEFAULT_MSG CUNIT_LIBRARY CUNIT_INCLUDE_DIR)

IF(CUNIT_FOUND)
  message(STATUS "Loading CUnit libraries")
  SET(CUNIT_LIBRARIES ${CUNIT_LIBRARY})
  SET(CUNIT_INCLUDE_DIRS ${CUNIT_INCLUDE_DIR})
ENDIF(CUNIT_FOUND)
