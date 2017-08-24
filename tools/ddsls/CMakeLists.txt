#idlc_generate(dds_builtInTopics "dds_builtInTopics.idl")
add_executable(ddsls ddsls.c dds_builtInTopics.c)
#target_link_libraries(ddsls dds_builtInTopics util VortexDDS::vddsc)
target_link_libraries(ddsls VortexDDS::vddsc)
