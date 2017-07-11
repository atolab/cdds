# Temporary platform-check; pingpong is NOT a WIN32 executable yet.
if(NOT WIN32)
  add_subdirectory(pingpong)
endif()
add_subdirectory(hello)
add_subdirectory(roundtrip)
add_subdirectory(throughput)