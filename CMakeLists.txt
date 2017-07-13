cmake_minimum_required(VERSION 3.5)

# Set module path before defining project for platform files to work
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

set(CMAKE_C_STANDARD 99)
if(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
  add_definitions(-std=c99)
endif()

# Build all executables and libraries into the top-level /bin /lib etc.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include(AnalyzeBuild)
# Include Coverage before CTest so that COVERAGE_COMMAND can be modified
# in the Coverage module should that ever be necessary.
include(Coverage)
set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --trace-children=yes --child-silent-after-fork=yes --xml=yes --xml-file=TestResultValgrind_%p.xml --tool=memcheck --show-reachable=yes --leak-resolution=high")
set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/valgrind.supp" CACHE FILEPATH "suppression file")

# By default building the testing tree is enabled by including CTest, but
# since not everybody has CUnit and/or Criterion installed, and because it is
# not strictly required to build the product itself, switch to off by default.
option(BUILD_TESTING "Build the testing tree." OFF)
include(CTest)

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

add_subdirectory(idlc)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)

include(Packaging) # Pulls in CPack

# set installation dirs
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for library files")
set(INSTALL_CONFIG_DIR etc CACHE PATH "Installation directory for config files")

# Include helpers functions for creating config files that can be
# included by other projects to find and use a package.
include(CMakePackageConfigHelpers)
configure_package_config_file(VortexDDSConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/VortexDDSConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/VortexDDS/cmake
  PATH_VARS INSTALL_INCLUDE_DIR INSTALL_LIB_DIR INSTALL_CONFIG_DIR)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/VortexDDSVersion.cmake
  VERSION 1.2.3
  COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VortexDDSConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/VortexDDSVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/Foo/cmake )

