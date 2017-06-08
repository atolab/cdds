find_package(Criterion REQUIRED)

include(Glob)

function(add_criterion_executable target)
  set(s "[ \t\r\n]") # space
  set(w "[0-9a-zA-Z_]") # word
  set(param "${s}*(${w}+)${s}*")
  set(pattern "(^|[^0-9a-zA-Z_])Test${s}*\\(${param},${param}(,${param})*\\)")

  glob(filenames "c" ${ARGN})

  foreach(filename ${filenames})
    file(READ "${filename}" contents)
    string(REGEX MATCHALL "${pattern}" captures "${contents}")

    list(LENGTH captures length)
    if(length)
      # Ensure only files that actually implement Criterion tests are added to
      # the list of source files.
      list(APPEND sources "${filename}")

      foreach(capture ${captures})
        string(REGEX REPLACE "${pattern}" "\\2" suite "${capture}")
        string(REGEX REPLACE "${pattern}" "\\3" test "${capture}")
        list(APPEND tests "${suite}:${test}")
      endforeach()
    endif()
  endforeach()

  add_executable(${target} ${sources})
  target_link_libraries(${target} Criterion)

  foreach(entry ${tests})
    string(REPLACE ":" ";" entry ${entry})
    list(GET entry 0 suite)
    list(GET entry 1 test)

    add_test(
      NAME "Criterion_${suite}_${test}"
      COMMAND ${target} --filter "${suite}/${test}")
  endforeach()
endfunction()

