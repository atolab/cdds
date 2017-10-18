find_package(Abstraction REQUIRED)

include (GenerateExportHeader)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)


option(VDDSC_SHARED "Build VDDSC as a shared library" ON)

if(VDDSC_SHARED AND ((NOT DEFINED BUILD_SHARED_LIBS) OR BUILD_SHARED_LIBS))
  # BUILD_SHARED_LIBS is set to off by for example VxWorks DKM environment
  add_library(vddsc SHARED "")
else()
  if(VDDSC_SHARED)
    message(STATUS "Option VDDSC_SHARED ignored. Only static libraries supported on this platform.")
  endif()
  add_library(vddsc "")
endif()

include(ddsi/CMakeLists.txt)
include(vddsc/CMakeLists.txt)

target_link_libraries(vddsc PRIVATE util)
target_link_libraries(vddsc PRIVATE OSAPI)

# SOVERSION should increase on incompatible ABI change
set_target_properties(vddsc PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

get_target_property(os_api_src_dir OSAPI SOURCE_DIR)
# We need to expose some of the OS headers as well.
target_include_directories(vddsc
    PUBLIC
        "$<BUILD_INTERFACE:${os_api_src_dir}/include>")

set_target_file_ids(vddsc)

# Create a pseudo-target that other targets (i.e. examples, tests) can depend
# on and can also be provided as import-target by a package-file when building
# those targets outside the regular Vortex build-tree (i.e. the installed tree)
add_library(${CMAKE_PROJECT_NAME}::vddsc ALIAS vddsc)

install(
  TARGETS vddsc
  EXPORT "${CMAKE_PROJECT_NAME}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT lib
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
)

