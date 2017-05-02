if(NOT WIN32)
  message(FATAL_ERROR "IP Helper API is only available on Windows")
endif()

# Verify that the library actually exists; should be the case on any normal Windows
find_library(IPHLPAPI_LIB iphlpapi)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IPHelper DEFAULT_MSG IPHLPAPI_LIB)

if(IPHelper_FOUND AND NOT TARGET IPHelper)
  add_library(IPHelper INTERFACE IMPORTED)
  # In order to be relocatable don't use IPHLPAPI_LIB here.
  set_property(TARGET IPHelper PROPERTY INTERFACE_LINK_LIBRARIES iphlpapi)
endif()
