if(WARNINGS_INCLUDED)
  return()
  endif()
  set(WARNINGS_INCLUDED true)

  add_compile_options(
#   $<$<C_COMPILER_ID:MSVC>:"/W4">
    $<$<C_COMPILER_ID:Clang>:-Wall>
#   $<$<C_COMPILER_ID:Clang>:-Weverything> # If you're ready for the madhouse or want to become ready
    $<$<C_COMPILER_ID:GNU>:-Wall>
    $<$<C_COMPILER_ID:GNU>:-Wno-unknown-pragmas>

  )
