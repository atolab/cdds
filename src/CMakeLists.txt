option(INCLUDE_BANDWIDTH_LIMITING "Bandwidth limiting support in DDSI core" ON)
option(INCLUDE_NETWORK_CHANNELS "Network channel support DDSI core" ON)
option(INCLUDE_NETWORK_PARTITIONS "Network partitions support in DDSI core" ON)
option(INCLUDE_SSL "SSL-support in DDSI core" ON)
option(INCLUDE_SSM "Source Specific Multicast support in DDSI core" ON)

find_package(Abstraction REQUIRED)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

# Create the os target
add_subdirectory(os)

PREPEND(srcs_util util ut_avl.c ut_crc.c ut_expand_envvars.c ut_fibheap.c ut_hopscotch.c ut_thread_pool.c ut_xmlparser.c)

PREPEND(srcs_ddsi ddsi ddsi_ser.c ddsi_ssl.c ddsi_tcp.c ddsi_tran.c ddsi_udp.c q_addrset.c q_bitset_inlines.c q_bswap.c q_bswap_inlines.c q_config.c q_ddsi_discovery.c q_debmon.c q_entity.c q_ephash.c q_gc.c q_init.c q_lat_estim.c q_lease.c q_log.c q_md5.c q_misc.c q_nwif.c q_pcap.c q_plist.c q_qosmatch.c q_radmin.c q_receive.c q_servicelease.c q_sockwaitset.c q_thread.c q_thread_inlines.c q_time.c q_transmit.c q_whc.c q_xevent.c q_xmsg.c q_freelist.c sysdeps.c)

PREPEND(srcs_kernel kernel dds_alloc.c dds_iid.c dds_participant.c dds_reader.c dds_thread.c dds_writer.c dds_condition.c dds_init.c dds_publisher.c dds_rhc.c dds_time.c q_osplser.c dds_domain.c dds_instance.c dds_qos.c dds_tkmap.c dds_entity.c dds_key.c dds_querycond.c dds_topic.c dds_err.c dds_listener.c dds_read.c dds_stream.c dds_waitset.c dds_guardcond.c dds_log.c dds_readcond.c dds_subscriber.c dds_write.c)

# Recipe for the vdds target
# The vdds target depends on the OSAbstraction target
add_library(vdds ${srcs_util} ${srcs_ddsi} ${srcs_kernel})

# TODO: Perhaps run a compile-check on q_features.h to check consistency of options without replicating them here?
if(INCLUDE_BANDWIDTH_LIMITING)
  target_compile_definitions(vdds PRIVATE DDSI_INCLUDE_BANDWIDTH_LIMITING)
endif(INCLUDE_BANDWIDTH_LIMITING)
if(INCLUDE_NETWORK_CHANNELS)
  target_compile_definitions(vdds PRIVATE DDSI_INCLUDE_NETWORK_CHANNELS)
endif(INCLUDE_NETWORK_CHANNELS)
if(INCLUDE_NETWORK_PARTITIONS)
  target_compile_definitions(vdds PRIVATE DDSI_INCLUDE_NETWORK_PARTITIONS)
endif(INCLUDE_NETWORK_PARTITIONS)
if(INCLUDE_SSL)
  find_package(OpenSSL REQUIRED)
  target_compile_definitions(vdds PRIVATE DDSI_INCLUDE_SSL OPENSSL_API_COMPAT=0x00908000L)
  target_link_libraries(vdds PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif(INCLUDE_SSL)
if(INCLUDE_SSM)
  target_compile_definitions(vdds PRIVATE DDSI_INCLUDE_SSM)
endif(INCLUDE_SSM)

target_link_libraries(vdds PRIVATE OSAbstraction)
target_compile_definitions(vdds PRIVATE VDDS_BUILD)

# Create a psuedo-target that other targets (i.e. examples, tests) can depend on and can also be
# provided as import-target by a package-file when building those targets outside the regular
# Vortex build-tree (i.e. the installed tree)
add_library(Vortex::DDSC99 ALIAS vdds)


# Just added for install SPIKE. TODO: remove or improve.
install(TARGETS vdds
        ARCHIVE
        DESTINATION lib
        COMPONENT lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/dds.h
        DESTINATION include
        COMPONENT dev)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/dds
        DESTINATION include
        COMPONENT dev)

#target_include_directories(vdds PUBLIC ../include)

# TODO: improve test inclusion.
if(BUILD_TESTING)
  add_subdirectory(kernel/tests)
endif()

