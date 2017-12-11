
PREPEND(headers_public_security "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/security>$<INSTALL_INTERFACE:include/security>"
    ddsc_security.h
)


set(IDLC_ARGS "-dll" "FOO")
idlc_generate(SecurityBuiltinTypes security/src/dds_security_builtintopics.idl security/src/dds_security_interface_types.idl)
set(IDLC_ARGS)
target_link_libraries(ddsc PRIVATE SecurityBuiltinTypes)

install(
  DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/security"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT dev)


# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tests")
endif()