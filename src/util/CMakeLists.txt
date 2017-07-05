include (GenerateExportHeader)

add_library(util ut_avl.c ut_crc.c ut_expand_envvars.c ut_fibheap.c ut_handleserver.c ut_hopscotch.c ut_thread_pool.c ut_xmlparser.c)
generate_export_header(util EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/exports/util/ut_export.h")
target_link_libraries(util PUBLIC OSAPI)
target_include_directories(util PUBLIC "${CMAKE_SOURCE_DIR}/include/" "${CMAKE_BINARY_DIR}/exports/")

# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
  add_subdirectory(tests)
endif()
