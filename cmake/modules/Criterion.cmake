find_package(Criterion REQUIRED)

include(Glob)

set(CRITERION_DIR "${CMAKE_CURRENT_LIST_DIR}/Criterion")

function(add_criterion_executable target)
  set(s "[ \t\r\n]") # space
  set(w "[0-9a-zA-Z_]") # word
  set(param "${s}*(${w}+)${s}*")
  set(pattern "(^|[^0-9a-zA-Z_])Test${s}*\\(${param},${param}(,${param})*\\)")

  glob(filenames "c" ${ARGN})

  foreach(filename ${filenames})
    file(READ "${filename}" contents)
    string(REGEX MATCHALL "${pattern}" captures "${contents}")

    list(APPEND sources "${filename}")
    list(LENGTH captures length)
    if(length)
      foreach(capture ${captures})
        string(REGEX REPLACE "${pattern}" "\\2" suite "${capture}")
        string(REGEX REPLACE "${pattern}" "\\3" test "${capture}")

        list(APPEND tests "${suite}:${test}")
      endforeach()
    endif()
  endforeach()

  set(root "${CRITERION_DIR}")
  add_executable(${target} "${root}/src/runner.c" ${sources})
  target_link_libraries(${target} Criterion)

  foreach(entry ${tests})
    string(REPLACE ":" ";" entry ${entry})
    list(GET entry 0 suite)
    list(GET entry 1 test)

    add_test(
      NAME "Criterion_${suite}_${test}"
      COMMAND ${target} --suite ${suite} --test ${test} --cunit=${suite}-${test} --quiet)
  endforeach()
endfunction()

