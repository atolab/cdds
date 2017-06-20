find_package(Vortex REQUIRED)
find_package(Criterion REQUIRED)

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
        set(TEST_NAME "vddsc_${NAME}")
        add_executable(${NAME} ${ARGN})
        target_link_libraries(${NAME} Vortex::DDSC ${SUITE})

        if(${SUITE} STREQUAL cunitrunner)
          add_test(${TEST_NAME} ${NAME} -a -r ${NAME})
        else()
          add_test(${TEST_NAME} ${NAME} --xml=${NAME}-Junit.xml)
        endif()
    endif()
endfunction()


#################################################################################
# The tests
#################################################################################
add_dds_test(basic                    Criterion      basic.c)
add_dds_test(entity_api               Criterion      entity_api.c)
add_dds_test(listener                 Criterion      listener.c)
add_dds_test(participant_creation     Criterion      participant_creation.c)
add_dds_test(publisher                Criterion      publisher.c)
add_dds_test(subscriber               Criterion      subscriber.c)
add_dds_test(topic                    Criterion      topic.c RoundTrip.c)
add_dds_test(reader                   Criterion      reader.c RoundTrip.c)