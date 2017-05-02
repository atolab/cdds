add_definitions(-DVDDS_BUILD=1)

string(TOLOWER ${CMAKE_SYSTEM_NAME} platform)

# For posix platforms include the files in the posix/ directory.
set (posix_platforms darwin linux)
IF(${platform} IN_LIST posix_platforms)
  set(platform posix)
ENDIF()

PREPEND(srcs_platform ${platform} os_platform_errno.c os_platform_heap.c os_platform_init.c os_platform_process.c os_platform_socket.c os_platform_stdlib.c os_platform_sync.c os_platform_thread.c os_platform_time.c)

# Recipe for the OSAbstraction target
PREPEND(srcs_os "${CMAKE_SOURCE_DIR}/src/os" os_atomics.c os_init.c os_report.c os_socket.c os_thread.c os_time.c ${srcs_platform})
add_library(OSAbstraction INTERFACE)
target_sources(OSAbstraction INTERFACE ${srcs_os})
target_link_libraries(OSAbstraction INTERFACE Abstraction)
