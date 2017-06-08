find_package(Vortex REQUIRED)

include(CUnit)
include(Criterion)

add_criterion_executable(criterion_vddsc .)
target_link_libraries(criterion_vddsc Vortex::DDSC99)

add_cunit_executable(cunit_vddsc .)
target_link_libraries(cunit_vddsc Vortex::DDSC99)

