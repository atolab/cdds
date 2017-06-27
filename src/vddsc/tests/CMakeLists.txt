find_package(Vortex REQUIRED)

include(Criterion)

add_criterion_executable(criterion_vddsc .)
target_link_libraries(criterion_vddsc vddsc)

