idlc_generate(RoundTrip_ddpingpong "RoundTrip.idl")
add_executable(ddpingpong ddpingpong.c)
target_link_libraries(ddpingpong RoundTrip_ddpingpong Vortex::DDSC)

