if(NOT WIN32)
  message(FATAL_ERROR "Winsock2 is only available on Windows")
endif()

# Verify that the library actually exists; should be the case on any normal Windows
find_library(WS_32_LIB ws2_32)

find_package_handle_standard_args(Winsock2 DEFAULT_MSG WS_32_LIB)

if(Winsock2_FOUND AND NOT TARGET Winsock2)
  add_library(Winsock2 INTERFACE IMPORTED)
  # In order to be relocatable don't use WS_32_LIB here.
  set_property(TARGET Winsock2 PROPERTY INTERFACE_LINK_LIBRARIES ws2_32)
endif()
