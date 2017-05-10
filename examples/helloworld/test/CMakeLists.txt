find_package(Criterion)

# Criterion tests
if(CRITERION_FOUND)
    find_package(Vortex REQUIRED)

    add_executable(criterion_test1 criterion/criterion_test1.c)
    target_link_libraries(criterion_test1 Vortex::DDSC99 Criterion)

    add_test(helloworld_criterion_test1 criterion_test1)
endif()


# CUnit tests
find_package(CUnit)

if(CUNIT_FOUND)
    find_package(Vortex REQUIRED)

    add_executable(cunit_test1 cunit/cunit_test1.c)
    target_link_libraries(cunit_test1 Vortex::DDSC99 cunitrunner)

    add_test(helloworld_cunit_test1 cunit_test1)
endif()

