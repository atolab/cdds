
add_library(Abstraction INTERFACE)

# Link with the platform-specific threads library that find_package provides us
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
target_link_libraries(Abstraction INTERFACE Threads::Threads)

if(WIN32)
  # Link with WIN32 core-libraries
  find_package(Winsock2 REQUIRED)
  find_package(IPHelper REQUIRED)
  target_link_libraries(Abstraction INTERFACE Winsock2 IPHelper)
elseif(UNIX AND NOT APPLE)
  find_package(GetTime REQUIRED)
  target_link_libraries(Abstraction INTERFACE GetTime)
endif()
