message(STATUS "Loading platform abstraction package")

add_library(Abstraction INTERFACE)

# Link with the platform-specific threads library that find_package provides us
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
find_package(GetTime REQUIRED)
target_link_libraries(Abstraction INTERFACE Threads::Threads GetTime)

if(WIN32)
  # Link with WIN32 core-libraries
  target_link_libraries(Abstraction INTERFACE wsock32 ws2_32 Iphlpapi)
endif()
