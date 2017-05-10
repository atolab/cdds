find_package(CUnit)

if(CUNIT_FOUND)
  include (GenerateExportHeader)
  add_library(cunitrunner ${LIB_TYPE} src/runner.c)
  generate_export_header(cunitrunner)
  target_link_libraries(cunitrunner PUBLIC CUnit PRIVATE OSAbstraction)
  target_include_directories(cunitrunner PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_BINARY_DIR}/cunitrunner")
endif(CUNIT_FOUND)
