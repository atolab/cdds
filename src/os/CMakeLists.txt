string(TOLOWER ${CMAKE_SYSTEM_NAME} platform)

PREPEND(srcs_os "${CMAKE_SOURCE_DIR}/src/os"
    os_atomics.c
    os_init.c
    os_report.c
    os_socket.c
    os_thread.c
    os_time.c
    os_errno.c
    os_iter.c
    ${srcs_platform}
)

# The includes should reside close to the code. As long as that's not the case,
# pull them in from this CMakeLists.txt.
# Anything PUBLIC can never be absolute paths, so use generator expressions. 
PREPEND(hdrs_public_os "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/os>$<INSTALL_INTERFACE:include/os>"
    os_public.h
    os_decl_attributes.h
    os_decl_attributes_sal.h
)

PREPEND(hdrs_private_os "${CMAKE_SOURCE_DIR}/include/os"
    os.h
    os_atomics.h
    os_atomics_gcc.h
    os_atomics_win32.h
    os_defs.h
    os_errno.h
    os_heap.h
    os_init.h
    os_iter.h
    os_process.h
    os_report.h
    os_socket.h
    os_stdlib.h
    os_sync.h
    os_thread.h
    os_time.h
)

add_library(OSAPI "")

include(${platform}/CMakeLists.txt)

include (GenerateExportHeader)
generate_export_header(OSAPI EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/os/osapi_export.h")

target_sources(OSAPI
    PRIVATE
        ${srcs_os}
        ${hdrs_private_os}
    PUBLIC
        ${hdrs_public_os}
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/os/osapi_export.h>$<INSTALL_INTERFACE:include/os/osapi_export.h>"
)

target_link_libraries(OSAPI INTERFACE Abstraction)
target_include_directories(OSAPI PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>"
                                        "$<INSTALL_INTERFACE:${INSTALL_PREFIX}/include/>" "$<INSTALL_INTERFACE:${INSTALL_PREFIX}/include/>")

get_target_property(osapi_interface_sources OSAPI INTERFACE_SOURCES)
# Not sure why CMake doesn't do this automagically
set_target_properties(OSAPI PROPERTIES PUBLIC_HEADER "${osapi_interface_sources}")

install(
  TARGETS OSAPI
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT lib
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT dev
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
