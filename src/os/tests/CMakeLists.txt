include(CUnit)

add_cunit_executable(abstraction .)
target_link_libraries(abstraction OSAbstraction)

