cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)


find_package(Abstraction QUIET REQUIRED)
find_package(CUnit QUIET)
find_package(Bitrock QUIET)

enable_testing()


include_directories(include)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)

#
# There's no such thing in cmake as a 'post-install' action. So, we have
# to do with a workaround. Cmake makes sure that all added sub directories
# are handled in sequence. This means that the last sub directory is always
# handled last.
#
# The postinstall directory must always be the last directory to make sure
# that the install steps within that directory are executed after all the
# other installation steps mentioned in the other sub-directories.
#
add_subdirectory(postinstall)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix prepended on to install directories." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
set(CPACK_PACKAGE_NAME "vortexdds")
set(CPACK_PACKAGE_VENDOR "PrismTech")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VortexDDS - Data Distribution Service for real-time systems")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0p1")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
message(STATUS "CPACK_PACKAGE_INSTALL_DIRECTORY \"${CPACK_PACKAGE_INSTALL_DIRECTORY}\"")
message(STATUS "CPACK_PACKAGE_VERSION \"${CPACK_PACKAGE_VERSION}\"")
#set(CPACK_PACKAGE_INSTALL_DIRECTORY "CPack Component Example")

set(CPACK_COMPONENTS_ALL dev lib)
set(CPACK_COMPONENTS_GROUPING "IGNORE")
set(CPACK_COMPONENT_LIB_DISPLAY_NAME "VortexDDS library")
set(CPACK_COMPONENT_LIB_DESCRIPTION  "Library used to run programs with VortexDDS")
set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Development files")
set(CPACK_COMPONENT_DEV_DESCRIPTION  "Development files for use with VortexDDS")

SET(CPACK_GENERATOR "DEB")
set(CPACK_DEB_COMPONENT_INSTALL ON)
#set(CPACK_GENERATOR "TGZ")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
#set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_SET_DESTDIR ON)
set(CPACK_PACKAGE_CONTACT "info@prismtech.com")

# This must always be last!
include(CPack)

