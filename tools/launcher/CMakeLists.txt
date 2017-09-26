# Verify Maven is available
find_package(Maven 3.0 REQUIRED)

file(GLOB_RECURSE LAUNCHERUI_JAVA_SOURCES LIST_DIRECTORIES true launcherui/oLauncher/*.java)
file(GLOB_RECURSE LAUNCHER_JAVA_SOURCES LIST_DIRECTORIES true launcherui/chamLauncher/*.java)

set(LAUNCHER_JAR "${CMAKE_CURRENT_BINARY_DIR}/launcherui/chamLauncher/target/chamlauncher.jar")
mark_as_advanced(LAUNCHER_JAR)

set(basedir "${CMAKE_CURRENT_SOURCE_DIR}/launcherui")
mark_as_advanced(basedir)
configure_file("launcherui/chamLauncher/pom.xml" "launcherui/chamLauncher/pom.xml")
configure_file("launcherui/oLauncher/pom.xml" "launcherui/oLauncher/pom.xml")
configure_file("launcherui/pom.xml" "launcherui/pom.xml")

add_custom_command(
    OUTPUT  "${LAUNCHER_JAR}"
    COMMAND "${Maven_EXECUTABLE}"
    ARGS    "-q" "package"
    DEPENDS "launcherui/pom.xml" "launcherui/oLauncher/pom.xml" "launcherui/chamLauncher/pom.xml" ${LAUNCHERUI_JAVA_SOURCES} ${LAUNCHER_JAVA_SOURCES}
    WORKING_DIRECTORY launcherui
    COMMENT "Building JAR file ${LAUNCHER_JAR}")

add_custom_target(launcher ALL DEPENDS "${LAUNCHER_JAR}")

install(
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/launcherui/chamLauncher/target/jfx/native/chamlauncher/app"
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  USE_SOURCE_PERMISSIONS
  COMPONENT dev)

install(
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/launcherui/chamLauncher/target/jfx/native/chamlauncher/runtime"
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  USE_SOURCE_PERMISSIONS
  COMPONENT dev)

# FIXME
# Refactor this to use install TARGET instead, which might take care of the platform specific file name
if (WIN32)
install(
  PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/launcherui/chamLauncher/target/jfx/native/chamlauncher/chamlauncher.exe"
           "${CMAKE_CURRENT_BINARY_DIR}/launcherui/chamLauncher/target/jfx/native/chamlauncher/packager.dll"
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  COMPONENT dev)
else()
install(
  PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/launcherui/chamLauncher/target/jfx/native/chamlauncher/chamlauncher"
           "${CMAKE_CURRENT_BINARY_DIR}/launcherui/chamLauncher/target/jfx/native/chamlauncher/libpackager.so"
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  COMPONENT dev)
endif()
