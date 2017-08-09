idlc_generate(HelloWorldData_lib "HelloWorldData.idl")
add_executable(HelloworldPublisher publisher.c)
add_executable(HelloworldSubscriber subscriber.c)
target_link_libraries(HelloworldPublisher HelloWorldData_lib VortexDDS::vddsc)
target_link_libraries(HelloworldSubscriber HelloWorldData_lib VortexDDS::vddsc)

# Get the HelloWorldData_lib generated files for installation
FUNCTION(CHANGE_EXTENSION FILES EXTENSION)
   SET(FILES_EXT "")
   FOREACH(f ${ARGN})
       get_filename_component(FILE_DIR ${f} DIRECTORY)
       get_filename_component(FILE_WE  ${f} NAME_WE)
       LIST(APPEND FILES_EXT "${FILE_DIR}/${FILE_WE}.${EXTENSION}")
   ENDFOREACH(f)
   SET(${FILES} "${FILES_EXT}" PARENT_SCOPE)
ENDFUNCTION(CHANGE_EXTENSION)
get_target_property(GENERATED_SOURCES HelloWorldData_lib INTERFACE_SOURCES)
change_extension(GENERATED_HEADERS "h" ${GENERATED_SOURCES})
LIST(APPEND GENERATED_FILES ${GENERATED_SOURCES} ${GENERATED_HEADERS})

install(
  TARGETS HelloworldSubscriber HelloworldPublisher
  DESTINATION "${CMAKE_INSTALL_EXAMPLESDIR}/helloworld/bin"
  COMPONENT dev)

install(
  FILES "CMakeLists.export"
  DESTINATION "${CMAKE_INSTALL_EXAMPLESDIR}/helloworld"
  COMPONENT dev
  RENAME "CMakeLists.txt")

install(
  FILES ${GENERATED_FILES}
  DESTINATION "${CMAKE_INSTALL_EXAMPLESDIR}/helloworld/generated"
  COMPONENT dev)
