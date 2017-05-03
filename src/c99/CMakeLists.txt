option(BUILD_UNITTEST_C99 "Build unit-tests for C99" NO)
option(BUILD_DOCS_C99 "Generate API documentation for C99" NO)

add_library(c99 EXCLUDE_FROM_ALL "")
target_sources(c99 PRIVATE dds_listener.c)
target_include_directories(c99 PUBLIC include)
target_include_directories(c99 PUBLIC ${CMAKE_SOURCE_DIR}/include/os)
target_link_libraries(c99 Abstraction)

if(BUILD_DOCS_C99)
    # todo sphinx+breathe to replace doxygen
    find_package(Doxygen)
    if(Doxygen_FOUND)
        configure_file(${CMAKE_CURRENT_LIST_DIR}/c99.Doxyfile.in
            ${CMAKE_CURRENT_BINARY_DIR}/c99.Doxyfile @ONLY)
        add_custom_target(docs
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/c99.Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Executing Doxygen for C99 API docs generation"
            VERBATIM)
    else()
        message(FATAL_ERROR "Unable to proceed with C99 API docs generation")
    endif()
endif()

if(BUILD_UNITTEST_C99)
    add_subdirectory(tests)
endif()
