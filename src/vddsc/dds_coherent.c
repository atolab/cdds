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
    dds_entity_t entity = e;
    dds_return_t ret;
    switch(e->m_kind) {
        case DDS_TYPE_WRITER:
            /* Invoking on a writer behaves as if invoked on its parent publisher */
            entity = e->m_parent;
            assert(entity->m_kind == DDS_TYPE_PUBLISHER);
            /* no break */
        case DDS_TYPE_PUBLISHER:
            ret = dds_publisher_begin_coherent(entity);
            break;
        case DDS_TYPE_READER:
            /* Invoking on a reader behaves as if invoked on its parent subscriber */
            entity = e->m_parent;
            assert(entity->m_kind == DDS_TYPE_SUBSCRIBER);
            /* no break */
        case DDS_TYPE_SUBSCRIBER:
            ret = dds_subscriber_begin_coherent(entity);
            break;
        default:
            ret = DDS_RETCODE_BAD_PARAMETER;
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
    dds_entity_t entity = e;
    dds_return_t ret;
    switch(e->m_kind) {
        case DDS_TYPE_WRITER:
            /* Invoking on a writer behaves as if invoked on its parent publisher */
            entity = e->m_parent;
            assert(entity->m_kind == DDS_TYPE_PUBLISHER);
            /* no break */
        case DDS_TYPE_PUBLISHER:
            ret = dds_publisher_end_coherent(entity);
            break;
        case DDS_TYPE_READER:
            /* Invoking on a reader behaves as if invoked on its parent subscriber */
            entity = e->m_parent;
            assert(entity->m_kind == DDS_TYPE_SUBSCRIBER);
            /* no break */
        case DDS_TYPE_SUBSCRIBER:
            ret = dds_subscriber_end_coherent(entity);
            break;
        default:
            ret = DDS_RETCODE_BAD_PARAMETER;
            break;
    }
    return ret;
}
