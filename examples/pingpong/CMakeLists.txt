include_directories(generated)
add_executable(ddpingpong ddpingpong.c generated/RoundTrip.c)
target_link_libraries(ddpingpong Vortex::DDSC)
