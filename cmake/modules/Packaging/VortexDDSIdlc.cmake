
# Find the IDL compiler. If it is not in a default location, try
# finding it relative to this file where it most likely resides.
find_program(IDLC_PATH dds_idlc PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../bin)
if(NOT IDLC_PATH)
  message(WARNING "Could not find 'dds_idlc' to generate source files from idl files. Please provide its location in CMAKE_PREFIX_PATH.")
endif()

#
# This function will create a library target that consists of
# the of the source files compilation, generated from the given
# idl file(s).
#
# This library target can then be used in other targets.
#
# Example usage:
#   idlc_generate(HelloWorldData_lib "HelloWorldData.idl")
#   add_executable(HelloworldPublisher publisher.c)
#   target_link_libraries(HelloworldPublisher HelloWorldData_lib VortexDDS::vddsc)
#
function(IDLC_GENERATE _target)
  if(NOT IDLC_PATH)
    message(FATAL_ERROR "Can not generate libraries from idl files without 'dds_idlc'.")
  endif()

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
      COMMAND  "${IDLC_PATH}"
      ARGS     "${ABS_FIL}"
      COMMENT  "Running idlc on ${FIL}"
      VERBATIM)
  endforeach()

  set_source_files_properties(
    ${_sources} ${_headers} PROPERTIES GENERATED TRUE)
  add_library(${_target} INTERFACE)
  target_sources(${_target} INTERFACE ${_sources})
  target_include_directories(${_target} INTERFACE "${_dir}")
endfunction()

