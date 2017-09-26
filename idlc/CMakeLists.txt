# Verify Maven is available
find_package(Maven 3.0 REQUIRED)

file(GLOB_RECURSE IDLC_G4_SOURCES LIST_DIRECTORIES true *.g4)
file(GLOB_RECURSE IDLC_JAVA_SOURCES LIST_DIRECTORIES true *.java)
file(GLOB_RECURSE IDLC_ST_SOURCES LIST_DIRECTORIES true *.st?)

set(IDLC_JAR "${CMAKE_CURRENT_BINARY_DIR}/target/idlc-jar-with-dependencies.jar")
mark_as_advanced(IDLC_JAR)

# Maven is executed within the idlc directory located in the build directory
# and generated sources are stored in idlc/target. Non-generated sources,
# however, do not reside in the build directory and Maven must be instructed
# to use those. To allow maven to be executed from both the source and build
# directories (idlc may be moved to it's own repository), the pom.xml file is
# pulled through the configure_file function with basedir set to the original
# source directory. It is a cute little hack to avoid having to use different
# profiles etc.
set(basedir "${CMAKE_CURRENT_SOURCE_DIR}")
mark_as_advanced(basedir)
configure_file("pom.xml" "pom.xml")
configure_file("com/prismtech/vortex/Project.java.in" "com/prismtech/vortex/Project.java")

add_custom_command(
    OUTPUT  "${IDLC_JAR}"
    COMMAND "${Maven_EXECUTABLE}"
    ARGS    "-q" "package"
    DEPENDS "pom.xml" ${IDLC_G4_SOURCES} ${IDLC_JAVA_SOURCES} ${IDLC_ST_SOURCES}
    COMMENT "Building JAR file ${IDLC_JAR}")

include(IdlcGenerate.cmake)

install(
  FILES "IdlcGenerate.cmake"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME_NOSPACE}"
  COMPONENT dev)

install(
  FILES "${IDLC_SCRIPT_IN}"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME_NOSPACE}/idlc"
  COMPONENT dev)

install(
  FILES "${IDLC_JAR}"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME_NOSPACE}/idlc"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  COMPONENT dev)

