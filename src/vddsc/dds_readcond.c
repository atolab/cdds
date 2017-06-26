#include <assert.h>
#include "kernel/dds_reader.h"
#include "kernel/dds_readcond.h"
#include "kernel/dds_waitset.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_entity.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"


static dds_return_t
dds_readcond_delete(dds_entity *e)
{
    dds_rhc_remove_readcondition((dds_readcond*)e);
    return DDS_RETCODE_OK;
}

_Must_inspect_result_ dds_readcond*
dds_create_readcond(
        _In_ dds_reader *rd,
        _In_ dds_entity_kind_t kind,
        _In_ uint32_t mask)
{
    dds_readcond * cond = dds_alloc(sizeof(*cond));
    cond->m_entity.m_hdl = dds_entity_init(&cond->m_entity, (dds_entity*)rd, kind, NULL, NULL, mask);
    cond->m_entity.m_deriver.delete = dds_readcond_delete;
    cond->m_rhc = rd->m_rd->rhc;
    cond->m_sample_states = mask & DDS_ANY_SAMPLE_STATE;
    cond->m_view_states = mask & DDS_ANY_VIEW_STATE;
    cond->m_instance_states = mask & DDS_ANY_INSTANCE_STATE;
    cond->m_rd_guid = ((dds_entity*)rd)->m_guid;
    dds_rhc_add_readcondition (cond);
    return cond;
}

_Pre_satisfies_((reader & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER)
_Must_inspect_result_ dds_entity_t
dds_create_readcondition(
        _In_ dds_entity_t reader,
        _In_ uint32_t mask)
{
    dds_entity_t hdl;
    dds_reader * rd;
    int32_t ret;

    ret = dds_reader_lock(reader, &rd);
    if (ret == DDS_RETCODE_OK) {
        dds_readcond *cond = dds_create_readcond(rd, DDS_KIND_COND_READ, mask);
        hdl = cond->m_entity.m_hdl;
        dds_reader_unlock(rd);
    } else {
        hdl = DDS_ERRNO(ret, DDS_MOD_COND, DDS_ERR_M2);
    }

    return hdl;
}

_Pre_satisfies_(((readcond & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((readcond & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY) )
dds_entity_t
dds_get_datareader(
        _In_ dds_entity_t readcond)
{
    if (readcond > 0) {
        if (dds_entity_kind(readcond) == DDS_KIND_COND_READ) {
            return dds_get_parent(readcond);
        } else if (dds_entity_kind(readcond) == DDS_KIND_COND_QUERY) {
            return dds_get_parent(readcond);
        } else {
            return (dds_entity_t)DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION, DDS_MOD_READER, DDS_ERR_M1);
        }
    }
    return readcond;
}
