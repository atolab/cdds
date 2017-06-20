add_executable(pubsub pubsub.c common.c testtype.c porting.c tglib.c)
target_link_libraries(pubsub Vortex::DDSC)
