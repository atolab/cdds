PREPEND(srcs_vddsc "${CMAKE_CURRENT_LIST_DIR}"
    dds_alloc.c
    dds_coherent.c
    dds_iid.c
    dds_participant.c
    dds_reader.c
    dds_writer.c
    dds_init.c
    dds_publisher.c
    dds_rhc.c
    dds_time.c
    q_osplser.c
    dds_domain.c
    dds_instance.c
    dds_qos.c
    dds_tkmap.c
    dds_entity.c
    dds_key.c
    dds_querycond.c
    dds_topic.c
    dds_err.c
    dds_listener.c
    dds_read.c
    dds_stream.c
    dds_waitset.c
    dds_log.c
    dds_readcond.c
    dds_subscriber.c
    dds_write.c
)

# The includes should reside close to the code. As long as that's not the case,
# pull them in from this CMakeLists.txt.
# Anything PUBLIC can never be absolute paths, so use generator expressions. 
PREPEND(hdrs_public_vddsc "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>$<INSTALL_INTERFACE:include>"
    dds.h
    dds/dds_public_error.h
    dds/dds_public_impl.h
    dds/dds_public_listener.h
    dds/dds_public_log.h
    dds/dds_public_qos.h
    dds/dds_public_status.h
    dds/dds_public_stream.h
    dds/dds_public_time.h
)

configure_file("${CMAKE_SOURCE_DIR}/include/vddsc/vddsc_version.h.in" "include/vddsc/vddsc_version.h")

PREPEND(hdrs_private_vddsc "${CMAKE_SOURCE_DIR}/include/kernel"
    dds_alloc.h
    dds_domain.h
    dds_entity.h
    dds_iid.h
    dds_init.h
    dds_key.h
    dds_listener.h
    dds_participant.h
    dds_publisher.h
    dds_qos.h
    dds_querycond.h
    dds_readcond.h
    dds_reader.h
    dds_rhc.h
    dds_stream.h
    dds_subscriber.h
    dds_tkmap.h
    dds_topic.h
    dds_types.h
    dds_write.h
    dds_writer.h
    q_osplser.h
)

generate_export_header(
    vddsc
    BASE_NAME DDS
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/dds/dds_export.h"
)

target_sources(vddsc
    PRIVATE
        ${srcs_vddsc}
        ${hdrs_private_vddsc}
        "${CMAKE_CURRENT_BINARY_DIR}/include/vddsc/vddsc_version.h"
    PUBLIC
        ${hdrs_public_vddsc}
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/dds/dds_export.h>$<INSTALL_INTERFACE:include/dds/dds_export.h>"
)
