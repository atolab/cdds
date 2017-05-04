# Verify Maven is available
find_program(MAVEN_COMMAND mvn)
if(NOT MAVEN_COMMAND)
    message(FATAL_ERROR "Maven is unavailable")
endif()

file(GLOB_RECURSE IDLC_G4_SOURCES LIST_DIRECTORIES true *.g4)
file(GLOB_RECURSE IDLC_JAVA_SOURCES LIST_DIRECTORIES true *.java)
file(GLOB_RECURSE IDLC_ST_SOURCES LIST_DIRECTORIES true *.st?)
mark_as_advanced(IDLC_G4_SOURCES IDLC_JAVA_SOURCES IDLC_ST_SOURCES)

set(LINE_ENDINGS "UNIX")
mark_as_advanced(LINE_ENDINGS)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(EXTENSION ".bat")
    set(LINE_ENDINGS "WIN32")
    mark_as_advanced(EXTENSION)
endif()

set(IDLC_JAR "${CMAKE_CURRENT_BINARY_DIR}/target/idlc-jar-with-dependencies.jar")
mark_as_advanced(IDLC_JAR)

configure_file(
    "cmake/dds_idlc${EXTENSION}.in"
    "dds_idlc${EXTENSION}"
    @ONLY
    NEWLINE_STYLE ${LINE_ENDINGS})
# FIXME: C++ IDL compiler depends idlpp. Leave it disabled for now.
#configure_file(
#    "cmake/dds_idlcpp${EXTENSION}.in"
#    "dds_idlcpp${EXTENSION}"
#    @ONLY
#    NEWLINE_STYLE ${LINE_ENDINGS})

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

if(NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows"))
    execute_process(COMMAND chmod +x "idlc/dds_idlc${EXTENSION}")
endif()

add_custom_command(
    OUTPUT "${IDLC_JAR}"
    COMMAND "${MAVEN_COMMAND}" package)

add_custom_target(
    idlc ALL
    DEPENDS "${IDLC_JAR}"
    SOURCES ${IDLC_G4_SOURCES} ${IDLC_JAVA_SOURCES} ${IDLC_ST_SOURCES})

