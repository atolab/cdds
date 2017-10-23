string(TOLOWER ${CMAKE_SYSTEM_NAME} platform)

# For posix platforms include the files in the posix/ directory.
set (posix_platforms darwin linux)
IF(${platform} IN_LIST posix_platforms)
  set(platform posix)
ENDIF()

PREPEND(srcs_platform ${platform} os_platform_errno.c os_platform_heap.c os_platform_init.c os_platform_process.c os_platform_socket.c os_platform_stdlib.c os_platform_sync.c os_platform_thread.c os_platform_time.c)

include (GenerateExportHeader)
PREPEND(srcs_os "${CMAKE_SOURCE_DIR}/src/os" os_atomics.c os_init.c os_report.c os_socket.c os_thread.c os_time.c os_errno.c os_iter.c ${srcs_platform})
add_library(OSAPI ${srcs_os})
generate_export_header(OSAPI EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/exports/os/osapi_export.h")
target_link_libraries(OSAPI INTERFACE Abstraction)
target_include_directories(OSAPI PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/exports/>"
                                        "$<INSTALL_INTERFACE:${INSTALL_PREFIX}/include/>" "$<INSTALL_INTERFACE:${INSTALL_PREFIX}/exports/>")

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(
  FILES "${CMAKE_SOURCE_DIR}/include/os/os_public.h" "${CMAKE_SOURCE_DIR}/include/os/os_decl_attributes.h" "${CMAKE_SOURCE_DIR}/include/os/os_decl_attributes_sal.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/os"
  COMPONENT dev)

# Currently, only windows and posix platforms are supported.
IF(WIN32 AND NOT UNIX)
    install(
      FILES "${CMAKE_SOURCE_DIR}/include/os/windows/os_platform_public.h"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/os"
      COMPONENT dev)
ELSE()
    install(
      FILES "${CMAKE_SOURCE_DIR}/include/os/posix/os_platform_public.h"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/os"
      COMPONENT dev)
ENDIF()

