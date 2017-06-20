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
        set(TEST_NAME "util_${NAME}")
        add_executable(${NAME} ${ARGN})
        target_link_libraries(${NAME} Vortex::DDSC ${SUITE})

        if(${SUITE} STREQUAL cunitrunner)
          add_test(${TEST_NAME} ${NAME} -a -r ${NAME})
        else()
          add_test(${TEST_NAME} ${NAME} --xml=${NAME}-Junit.xml ${CRITERION_OUTPUT_MODE})
        endif()
    endif()
endfunction()


#################################################################################
# The tests
#################################################################################
add_dds_test(handleserver               Criterion      handleserver.c)

