find_package(Vortex REQUIRED)
find_package(Criterion REQUIRED)

# TEST2
add_executable(criterion_test2 criterion_test2.c)
target_link_libraries(criterion_test2 Vortex::DDSC99 Criterion)
add_test(kernel_criterion_test2 criterion_test2 --verbose)

