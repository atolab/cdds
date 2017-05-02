find_package(CUnit)

if(CUNIT_FOUND)
    find_package(Vortex REQUIRED)

    add_executable(test1 test1.c)
    target_link_libraries(test1 Vortex CUnit)

    add_test(helloworld_test1 test1)
endif()
