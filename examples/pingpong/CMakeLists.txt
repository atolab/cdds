idlc_generate(RoundTrip_SRCS RoundTrip_HDRS "generated/RoundTrip.idl")
add_executable(ddpingpong ddpingpong.c ${RoundTrip_SRCS})
target_link_libraries(ddpingpong Vortex::DDSC)
target_include_directories(ddpingpong PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
