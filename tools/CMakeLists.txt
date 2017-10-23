set(CMAKE_INSTALL_TOOLSDIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/tools")

function(INSTALL_LAUNCH_SCRIPT _scriptname)
  set(LINE_ENDINGS "UNIX")
  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
      set(EXTENSION ".bat")
      set(LINE_ENDINGS "WIN32")
  endif()

  #set(SCRIPT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  set(SCRIPT "${_scriptname}${EXTENSION}")
  #mark_as_advanced(IDLC_DIR IDLC)

  set(SCRIPT_IN "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}.in")

  configure_file(
      "${SCRIPT_IN}" "${SCRIPT}"
      @ONLY
      NEWLINE_STYLE ${LINE_ENDINGS})

  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT}"
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    COMPONENT dev)
endfunction()

add_subdirectory(pubsub)
add_subdirectory(config)
add_subdirectory(ddsls)

# VxWorks build machines use OpenJDK 8, which lack jfxrt.jar. Do not build launcher on that platform.
#
# TODO Instead of making inclusion dependent on platform name, the condition should instead be on the
#      jdk vendor (Oracle JDK, not OpenJDK). Find a way to make CMake aware of jdk vendor.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
  add_subdirectory(launcher)
endif()
