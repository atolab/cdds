find_package(Vortex::DDSC99 REQUIRED)

add_executable(ddhelloworld ddhelloworld.c)
target_link_libraries(ddhelloworld Vortex::DDSC99)

add_subdirectory(test)
