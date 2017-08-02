find_package(Abstraction REQUIRED)

include (GenerateExportHeader)

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

PREPEND(srcs_vddsc vddsc dds_alloc.c dds_coherent.c dds_iid.c dds_participant.c dds_reader.c dds_thread.c dds_writer.c dds_init.c dds_publisher.c dds_rhc.c dds_time.c q_osplser.c dds_domain.c dds_instance.c dds_qos.c dds_tkmap.c dds_entity.c dds_key.c dds_querycond.c dds_topic.c dds_err.c dds_listener.c dds_read.c dds_stream.c dds_waitset.c dds_log.c dds_readcond.c dds_subscriber.c dds_write.c)

option(VDDSC_SHARED "Build VDDSC as a shared library" ON)

if(VDDSC_SHARED AND ((NOT DEFINED BUILD_SHARED_LIBS) OR BUILD_SHARED_LIBS))
  # BUILD_SHARED_LIBS is set to off by for example VxWorks DKM environment
  add_library(vddsc SHARED "")
else()
  if(VDDSC_SHARED)
    message(STATUS "Option VDDSC_SHARED ignored. Only static libraries supported on this platform.")
  endif()
  add_library(vddsc "")
endif()

target_sources(vddsc PRIVATE ${srcs_ddsi} ${srcs_vddsc})
target_link_libraries(vddsc PRIVATE util)
set_target_file_ids(vddsc)
# SOVERSION should increase on incompatible ABI change
set_target_properties(vddsc PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

generate_export_header(
  vddsc
  BASE_NAME DDS
  EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/exports/dds/dds_export.h")

# Generator expressions are used here to refer to different paths during build
# and install phases. It is a requirement for creating relocatable packages.
target_include_directories(vddsc PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/exports/>"
  "$<INSTALL_INTERFACE:include>")
target_include_directories(vddsc PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>")

configure_file("${CMAKE_SOURCE_DIR}/include/vddsc/vddsc_version.h.in" "include/vddsc/vddsc_version.h")

# Create a pseudo-target that other targets (i.e. examples, tests) can depend
# on and can also be provided as import-target by a package-file when building
# those targets outside the regular Vortex build-tree (i.e. the installed tree)
add_library(${CMAKE_PROJECT_NAME}::vddsc ALIAS vddsc)

install(
  TARGETS vddsc
  EXPORT "${CMAKE_PROJECT_NAME}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  COMPONENT lib)

install(
  FILES "${CMAKE_SOURCE_DIR}/include/dds.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT dev)

# FIXME: Not sure if this is actually required.
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/exports/dds/dds_export.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/dds"
  COMPONENT dev)

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/include/dds"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT dev)

# TODO: improve test inclusion.
if((BUILD_TESTING) AND ((NOT DEFINED MSVC_VERSION) OR (MSVC_VERSION GREATER "1800")))
  add_subdirectory(vddsc/tests)
endif()

