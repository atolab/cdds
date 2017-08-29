# The includes should reside close to the code. As long as that's not the case,
# pull them in from this CMakeLists.txt.

PREPEND(hdrs_private_${platform}_os "${CMAKE_SOURCE_DIR}/include/os/${platform}"
    os_platform.h
)

# Gather sources for this platform from generic Posix implementation
include("${CMAKE_CURRENT_SOURCE_DIR}/posix/CMakeLists.txt") # Posix-CMakeLists.txt doesn't extend the target

target_sources(OSAPI
    PRIVATE
        ${srcs_posix_os}
        ${hdrs_private_posix_os}
        ${hdrs_private_${platform}_os}
    PUBLIC
        ${hdrs_public_posix_os}
)
