find_package(CUnit REQUIRED)
find_package(Vortex REQUIRED)

#TEST2
add_executable(cunit_test2 cunit_test2.c)
target_link_libraries(cunit_test2 Vortex::DDSC99 CUnit)
add_test(kernel_cunit_test2 cunit_test2)

