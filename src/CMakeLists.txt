find_package(Abstraction REQUIRED)

include (GenerateExportHeader)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

# Create the os & util target
add_subdirectory(os)
add_subdirectory(util)

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

get_target_property(vddsc_interface_sources vddsc INTERFACE_SOURCES)
# Not sure why CMake doesn't do this automagically
set_target_properties(vddsc PROPERTIES PUBLIC_HEADER "${vddsc_interface_sources}")

target_link_libraries(vddsc PRIVATE util)
set_target_file_ids(vddsc)
# SOVERSION should increase on incompatible ABI change
set_target_properties(vddsc PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

# Generator expressions are used here to refer to different paths during build
# and install phases. It is a requirement for creating relocatable packages.
target_include_directories(vddsc
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>"
        "$<INSTALL_INTERFACE:include>"
    PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>"
)

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
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT dev
)

# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
  add_subdirectory(vddsc/tests)
endif()

