if(CUNIT_FOUND)
    # create executable for test1
    add_executable(test1 test1.c)
    target_link_libraries(test1 Vortex::DDSC99 cunit)

    add_test(helloworld_test1 test1)
endif()
