find_path(CUNIT_INC CUnit/CUnit.h)
find_library(CUNIT_LIB cunit)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CUnit DEFAULT_MSG CUNIT_LIB CUNIT_INC)

if(CUNIT_FOUND AND NOT TARGET CUnit)
  add_library(CUnit INTERFACE IMPORTED)

  set_property(TARGET CUnit PROPERTY INTERFACE_LINK_LIBRARIES "${CUNIT_LIB}")
  set_property(TARGET CUnit PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CUNIT_INC}")
endif()
