add_executable(ddhelloworld ddhelloworld.c)
target_link_libraries(ddhelloworld Vortex::DDSC99)

if (CMAKE_TEST_EXAMPLES)
    message(STATUS "Include helloworld example test")
    add_subdirectory(test)
endif()
