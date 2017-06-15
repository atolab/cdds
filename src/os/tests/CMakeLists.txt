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
#       TYPE    The type of test 'cunitrunner'
#       ARGN    The c file(s) that contains the test.
#
function(add_dds_test NAME SUITE)
    if ((${SUITE} STREQUAL cunitrunner))
        set(TEST_NAME "abstraction_${NAME}")
        add_executable(${NAME} ${ARGN})
        target_link_libraries(${NAME} PRIVATE OSAPI ${SUITE})

	if(SUITE STREQUAL cunitrunner)
          add_test(${TEST_NAME} ${NAME} -a -r ${NAME})
        else()
          add_test(${TEST_NAME} ${NAME})
        endif()
    endif()
endfunction()

#################################################################################
# The tests
#################################################################################
add_dds_test(atomics                  cunitrunner          atomics.c)
add_dds_test(error_no                 cunitrunner          error_no.c)
add_dds_test(mutex                    cunitrunner          mutex.c)
add_dds_test(rwlock                   cunitrunner          rwlock.c)
add_dds_test(stdlib                   cunitrunner          stdlib.c)
add_dds_test(strtoll                  cunitrunner          strtoll.c)
add_dds_test(thread                   cunitrunner          thread.c)
