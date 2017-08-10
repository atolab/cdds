if(NOT IDLC_JAR)
    set(IDLC_JAR "${CMAKE_CURRENT_LIST_DIR}/idlc/idlc-jar-with-dependencies.jar")
endif()

set(LINE_ENDINGS "UNIX")
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(EXTENSION ".bat")
    set(LINE_ENDINGS "WIN32")
endif()

set(IDLC_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "")
set(IDLC "dds_idlc${EXTENSION}" CACHE STRING "")
mark_as_advanced(IDLC_DIR IDLC)

set(IDLC_SCRIPT_IN "${CMAKE_CURRENT_LIST_DIR}/idlc/dds_idlc${EXTENSION}.in")

configure_file(
    "${IDLC_SCRIPT_IN}" "${IDLC}"
    @ONLY
    NEWLINE_STYLE ${LINE_ENDINGS})

# FIXME: C++ IDL compiler depends idlpp. Leave it disabled for now.
#configure_file(
#    "cmake/dds_idlcpp${EXTENSION}.in"
#    "dds_idlcpp${EXTENSION}"
#    @ONLY
#    NEWLINE_STYLE ${LINE_ENDINGS})

if(NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows"))
    execute_process(COMMAND chmod +x "${IDLC_DIR}/${IDLC}")
endif()

add_custom_target(idlc ALL DEPENDS "${IDLC_JAR}")

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

