if (NOT TARGET Vortex::DDSC99)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
    find_package(Vortex REQUIRED)
endif()

# Temporary platform-check; pingpong is NOT a WIN32 executable.
if(NOT WIN32)
  add_subdirectory(pingpong)
endif()
add_subdirectory(helloworld)
