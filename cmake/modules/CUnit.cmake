find_package(CUnit REQUIRED)

set(CUNIT_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(s "[ \t\r\n]") # space
set(w "[0-9a-zA-Z_]") # word
set(param "${s}*(${w}+)${s}*")
set(func "CUnit_${w}+${s}*\\(${param}(,${param}(,${param})?)?\\)")

function(add_cunit_executable target)
  string(RANDOM basename) # Don't use target as basename

  # Find all c source files
  foreach(filename ${ARGN})
    if(NOT IS_ABSOLUTE "${filename}")
      set(filename "${CMAKE_CURRENT_SOURCE_DIR}/${filename}")
    endif()

    if(IS_DIRECTORY "${filename}")
      file(GLOB_RECURSE filenames "${filename}/*.c")
    elseif(EXISTS "${filename}")
      set(filenames "${filename}")
    else()
      set(filenames "")
    endif()

    list(FILTER filenames INCLUDE REGEX "\.c$")
    list(APPEND sources ${filenames})
  endforeach()

  set(suites "")
  set(suites_w_init "")
  set(suites_w_deinit "")
  mark_as_advanced(suites suites_w_init suites_w_deinit)
  set(tests "")
  mark_as_advanced(tests)

  foreach(source ${sources})
    file(READ "${source}" contents)

    string(REGEX MATCHALL "${func}" captures "${contents}")
    foreach(capture ${captures})
      string(REGEX REPLACE "${func}" "\\1" suite "${capture}")

      if("${capture}" MATCHES "CUnit_Suite_Initialize")
        list(APPEND suites ${suite})
        list(APPEND suites_w_init ${suite})
      elseif("${capture}" MATCHES "CUnit_Suite_Cleanup")
        list(APPEND suites ${suite})
        list(APPEND suites_w_deinit ${suite})
      elseif("${capture}" MATCHES "CUnit_Test")
        list(APPEND suites ${suite})

        # Specifying a test name is mandatory
        if("${capture}" MATCHES ",")
          string(REGEX REPLACE "${func}" "\\3" test "${capture}")
        else()
          message(FATAL_ERROR "Unsupported CUnit_Test signature in ${source}")
        endif()

        # Specifying if a test is enabled is optional
        set(enable "true")

        if("${capture}" MATCHES ",${param},")
          string(REGEX REPLACE "${func}" "\\5" enable "${capture}")
        endif()

        if((NOT "${enable}" STREQUAL "true") AND
           (NOT "${enable}" STREQUAL "false"))
          message(FATAL_ERROR "Unsupport CUnit_Test signature in ${source}")
        endif()

        list(APPEND tests "${suite}:${test}:${enable}")
      else()
        message(FATAL_ERROR "Unsupported CUnit signature in ${source}")
      endif()
    endforeach()
  endforeach()

  # Test suite signature can be declared only after everything is parsed
  set(lf "\n")
  set(declf "")
  set(deflf "")

  list(REMOVE_DUPLICATES suites)
  list(SORT suites)
  foreach(suite ${suites})
    set(init "NULL")
    set(deinit "NULL")
    if(${suite} IN_LIST suites_w_init)
      set(init "CUnit_Suite_Initialize__(${suite})")
      set(decls "${decls}${declf}CUnit_Suite_Initialize_Decl__(${suite});")
      set(declf "${lf}")
    endif()
    if(${suite} IN_LIST suites_w_deinit)
      set(deinit "CUnit_Suite_Cleanup__(${suite})")
      set(decls "${decls}${declf}CUnit_Suite_Cleanup_Decl__(${suite});")
      set(declf "${lf}")
    endif()

    set(defs "${defs}${deflf}CUnit_Suite__(${suite}, ${init}, ${deinit});")
    set(deflf "${lf}")
  endforeach()

  list(REMOVE_DUPLICATES tests)
  list(SORT tests)
  foreach(entry ${tests})
    string(REPLACE ":" ";" entry ${entry})
    list(GET entry 0 suite)
    list(GET entry 1 test)
    list(GET entry 2 enable)

    set(decls "${decls}${declf}CUnit_Test_Decl__(${suite}, ${test});")
    set(declf "${lf}")
    set(defs "${defs}${deflf}CUnit_Test__(${suite}, ${test}, ${enable});")
    set(deflf "${lf}")

    add_test(
      NAME "${suite}_${test}"
      COMMAND ${target} -a -j -r "${suite}-${test}" -s ${suite} -t ${test})
  endforeach()

  set(root "${CUNIT_MODULE_DIR}/CUnit")
  set(CUnit_Decls "${decls}")
  set(CUnit_Defs "${defs}")

  configure_file("${root}/src/main.c.in" "${basename}.c" @ONLY)
  add_executable(${target} "${basename}.c" "${root}/src/runner.c" ${sources})
  target_link_libraries(${target} cunit)
  target_include_directories(${target} PRIVATE "${root}/include")
endfunction()

