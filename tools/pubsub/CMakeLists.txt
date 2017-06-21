add_executable(pubsub pubsub.c common.c testtype.c porting.c)
target_link_libraries(pubsub Vortex::DDSC)
