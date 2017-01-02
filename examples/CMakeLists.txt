cmake_minimum_required(VERSION 3.5)

include_directories(generated)
add_executable(ddpingpong ddpingpong.c generated/RoundTrip.c)
target_link_libraries(ddpingpong vdds)
add_dependencies(ddpingpong vdds)
