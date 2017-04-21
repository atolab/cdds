cmake_minimum_required(VERSION 3.5)

add_executable(ddhelloworld ddhelloworld.c)
target_link_libraries(ddhelloworld vdds)
add_dependencies(ddhelloworld vdds)


add_subdirectory(test)

add_test(helloworld_test1 test/test1)


