cmake_minimum_required(VERSION 3.5)

if (NOT TARGET VortexDDS::vddsc)
    # Find the VortexDDS package. If it is not in a default location, try
    # finding it relative to the example where it most likely resides.
    find_package(VortexDDS REQUIRED PATHS "${CMAKE_SOURCE_DIR}/../../")
endif()

add_executable(pubsub pubsub.c common.c testtype.c porting.c)
target_link_libraries(pubsub util VortexDDS::vddsc)

# FIXME
# I'd rather not see if(WIN32) stuff outside of some abstraction CMake.
# In this case, if pubsub builds on top of abstraction-layer it should
# explicitly depend on it. Otherwise use the find_package(Abstraction)
# and depend on that. It will set the flags to build os-stuff without
# warnings on Windows.
if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
  add_subdirectory(tests)
endif()

install(
  TARGETS pubsub
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  COMPONENT dev
)
