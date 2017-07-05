find_package(Abstraction REQUIRED)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

# Create the os & util target
add_subdirectory(os)
add_subdirectory(util)

PREPEND(srcs_ddsi ddsi ddsi_ser.c ddsi_ssl.c ddsi_tcp.c ddsi_tran.c ddsi_udp.c q_addrset.c q_bitset_inlines.c q_bswap.c q_bswap_inlines.c q_config.c q_ddsi_discovery.c q_debmon.c q_entity.c q_ephash.c q_gc.c q_init.c q_lat_estim.c q_lease.c q_log.c q_md5.c q_misc.c q_nwif.c q_pcap.c q_plist.c q_qosmatch.c q_radmin.c q_receive.c q_security.c q_servicelease.c q_sockwaitset.c q_thread.c q_thread_inlines.c q_time.c q_transmit.c q_whc.c q_xevent.c q_xmsg.c q_freelist.c sysdeps.c)

PREPEND(srcs_vddsc vddsc dds_alloc.c dds_coherent.c dds_iid.c dds_participant.c dds_reader.c dds_thread.c dds_writer.c dds_condition.c dds_init.c dds_publisher.c dds_rhc.c dds_time.c q_osplser.c dds_domain.c dds_instance.c dds_qos.c dds_tkmap.c dds_entity.c dds_key.c dds_querycond.c dds_topic.c dds_err.c dds_listener.c dds_read.c dds_stream.c dds_waitset.c dds_guardcond.c dds_log.c dds_readcond.c dds_subscriber.c dds_write.c)

# The vddsc target depends on the OSAPI target
add_library(vddsc SHARED ${srcs_ddsi} ${srcs_vddsc})
target_link_libraries(vddsc PRIVATE OSAPI util)
target_compile_definitions(vddsc PRIVATE VDDS_BUILD)

target_include_directories(vddsc PUBLIC "${CMAKE_SOURCE_DIR}/include/" "${CMAKE_BINARY_DIR}/exports/")

# Create a psuedo-target that other targets (i.e. examples, tests) can depend on and can also be
# provided as import-target by a package-file when building those targets outside the regular
# Vortex build-tree (i.e. the installed tree)
add_library(Vortex::DDSC ALIAS vddsc)


# Just added for install SPIKE. TODO: remove or improve.
install(TARGETS vddsc
        ARCHIVE
        DESTINATION lib
        COMPONENT lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/dds.h
        DESTINATION include
        COMPONENT dev)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/dds
        DESTINATION include
        COMPONENT dev)

# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
  add_subdirectory(vddsc/tests)
endif()

