set(CMAKE_INSTALL_EXAMPLESDIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/examples")

add_subdirectory(helloworld)
add_subdirectory(roundtrip)
add_subdirectory(throughput)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(platform_exclude "examples/helloworld/Makefile|examples/install_examples.sh")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(platform_exclude "examples/helloworld/vs|examples/helloworld/HelloWorld\.sln")
else()
  set(platform_exclude "this_is_a_placeholder")
endif()

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/examples/"
  DESTINATION "${CMAKE_INSTALL_EXAMPLESDIR}"
  COMPONENT dev
  USE_SOURCE_PERMISSIONS
  PATTERN "CMakeLists.export" EXCLUDE
  PATTERN "examples/CMakeLists.txt" EXCLUDE
  REGEX ${platform_exclude} EXCLUDE)
