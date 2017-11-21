include(Criterion)
add_criterion_executable(criterion_tools_pubsub . ../common.c ../testtype.c ../porting.c)
target_link_libraries(criterion_tools_pubsub util VortexDDS::ddsc)
