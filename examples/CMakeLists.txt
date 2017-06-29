# Temporary platform-check; pingpong is NOT a WIN32 executable yet.
if(NOT WIN32)
  add_subdirectory(pingpong)
endif()
add_subdirectory(helloworld)
