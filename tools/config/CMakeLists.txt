find_package(Java 1.8 REQUIRED)
include(UseJava)
#enable_testing()

project (chamconf)

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.8" "-target" "1.8")

file(GLOB_RECURSE JAVA_SOURCES LIST_DIRECTORIES true *.java)
set(RESOURCES metaconfig.xml metaconfig.xsd resources/ptlogoc16.png resources/ptlogoc24.png resources/ptlogoc32.png resources/ptlogoc48.png)

add_jar(chamconf ${JAVA_SOURCES} ${RESOURCES} ENTRY_POINT org.opensplice.config.SpliceConfig)

#add_test(NAME TestHelloWorld COMMAND ${Java_JAVA_EXECUTABLE} -cp ${_jarFile} HelloWorld)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/chamconf.jar"
  DESTINATION "${CMAKE_INSTALL_TOOLSDIR}"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  COMPONENT dev)
