find_package(Java REQUIRED)
include(UseJava)

set(CONFJAR_TARGET "${CMAKE_PROJECT_NAME_NOSPACE_SMALL}conf")

# TODO This should specify 1.8, but VS2013 build machine only has jdk 1.7, this should be fixed soon
set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.7" "-target" "1.7")

# Oddly enough, add_jar can't deal with a resource having an absolute path (experienced on Windows), so
# generate into the list-dir.
configure_file("metaconfig.xml.in" "${CMAKE_CURRENT_LIST_DIR}/metaconfig.xml" @ONLY)

file(GLOB_RECURSE JAVA_SOURCES LIST_DIRECTORIES true *.java)

set(RESOURCES metaconfig.xml metaconfig.xsd resources/ptlogoc16.png resources/ptlogoc24.png resources/ptlogoc32.png resources/ptlogoc48.png)
add_jar(${CONFJAR_TARGET} ${JAVA_SOURCES} ${RESOURCES} ENTRY_POINT org.opensplice.config.SpliceConfig)

#add_test(NAME TestHelloWorld COMMAND ${Java_JAVA_EXECUTABLE} -cp ${_jarFile} HelloWorld)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${CONFJAR_TARGET}.jar"
  DESTINATION "${CMAKE_INSTALL_TOOLSDIR}"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  COMPONENT dev)
