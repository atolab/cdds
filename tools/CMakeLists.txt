set(CMAKE_INSTALL_TOOLSDIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/tools")

add_subdirectory(pubsub)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(platform_exclude "tools/pubsub/Makefile")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(platform_exclude "examples/pubsub/vs|examples/pubsub/pubsub\.sln")
else()
  set(platform_exclude "this_is_a_placeholder")
endif()

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/tools/"
  DESTINATION "${CMAKE_INSTALL_TOOLSDIR}"
  COMPONENT dev
  PATTERN "CMakeLists.export" EXCLUDE
  PATTERN "tools/CMakeLists.txt" EXCLUDE
  REGEX ${platform_exclude} EXCLUDE)