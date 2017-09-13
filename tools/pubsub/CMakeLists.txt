cmake_minimum_required(VERSION 3.5)

if (NOT TARGET VortexDDS::vddsc)
    # Find the VortexDDS package. If it is not in a default location, try
    # finding it relative to the example where it most likely resides.
    find_package(VortexDDS REQUIRED PATHS "${CMAKE_SOURCE_DIR}/../../")
endif()

add_executable(pubsub pubsub.c common.c testtype.c porting.c)
target_link_libraries(pubsub util VortexDDS::vddsc)

if(WIN32)
  # Many of the secure versions provided by Microsoft have failure modes
  # which are not supported by our abstraction layer, so efforts trying
  # to use the _s versions aren't typically the proper solution and C11
  # (which contains most of the secure versions) is 'too new'. So we rely
  # on static detection of misuse instead of runtime detection, so all
  # these warnings can be disabled on Windows.
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
  add_subdirectory(tests)
endif()