cmake_minimum_required(VERSION 3.5)

# Set module path before defining project for platform files to work
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

set(CMAKE_C_STANDARD 99)

include(AnalyzeBuild)
# Include Coverage before CTest so that COVERAGE_COMMAND can be modified
# in the Coverage module should that ever be necessary.
include(Coverage)
set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --trace-children=yes --xml=yes --xml-file=TestResultValgrind.xml --tool=memcheck --show-reachable=yes --leak-resolution=high")
include (CTest)
enable_testing()

# todo sphinx+breathe to replace doxygen
find_package(Doxygen)
if(Doxygen_FOUND)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Executing Doxygen for document generation"
        VERBATIM)
endif()

include_directories(include)
#add_subdirectory(idlc)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)

include(Packaging) # Pulls in CPack

