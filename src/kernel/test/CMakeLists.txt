# Criterion tests
find_package(Criterion)
if(CRITERION_FOUND)
    add_subdirectory(criterion)
endif()

# CUnit tests
find_package(CUnit)
if(CUNIT_FOUND)
    add_subdirectory(cunit)
endif()

