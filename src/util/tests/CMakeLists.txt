find_package(Vortex REQUIRED)

include(Criterion)
add_criterion_executable(criterion_util .)
target_link_libraries(criterion_util vddsc)

