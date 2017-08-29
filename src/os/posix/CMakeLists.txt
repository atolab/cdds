# The includes should reside close to the code. As long as that's not the case,
# pull them in from this CMakeLists.txt.
# Anything PUBLIC can never be absolute paths, so use generator expressions. 
PREPEND(hdrs_public_posix_os "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/os/posix>$<INSTALL_INTERFACE:include/os/posix>"
    os_platform_public.h
)

PREPEND(hdrs_private_posix_os "${CMAKE_SOURCE_DIR}/include/os/posix"
    os_platform_process.h
    os_platform_socket.h
    os_platform_stdlib.h
    os_platform_sync.h
    os_platform_thread.h
)

# Gather sources for this platform
PREPEND(srcs_posix_os "${CMAKE_CURRENT_LIST_DIR}"
    os_platform_errno.c
    os_platform_heap.c
    os_platform_init.c
    os_platform_process.c
    os_platform_socket.c
    os_platform_stdlib.c
    os_platform_sync.c
    os_platform_thread.c
    os_platform_time.c
)
