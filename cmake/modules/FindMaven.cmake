if(DEFINED ENV{M2})
  list(APPEND _mvn_hints "$ENV{M2}")
endif()

if(DEFINED ENV{M2_HOME})
  list(APPEND _mvn_hints "$ENV{M2_HOME}/bin")
endif()

# Maven documentation mentions intalling maven under C:\Program Files on
# Windows and under /opt on *NIX platforms
if(WIN32)
  if("$ENV{ProgramFiles}")
    list(APPEND _dirs "$ENV{ProgramFiles}")
  endif()

  if("$ENV{ProgramFiles(x86)}")
    list(APPEND _dirs "$ENV{ProgramFiles(x86)}")
  endif()
else()
  list(APPEND _dirs "/opt")
endif()

foreach(_dir ${_dirs})
  file(GLOB _mvn_dirs "${_dir}/apache-maven-*")
  foreach(_mvn_dir ${_mvn_dirs})
    if((IS_DIRECTORY "${_mvn_dir}") AND (IS_DIRECTORY "${_mvn_dir}/bin"))
      list(APPEND _mvn_paths "${_mvn_dir}/bin")
    endif()
  endforeach()
endforeach()

find_program(Maven_EXECUTABLE
  NAMES mvn
  HINTS ${_mvn_hints}
  PATHS ${_mvn_paths})

if(Maven_EXECUTABLE)
  execute_process(COMMAND ${Maven_EXECUTABLE} -version
    RESULT_VARIABLE result
    OUTPUT_VARIABLE var
    ERROR_VARIABLE var
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  if(NOT res)
    if(var MATCHES "Apache Maven ([0-9]+)\\.([0-9]+)\\.([0-9]+)")
      set(Maven_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Maven
  FOUND_VAR Maven_FOUND
  REQUIRED_VARS Maven_EXECUTABLE
  VERSION_VAR Maven_VERSION)

mark_as_advanced(Maven_FOUND Maven_EXECUTABLE Maven_VERSION)

