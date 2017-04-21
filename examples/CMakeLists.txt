cmake_minimum_required(VERSION 3.5)

# Temporary platform-check; pingpong is NOT a WIN32 executable.
if(NOT WIN32)
  add_subdirectory(pingpong)
endif()
add_subdirectory(helloworld)
