# To uniquely identify the origin of every error all source files must be
# assigned a pseudo unique identifier (or module). Because only 32 bits are
# available in the return code (for now) to store the sign bit (1), return
# code (4), line number (8) and file identifier, using a deterministic hash
# will likely lead to collisions. To work around this issue a static map is
# applied, which also ensures that file identifiers are persisted accross
# versions/branches. Of course one could choose to specify the module manually
# with every return, but that is tedious and error prone.

if(SOURCE_FILE_IDS_INCLUDED)
  return()
endif()
set(SOURCE_FILE_IDS_INCLUDED true)


# Verify syntax for all .fileids files and ensure no source file id is used
# more than once.
file(GLOB_RECURSE fils__ LIST_DIRECTORIES false "${CMAKE_SOURCE_DIR}/.fileids")

set(ids__)
foreach(fil__ ${fils__})
  file(READ "${fil__}" lines__)
  string(REGEX REPLACE "\n" ";" lines__ ${lines__})
  foreach(line__ ${lines__})
    if("${line__}" MATCHES "^[ \t]*([0-9]+)[ \t]+.*$")
      set(id__ "${CMAKE_MATCH_1}")
      if(${id__} IN_LIST ids__)
        set(dup__ true)
        message(STATUS "Id ${id__} used more than once")
      else()
        list(APPEND ids__ ${id__})
      endif()
    elseif(NOT "${line__}" MATCHES "^[ \t]*#")
      message(FATAL_ERROR "Syntax error in ${fil__}")
    endif()
  endforeach()
endforeach()

if(dup__)
  message(FATAL_ERROR "Duplicate ids")
endif()


function(JOIN lst glue var)
  string(REPLACE ";" "${glue}" tmp "${${lst}}")
  set(${var} "${tmp}" PARENT_SCOPE)
endfunction()

function(SOURCE_FILE_ID src var) # private
  # .fileids files may reside in subdirectories to keep them together with the
  # files they assign an identifier to, much like .gitignore files
  set(dir "${CMAKE_SOURCE_DIR}")
  set(parts "${src}")
  string(REGEX REPLACE "[/\\]+" ";" parts "${parts}")
  while(parts)
    set(map "${dir}/.fileids")
    join(parts "/" fil)
    list(APPEND maps "${map}:${fil}")
    list(GET parts 0 part)
    list(REMOVE_AT parts 0)
    set(dir "${dir}/${part}")
  endwhile()

  set(id)
  foreach(entry ${maps})
    string(REPLACE ":" ";" entry "${entry}")
    list(GET entry 0 map)
    list(GET entry 1 fil)
    if(EXISTS "${map}")
      file(READ "${map}" contents)
      string(REGEX REPLACE "\n" ";" lines "${contents}")

      foreach(line ${lines})
        if("${line}" MATCHES "^[ \t]*([0-9]+)[ \t]+(.*)$")
          set(id "${CMAKE_MATCH_1}")
          string(STRIP "${CMAKE_MATCH_2}" expr)
          if("${fil}" STREQUAL "${expr}")
            set(${var} ${id} PARENT_SCOPE)
            return()
          endif()
        elseif(NOT "${line}" MATCHES "^[ \t]*#")
          message(FATAL_ERROR "Syntax error in ${map}")
        endif()
      endforeach()
    endif()
  endforeach()
endfunction()

# Source file properties are visible only to targets added in the same
# directory (CMakeLists.txt).
# https://cmake.org/cmake/help/latest/command/set_source_files_properties.html
function(SET_TARGET_SOURCE_FILE_IDS tgt)
  get_target_property(external ${tgt} IMPORTED)
  get_target_property(alias ${tgt} ALIASED_TARGET)
  string(LENGTH "${CMAKE_SOURCE_DIR}" len)
  math(EXPR len "${len} + 1") # strip slash following source dir too

  if((NOT external) AND (NOT alias))
    get_target_property(srcs ${tgt} SOURCES)
    get_target_property(src_dir ${tgt} SOURCE_DIR)
    foreach(src ${srcs})
      set(id)
      if(IS_ABSOLUTE "${src}")
        set(fil "${src}")
      else()
        set(fil "${src_dir}/${src}")
      endif()

      get_filename_component(fil "${fil}" ABSOLUTE)

      string(FIND "${src}" "${CMAKE_SOURCE_DIR}" pos)
      if(${pos} EQUAL 0)
        string(SUBSTRING "${fil}" ${len} -1 rel)
        source_file_id("${rel}" id)
      endif()

      if(id)
        if(("${source_file_id_${id}}" STREQUAL "") OR
           ("${source_file_id_${id}}" STREQUAL "${src}"))
          set("source_file_id_${id}" "${rel}" CACHE INTERNAL "")
          set_source_files_properties(
            "${src}" PROPERTIES COMPILE_DEFINITIONS SOURCE_FILE_ID=${id})
        else()
          message(FATAL_ERROR "Same id for ${rel} and ${source_file_id_${id}}")
        endif()
      else()
        message(FATAL_ERROR "No source file id for ${rel}")
      endif()
    endforeach()
  endif()
endfunction()

