# The includes should reside close to the code. As long as that's not the case,
# pull them in from this CMakeLists.txt.
# Anything PUBLIC can never be absolute paths, so use generator expressions. 
PREPEND(hdrs_public_posix_os "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/os/posix>$<INSTALL_INTERFACE:include/os/posix>"
    os_platform_public.h
)

PREPEND(hdrs_private_${platform}_os "${CMAKE_SOURCE_DIR}/include/os/${platform}"
    os_platform.h
)

# Gather sources for this platform from generic Posix implementation
include("${CMAKE_CURRENT_SOURCE_DIR}/posix/CMakeLists.txt") # Posix-CMakeLists.txt doesn't extend the target

# Gather sources for this platform
PREPEND(srcs_${platform}_os "${CMAKE_CURRENT_LIST_DIR}"
    os_platform_errno.c
    os_platform_heap.c
    os_platform_init.c
    os_platform_process.c
    os_platform_socket.c
    os_platform_stdlib.c
    os_platform_sync.c
    os_platform_thread.c
    os_platform_time.c
    # Actually, the snippets making up the sources for this platform should be
    # included here instead of C-files including the snippets.
)

target_sources(OSAPI
    PRIVATE
        ${srcs_${platform}_os}
        ${hdrs_private_posix_os}
        ${hdrs_private_${platform}_os}
    PUBLIC
        ${hdrs_public_posix_os}
)
