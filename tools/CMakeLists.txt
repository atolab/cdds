option(BUILD_TOOLS "Enable building of tools." ON)

if(BUILD_TOOLS)
  set(CMAKE_INSTALL_TOOLSDIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/tools")

  add_subdirectory(pubsub)
  add_subdirectory(config)

  # VxWorks build machines use OpenJDK 8, which lack jfxrt.jar. Do not build
  # launcher on that platform.
  #
  # TODO: Instead of making inclusion dependent on platform name, the condition
  #       should instead be on the jdk vendor (Oracle JDK, not OpenJDK). Find a
  #       way to make CMake aware of jdk vendor.
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
    add_subdirectory(launcher)
  endif()
endif()
