idlc_generate(HelloWorldData_lib "HelloWorldData.idl")
add_executable(HelloworldPublisher publisher.c)
add_executable(HelloworldSubscriber subscriber.c)
target_link_libraries(HelloworldPublisher HelloWorldData_lib VortexDDS::vddsc)
target_link_libraries(HelloworldSubscriber HelloWorldData_lib VortexDDS::vddsc)

install(
  TARGETS HelloworldSubscriber HelloworldPublisher
  DESTINATION "${CMAKE_INSTALL_EXAMPLESDIR}/helloworld/bin"
  COMPONENT dev)

idlc_install_generated(
  "${CMAKE_INSTALL_EXAMPLESDIR}/helloworld"
  dev
  "HelloWorldData.idl")
