# Verify JDK is available
find_package(Java REQUIRED COMPONENTS Development)
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

configure_file(
    "dds_idlc${EXTENSION}.in"
    "dds_idlc${EXTENSION}"
    @ONLY
    NEWLINE_STYLE ${LINE_ENDINGS})
configure_file(
    "dds_idlcpp${EXTENSION}.in"
    "dds_idlcpp${EXTENSION}"
    @ONLY
    NEWLINE_STYLE ${LINE_ENDINGS})

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
    execute_process(COMMAND chmod +x "idlc/dds_idlcpp${EXTENSION}")
endif()

set(IDLC_JAR "target/idlc.jar")
# The jar in the local repository could be used in the script, but it is
# versioned. The user can check out a different branch while the target has
# been built. Since that could cause undefined behavior, it's better to
# prevent that from happening by copying the archive into the build-tree.
set(IDLPP_JAR "target/idlpp.jar")
set(PREPROCESSOR_JAR "${CMAKE_CURRENT_SOURCE_DIR}/repository/com/prismtech/idt/imports/idl/preprocessor/VLITE-01/preprocessor-VLITE-01.jar")
mark_as_advanced(IDLC_JAR IDLPP_JAR PREPROCESSOR_JAR)

add_custom_command(
    OUTPUT "${IDLC_JAR}"
    COMMAND "${MAVEN_COMMAND}" package)

add_custom_command(
    OUTPUT "${IDLPP_JAR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${PREPROCESSOR_JAR}" "${IDLPP_JAR}")

add_custom_target(
    idlc ALL
    DEPENDS "${IDLC_JAR}" "${IDLPP_JAR}"
    SOURCES ${IDLC_G4_SOURCES} ${IDLC_JAVA_SOURCES} ${IDLC_ST_SOURCES})

