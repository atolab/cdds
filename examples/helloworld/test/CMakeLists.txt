find_package(Criterion)


if(CRITERION_FOUND)
    find_package(Vortex REQUIRED)

    add_executable(test1 test1.c)
    target_link_libraries(test1 Vortex::DDSC99 Criterion)

    add_test(helloworld_test1 test1)
endif()
