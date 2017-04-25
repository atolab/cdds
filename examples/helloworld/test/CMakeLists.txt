cmake_minimum_required(VERSION 3.5)

# create executable for test1
add_executable(test1 test1.c)
target_link_libraries(test1 vdds)
add_dependencies(test1 vdds)
