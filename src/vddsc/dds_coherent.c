#include <assert.h>

#include "dds.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_subscriber.h"
#include "kernel/dds_publisher.h"

dds_return_t
dds_begin_coherent
(
    _In_ dds_entity_t e
)
{
    dds_return_t ret;

    switch(dds_entity_kind(e)) {
        case DDS_KIND_READER:
        case DDS_KIND_WRITER:
            /* Invoking on a writer/reader behaves as if invoked on
             * its parent publisher/subscriber. */
            ret = dds_begin_coherent(dds_get_parent(e));
            break;
        case DDS_KIND_PUBLISHER:
            ret = dds_publisher_begin_coherent(e);
            break;
        case DDS_KIND_SUBSCRIBER:
            ret = dds_subscriber_begin_coherent(e);
            break;
        default:
            ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_KERNEL, 0);
            break;
    }
    return ret;
}

dds_return_t
dds_end_coherent
(
    _In_ dds_entity_t e
)
{
    dds_return_t ret;

    switch(dds_entity_kind(e)) {
        case DDS_KIND_READER:
        case DDS_KIND_WRITER:
            /* Invoking on a writer/reader behaves as if invoked on
             * its parent publisher/subscriber. */
            ret = dds_end_coherent(dds_get_parent(e));
            break;
        case DDS_KIND_PUBLISHER:
            ret = dds_publisher_end_coherent(e);
            break;
        case DDS_KIND_SUBSCRIBER:
            ret = dds_subscriber_end_coherent(e);
            break;
        default:
            ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_KERNEL, 0);
            break;
    }
    return ret;
}
