find_package(Abstraction REQUIRED)

include (GenerateExportHeader)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)


option(DDSC_SHARED "Build DDSC as a shared library" ON)

if(DDSC_SHARED AND ((NOT DEFINED BUILD_SHARED_LIBS) OR BUILD_SHARED_LIBS))
  # BUILD_SHARED_LIBS is set to off by for example VxWorks DKM environment
  add_library(ddsc SHARED "")
else()
  if(DDSC_SHARED)
    message(STATUS "Option DDSC_SHARED ignored. Only static libraries supported on this platform.")
  endif()
  add_library(ddsc "")
endif()

include(ddsi/CMakeLists.txt)
include(ddsc/CMakeLists.txt)
include(security/CMakeLists.txt)

target_link_libraries(ddsc PRIVATE util)
target_link_libraries(ddsc PRIVATE OSAPI)

# SOVERSION should increase on incompatible ABI change
set_target_properties(ddsc PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

get_target_property(os_api_src_dir OSAPI SOURCE_DIR)
# We need to expose some of the OS headers as well.
target_include_directories(ddsc
    PUBLIC
        "$<BUILD_INTERFACE:${os_api_src_dir}/include>")

set_target_file_ids(ddsc)

# Create a pseudo-target that other targets (i.e. examples, tests) can depend
# on and can also be provided as import-target by a package-file when building
# those targets outside the regular Vortex build-tree (i.e. the installed tree)
add_library(${CMAKE_PROJECT_NAME}::ddsc ALIAS ddsc)

install(
  TARGETS ddsc
  EXPORT "${CMAKE_PROJECT_NAME}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT lib
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
)


