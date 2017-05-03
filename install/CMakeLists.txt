cmake_minimum_required(VERSION 3.5)

include(Version)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix prepended on to install directories." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Packages meta info
set(CPACK_PACKAGE_NAME "vortexdds") # This will also be part of the package file name
set(CPACK_PACKAGE_VENDOR "PrismTech")
set(CPACK_PACKAGE_CONTACT "info@prismtech.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vortex DDS - Data Distribution Service for real-time systems")
set(CPACK_PACKAGE_VERSION_MAJOR "${VORTEXDDS_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VORTEXDDS_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VORTEXDDS_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# Components meta info
set(CPACK_COMPONENT_LIB_DISPLAY_NAME "VortexDDS library")
set(CPACK_COMPONENT_LIB_DESCRIPTION  "Library used to run programs with Vortex DDS")
set(CPACK_COMPONENT_DEV_DISPLAY_NAME "VortexDDS development")
set(CPACK_COMPONENT_DEV_DESCRIPTION  "Development files for use with Vortex DDS")

# You should have the same lib package when installing the dev package
# TODO: figure out how to do this for WIX (if at all possible)
set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "vortexdds-lib (= ${CPACK_PACKAGE_VERSION})")
set(CPACK_RPM_DEV_PACKAGE_REQUIRES   "vortexdds-lib = ${CPACK_PACKAGE_VERSION}")

# Create different packages (dev, lib, etc) for the following targets
#set(CPACK_GENERATOR DEB)
set(CPACK_COMPONENTS_ALL dev lib)
set(CPACK_COMPONENTS_GROUPING "IGNORE")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_WIX_COMPONENT_INSTALL ON)

# TODO: figure out what these actually do.
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_SET_DESTDIR ON)
set(CPACK_PACKAGE_RELOCATABLE OFF)

# This must always be last!
include(CPack)

