if(PACKAGING_INCLUDED)
  return()
endif()
set(PACKAGING_INCLUDED true)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(PACKAGING_MODULE_DIR "${CMAKE_SOURCE_DIR}/cmake/modules/Packaging")
set(CMAKE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}")

# Generates <Package>Config.cmake.
configure_package_config_file(
  "${PACKAGING_MODULE_DIR}/PackageConfig.cmake.in"
  "${CMAKE_PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")

# Generates <Package>Version.cmake.
write_basic_package_version_file(
  "${CMAKE_PROJECT_NAME}Version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  FILES "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
        "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Version.cmake"
  DESTINATION "${CMAKE_INSTALL_CMAKEDIR}" COMPONENT dev)

if(VDDSC_SHARED AND ((NOT DEFINED BUILD_SHARED_LIBS) OR BUILD_SHARED_LIBS))
  # Generates <Package>Targets.cmake file included by <Package>Config.cmake.
  # The files are placed in CMakeFiles/Export in the build tree.
  install(
    EXPORT "${CMAKE_PROJECT_NAME}"
    FILE "${CMAKE_PROJECT_NAME}Targets.cmake"
    NAMESPACE "${CMAKE_PROJECT_NAME}::"
    DESTINATION "${CMAKE_INSTALL_CMAKEDIR}" COMPONENT dev)
endif()


set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION_TWEAK ${PROJECT_VERSION_TWEAK})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "PrismTech")
set(CPACK_PACKAGE_CONTACT "info@prismtech.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Leading OMG DDS implementation")
set(CPACK_PACKAGE_ICON "${PACKAGING_MODULE_DIR}/vortex.ico")
set(CPACK_RESOURCE_FILE_LICENSE "${PACKAGING_MODULE_DIR}/license.txt")

# Packages could be generated on alien systems. e.g. Debian packages could be
# created on Red Hat Enterprise Linux, but since packages also need to be
# verified on the target platform, please refrain from doing so. Another
# reason for building installer packages on the target platform is to ensure
# the binaries are linked to the libc version shipped with that platform. To
# support "generic" Linux distributions, eventually compressed tarballs will
# be shipped.
#
# NOTE: Settings for different platforms are in separate control branches.
#       Although that does not make sense from a technical point-of-view, it
#       does help to clearify which settings are required for a platform.

set(CPACK_COMPONENTS_ALL dev lib)
set(CPACK_COMPONENT_LIB_DISPLAY_NAME "Vortex DDS library")
set(CPACK_COMPONENT_LIB_DESCRIPTION  "Library used to run programs with Vortex DDS")
set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Vortex DDS development")
set(CPACK_COMPONENT_DEV_DESCRIPTION  "Development files for use with Vortex DDS")

if(WIN32 AND NOT UNIX)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(__arch "win64")
  else()
    set(__arch "win32")
  endif()
  mark_as_advanced(__arch)

  set(CPACK_GENERATOR "WIX;${CPACK_GENERATOR}" CACHE STRING "List of package generators")

  set(CPACK_PACKAGE_FILE_NAME "VortexDDS-${CPACK_PACKAGE_VERSION}-${__arch}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_VENDOR}/DDS")

  set(CPACK_WIX_COMPONENT_INSTALL ON)
  set(CPACK_WIX_ROOT_FEATURE_TITLE "Vortex DDS")
  set(CPACK_WIX_PRODUCT_ICON "${PACKAGING_MODULE_DIR}/vortex.ico")
  # Bitmap (.bmp) of size 493x58px
  set(CPACK_WIX_UI_BANNER "${PACKAGING_MODULE_DIR}/banner.bmp")
  # Bitmap (.bmp) of size 493x312px
  set(CPACK_WIX_UI_DIALOG "${PACKAGING_MODULE_DIR}/dialog.bmp")
  set(CPACK_WIX_PROGRAM_MENU_FOLDER "${CPACK_PACKAGE_NAME}")
  set(CPACK_WIX_PROPERTY_ARPHELPLINK "http://www.prismtech.com/support")
  set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "http://www.prismtech.com/")
  set(CPACK_WIX_PROPERTY_ARPURLUPDATEINFO "http://www.prismtech.com/vortex/software-downloads")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix prepended on to install directories." FORCE)
  endif()

  set(CPACK_COMPONENTS_GROUPING "IGNORE")

  # FIXME: Requiring lsb_release to be installed may be a viable option.

  if(EXISTS "/etc/redhat-release")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(__arch "x86_64")
    else()
      set(__arch "i686")
    endif()

    set(CPACK_GENERATOR "RPM;${CPACK_GENERATOR}" CACHE STRING "List of package generators")

    set(CPACK_RPM_COMPONENT_INSTALL ON)
    # FIXME: The package file name must be updated to include the distribution.
    #        See Fedora and Red Hat packaging guidelines for details.
    set(CPACK_RPM_LIB_PACKAGE_NAME "vortex-dds")
    set(CPACK_RPM_LIB_FILE_NAME "${CPACK_RPM_LIB_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${__arch}.rpm")
    set(CPACK_RPM_DEV_PACKAGE_NAME "vortex-dds-devel")
    set(CPACK_RPM_DEV_FILE_NAME "${CPACK_RPM_DEV_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${__arch}.rpm")
    set(CPACK_RPM_DEV_PACKAGE_REQUIRES "${CPACK_RPM_LIB_PACKAGE_NAME} = ${CPACK_PACKAGE_VERSION}")
  elseif(EXISTS "/etc/debian_version")
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(__arch "amd64")
    else()
      set(__arch "i386")
    endif()

    set(CPACK_GENERATOR "DEB;${CPACK_GENERATOR}" CACHE STRING "List of package generators")

    set(CPACK_DEBIAN_LIB_PACKAGE_NAME "vortex-dds")
    set(CPACK_DEBIAN_LIB_FILE_NAME "${CPACK_DEBIAN_LIB_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${__arch}.deb")
    set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIB_PACKAGE_NAME} (= ${CPACK_PACKAGE_VERSION})")
    set(CPACK_DEBIAN_DEV_PACKAGE_NAME "vortex-dds-dev")
    set(CPACK_DEBIAN_DEV_FILE_NAME "${CPACK_DEBIAN_DEV_PACKAGE_NAME}-dev_${CPACK_PACKAGE_VERSION}_${__arch}.deb")
  else()
    # Generic tgz package
    set(CPACK_GENERATOR "TGZ;${CPACK_GENERATOR}" CACHE STRING "List of package generators")
  endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "VxWorks")
  # FIXME: Support for VxWorks packages must still be implemented (probably
  #        just a compressed tarball)
  message(STATUS "Packaging for VxWorks is unsupported")
endif()

# This must always be last!
include(CPack)

