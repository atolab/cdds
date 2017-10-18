function(glob variable extension)
  set(dirname "${CMAKE_CURRENT_SOURCE_DIR}")

  foreach(filename ${ARGN})
    unset(filenames)

    if((NOT IS_ABSOLUTE "${filename}") AND
       (EXISTS "${dirname}/${filename}"))
      set(filename "${dirname}/${filename}")
    endif()

    if(IS_DIRECTORY "${filename}")
      file(GLOB_RECURSE filenames "${filename}/*.${extension}")
    elseif(EXISTS "${filename}")
      if("${filename}" MATCHES "\.${extension}$")
        set(filenames "${filename}")
      endif()
    else()
      message(FATAL_ERROR "File ${filename} does not exist")
    endif()

    list(APPEND files ${filenames})
  endforeach()

  set(${variable} "${files}" PARENT_SCOPE)
endfunction()

