find_path(CUNIT_INC CUnit/CUnit.h)
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  SET(SUFFIX "64")
endif()
find_library(CUNIT_LIB cunit${SUFFIX} NAMES libcunit${SUFFIX})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CUnit DEFAULT_MSG CUNIT_LIB CUNIT_INC)

if(CUNIT_FOUND AND NOT TARGET CUnit)
  add_library(CUnit INTERFACE IMPORTED)

  set_property(TARGET CUnit PROPERTY INTERFACE_LINK_LIBRARIES "${CUNIT_LIB}")
  set_property(TARGET CUnit PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CUNIT_INC}")
endif()
