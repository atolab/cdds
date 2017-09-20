find_package(Java REQUIRED)
include(UseJava)

#enable_testing()

project (chamconf)

# TODO This should specify 1.8, but VS2013 build machine only has jdk 1.7, this should be fixed soon
set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.7" "-target" "1.7")

file(GLOB_RECURSE JAVA_SOURCES LIST_DIRECTORIES true *.java)
set(RESOURCES metaconfig.xml metaconfig.xsd resources/ptlogoc16.png resources/ptlogoc24.png resources/ptlogoc32.png resources/ptlogoc48.png)

add_jar(chamconf ${JAVA_SOURCES} ${RESOURCES} ENTRY_POINT org.opensplice.config.SpliceConfig)

#add_test(NAME TestHelloWorld COMMAND ${Java_JAVA_EXECUTABLE} -cp ${_jarFile} HelloWorld)
