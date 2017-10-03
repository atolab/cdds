find_package(Abstraction REQUIRED)

add_executable(pubsub pubsub.c common.c testtype.c porting.c)
target_link_libraries(pubsub vddsc OSAPI)

# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
  add_subdirectory(tests)
endif()

install(
  TARGETS pubsub
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  COMPONENT dev
)
