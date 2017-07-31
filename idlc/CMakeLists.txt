# Verify Maven is available
find_package(Maven 3.0 REQUIRED)

file(GLOB_RECURSE IDLC_G4_SOURCES LIST_DIRECTORIES true *.g4)
file(GLOB_RECURSE IDLC_JAVA_SOURCES LIST_DIRECTORIES true *.java)
file(GLOB_RECURSE IDLC_ST_SOURCES LIST_DIRECTORIES true *.st?)

set(LINE_ENDINGS "UNIX")
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(EXTENSION ".bat")
    set(LINE_ENDINGS "WIN32")
endif()

set(IDLC_JAR "${CMAKE_CURRENT_BINARY_DIR}/target/idlc-jar-with-dependencies.jar")
set(IDLC_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "")
set(IDLC "dds_idlc${EXTENSION}" CACHE STRING "")
mark_as_advanced(IDLC_JAR IDLC_DIR IDLC)

configure_file(
    "cmake/dds_idlc${EXTENSION}.in" "${IDLC}"
    @ONLY
    NEWLINE_STYLE ${LINE_ENDINGS})

# FIXME: C++ IDL compiler depends idlpp. Leave it disabled for now.
#configure_file(
#    "cmake/dds_idlcpp${EXTENSION}.in"
#    "dds_idlcpp${EXTENSION}"
#    @ONLY
#    NEWLINE_STYLE ${LINE_ENDINGS})

if(NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows"))
    execute_process(COMMAND chmod +x "idlc/${IDLC}")
endif()

# Maven is executed within the idlc directory located in the build directory
# and generated sources are stored in idlc/target. Non-generated sources,
# however, do not reside in the build directory and Maven must be instructed
# to use those. To allow maven to be executed from both the source and build
# directories (idlc may be moved to it's own repository), the pom.xml file is
# pulled through the configure_file function with basedir set to the original
# source directory. It is a cute little hack to avoid having to use different
# profiles etc.
set(basedir "${CMAKE_CURRENT_SOURCE_DIR}")
mark_as_advanced(basedir)
configure_file("pom.xml" "pom.xml")
configure_file("com/prismtech/vortex/Version.java.in" "com/prismtech/vortex/Version.java")

add_custom_command(
    OUTPUT  "${IDLC_JAR}"
    COMMAND "${Maven_EXECUTABLE}"
    ARGS    "-q" "package"
    DEPENDS "pom.xml" ${IDLC_G4_SOURCES} ${IDLC_JAVA_SOURCES} ${IDLC_ST_SOURCES})

add_custom_target(idlc ALL DEPENDS "${IDLC_JAR}")

install(
  FILES "${IDLC_DIR}/${IDLC}"
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  COMPONENT dev)

function(IDLC_GENERATE _target)
  if(NOT ARGN)
    message(FATAL_ERROR "idlc_generate called without any idl files")
  endif()

  set(_dir "${CMAKE_CURRENT_BINARY_DIR}")
  set(_sources)
  set(_headers)
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    set(_source "${_dir}/${FIL_WE}.c")
    set(_header "${_dir}/${FIL_WE}.h")
    list(APPEND _sources "${_source}")
    list(APPEND _headers "${_header}")

    add_custom_command(
      OUTPUT   "${_source}" "${_header}"
      COMMAND  "${IDLC_DIR}/${IDLC}"
      ARGS     "${ABS_FIL}"
      DEPENDS  "${ABS_FIL}" idlc
      COMMENT  "Running idlc on ${FIL}"
      VERBATIM)
  endforeach()

  set_source_files_properties(
    ${_sources} ${_headers} PROPERTIES GENERATED TRUE)
  add_library(${_target} INTERFACE)
  target_sources(${_target} INTERFACE ${_sources})
  target_include_directories(${_target} INTERFACE "${_dir}")
endfunction()

function(IDLC_INSTALL_GENERATED _dest _comp)
  if(NOT ARGN)
    message(FATAL_ERROR "idlc_install_generated called without any idl files")
  endif()

  set(_dir "${CMAKE_CURRENT_BINARY_DIR}")
  set(_files)
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    set(_source "${_dir}/${FIL_WE}.c")
    set(_header "${_dir}/${FIL_WE}.h")
    list(APPEND _files "${_source}" "${_header}")
  endforeach()

  install(FILES ${_files}
          DESTINATION "${_dest}"
          COMPONENT ${_comp})
endfunction()

