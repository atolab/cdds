find_package(CUnit REQUIRED)

include (GenerateExportHeader)
add_library(cunitrunner ${LIB_TYPE} src/runner.c)
generate_export_header(cunitrunner)
target_link_libraries(cunitrunner PUBLIC CUnit PRIVATE OSAPI)
target_include_directories(cunitrunner PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_BINARY_DIR}/cunitrunner")

