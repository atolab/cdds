find_package(Java REQUIRED)
include(UseJava)

#enable_testing()

project (chamconf)

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.8" "-target" "1.8")

file(GLOB_RECURSE JAVA_SOURCES LIST_DIRECTORIES true *.java)

add_jar(chamconf ${JAVA_SOURCES})

#add_test(NAME TestHelloWorld COMMAND ${Java_JAVA_EXECUTABLE} -cp ${_jarFile} HelloWorld)
