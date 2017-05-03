add_executable(UT_C99_Listener "")
target_link_libraries(UT_C99_Listener c99 cunit)

target_sources(UT_C99_Listener PRIVATE "listener.c")

add_test(C99_Listener UT_C99_Listener)
