#include <assert.h>

#include "vddsc/dds.h"
#include "dds__entity.h"
#include "dds__subscriber.h"
#include "dds__publisher.h"
#include "dds__err.h"
#include "dds__report.h"

_Pre_satisfies_(((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER    ) || \
                ((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER) || \
                ((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER    ) || \
                ((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER) )
dds_return_t
dds_begin_coherent(
        _In_ dds_entity_t entity)
{
    dds_return_t ret;

    switch(dds_entity_kind(entity)) {
        case DDS_KIND_READER:
        case DDS_KIND_WRITER:
            /* Invoking on a writer/reader behaves as if invoked on
             * its parent publisher/subscriber. */
            ret = dds_begin_coherent(dds_get_parent(entity));
            break;
        case DDS_KIND_PUBLISHER:
            ret = dds_publisher_begin_coherent(entity);
            break;
        case DDS_KIND_SUBSCRIBER:
            ret = dds_subscriber_begin_coherent(entity);
            break;
        default:
            ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Given entity can not control coherency");
            break;
    }
    return ret;
}

_Pre_satisfies_(((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER    ) || \
                ((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER) || \
                ((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER    ) || \
                ((entity & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER) )
dds_return_t
dds_end_coherent(
        _In_ dds_entity_t entity)
{
    dds_return_t ret;

    switch(dds_entity_kind(entity)) {
        case DDS_KIND_READER:
        case DDS_KIND_WRITER:
            /* Invoking on a writer/reader behaves as if invoked on
             * its parent publisher/subscriber. */
            ret = dds_end_coherent(dds_get_parent(entity));
            break;
        case DDS_KIND_PUBLISHER:
            ret = dds_publisher_end_coherent(entity);
            break;
        case DDS_KIND_SUBSCRIBER:
            ret = dds_subscriber_end_coherent(entity);
            break;
        default:
            ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Given entity can not control coherency");
            break;
    }
    return ret;
}
