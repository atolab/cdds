find_package(Vortex REQUIRED)
find_package(Criterion REQUIRED)

include(CUnit)

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
#       TYPE    The type of test 'Criterion' or 'cunitrunner'
#       ARGN    The c file(s) that contains the test.
#
function(add_dds_test NAME SUITE)
    if (((${SUITE} STREQUAL Criterion) AND (CRITERION_FOUND)) OR
        ((${SUITE} STREQUAL cunitrunner)))
        set(TEST_NAME "c99_${NAME}")
        add_executable(${NAME} ${ARGN})
        target_link_libraries(${NAME} Vortex::DDSC99 ${SUITE})

        if(${SUITE} STREQUAL cunitrunner)
          add_test(${TEST_NAME} ${NAME} -a -r ${NAME})
        else()
          add_test(${TEST_NAME} ${NAME})
        endif()
    endif()
endfunction()


#################################################################################
# The tests
#################################################################################
add_dds_test(entity_api               Criterion      entity_api.c)
add_dds_test(participant_creation     Criterion      participant_creation.c)
add_dds_test(publisher                Criterion      publisher.c)

add_cunit_executable(basic basic.c)
target_link_libraries(basic Vortex::DDSC99)
add_cunit_executable(listener listener.c)
target_link_libraries(listener Vortex::DDSC99)

