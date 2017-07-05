include (GenerateExportHeader)

add_library(util ut_avl.c ut_crc.c ut_expand_envvars.c ut_fibheap.c ut_handleserver.c ut_hopscotch.c ut_thread_pool.c ut_xmlparser.c)
generate_export_header(util EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/exports/util/ut_export.h")
target_link_libraries(util PRIVATE OSAPI)
target_include_directories(util PUBLIC "${CMAKE_SOURCE_DIR}/include/" "${CMAKE_BINARY_DIR}/exports/")

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
