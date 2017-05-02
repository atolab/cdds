add_definitions(-DVDDS_BUILD=1)

find_package(Abstraction REQUIRED)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

string(TOLOWER ${CMAKE_SYSTEM_NAME} platform)

set (posix_platforms darwin linux)
IF(${platform} IN_LIST posix_platforms)
  set(platform posix)
ENDIF()

PREPEND(srcs_platform ${platform} os_platform_errno.c os_platform_heap.c os_platform_init.c os_platform_process.c os_platform_socket.c os_platform_stdlib.c os_platform_sync.c os_platform_thread.c os_platform_time.c)

PREPEND(srcs_os os os_atomics.c os_init.c os_report.c os_socket.c os_thread.c os_time.c ${srcs_platform})

PREPEND(srcs_util util ut_avl.c ut_crc.c ut_expand_envvars.c ut_fibheap.c ut_hopscotch.c ut_thread_pool.c ut_xmlparser.c)

PREPEND(srcs_ddsi ddsi ddsi_ser.c ddsi_ssl.c ddsi_tcp.c ddsi_tran.c ddsi_udp.c q_addrset.c q_bitset_inlines.c q_bswap.c q_bswap_inlines.c q_config.c q_ddsi_discovery.c q_debmon.c q_entity.c q_ephash.c q_gc.c q_init.c q_lat_estim.c q_lease.c q_log.c q_md5.c q_misc.c q_nwif.c q_pcap.c q_plist.c q_qosmatch.c q_radmin.c q_receive.c q_security.c q_servicelease.c q_sockwaitset.c q_thread.c q_thread_inlines.c q_time.c q_transmit.c q_whc.c q_xevent.c q_xmsg.c q_freelist.c sysdeps.c)

PREPEND(srcs_kernel kernel dds_alloc.c dds_iid.c dds_participant.c dds_reader.c dds_thread.c dds_writer.c dds_condition.c dds_init.c dds_publisher.c dds_rhc.c dds_time.c q_osplser.c dds_domain.c dds_instance.c dds_qos.c dds_status.c dds_tkmap.c dds_entity.c dds_key.c dds_querycond.c dds_statuscond.c dds_topic.c dds_err.c dds_listener.c dds_read.c dds_stream.c dds_waitset.c dds_guardcond.c dds_log.c dds_readcond.c dds_subscriber.c dds_write.c)

set(srcs ${srcs_os} ${srcs_util} ${srcs_ddsi} ${srcs_kernel})

add_library(vdds ${srcs})
# Create a psuedo-target that other targets (i.e. examples, tests) can depend on and can also be
# provided as import-target by a package-file when building those targets outside the regular
# Vortex build-tree (i.e. the installed tree)
add_library(Vortex::DDSC99 ALIAS vdds)

target_link_libraries(vdds Abstraction)

#target_include_directories(vdds PUBLIC ../include)
