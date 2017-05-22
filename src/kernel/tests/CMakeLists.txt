find_package(Vortex REQUIRED)
find_package(Criterion)
find_package(CUnit)

#
# Function
#       add_dds_test
#
# Description
#       This function will call the various cmake functions with
#       the proper values and libraries so that the given test
#       is added to the framework.
#
# Parameters
#       NAME    The executable name of the test
#       TYPE    The type of test 'Criterion' or 'CUnit'
#       ARGN    The c file(s) that contains the test.
#
function(add_dds_test NAME SUITE)
    if (((${SUITE} STREQUAL Criterion) AND (CRITERION_FOUND)) OR
        ((${SUITE} STREQUAL CUnit    ) AND (CUNIT_FOUND    )) )

        set(TEST_NAME "c99_${NAME}")
        add_executable(${NAME} ${ARGN})
        target_link_libraries(${NAME} Vortex::DDSC99 ${SUITE})
        add_test(${TEST_NAME} ${NAME})
    endif()
endfunction()


#################################################################################
# The tests
#################################################################################
add_dds_test(basic                    CUnit          basic.c)
add_dds_test(entity_api               Criterion      entity_api.c)
add_dds_test(listener                 CUnit          listener.c)
add_dds_test(participant_creation     Criterion      participant_creation.c)

