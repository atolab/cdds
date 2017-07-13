#include <assert.h>
#include "kernel/dds_types.h"
#include "kernel/dds_reader.h"
#include "kernel/dds_readcond.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_entity.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"

static dds_return_t
dds_readcond_delete(
        dds_entity *e)
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
    cond->m_entity.m_hdl = dds_entity_init(&cond->m_entity, (dds_entity*)rd, kind, NULL, NULL, 0);
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
        assert(cond);
        hdl = cond->m_entity.m_hdl;
        dds_reader_unlock(rd);
    } else {
        hdl = DDS_ERRNO(ret, DDS_MOD_COND, DDS_ERR_M2);
    }

    return hdl;
}

_Pre_satisfies_(((condition & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((condition & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY) )
dds_entity_t
dds_get_datareader(
        _In_ dds_entity_t condition)
{
    if (condition >= 0) {
        if (dds_entity_kind(condition) == DDS_KIND_COND_READ) {
            return dds_get_parent(condition);
        } else if (dds_entity_kind(condition) == DDS_KIND_COND_QUERY) {
            return dds_get_parent(condition);
        } else {
            dds_return_t ret = dds_valid_hdl(condition, DDS_KIND_DONTCARE);
            if (ret == DDS_RETCODE_OK) {
                return (dds_entity_t)DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION, DDS_MOD_COND, DDS_ERR_M1);
            } else {
                return (dds_entity_t)DDS_ERRNO(ret, DDS_MOD_COND, DDS_ERR_M1);
            }
        }
    }
    return condition;
}


_Pre_satisfies_(((condition & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((condition & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY) )
dds_return_t
dds_get_mask(
        _In_ dds_entity_t condition,
        _Out_ uint32_t   *mask)
{
    dds_return_t ret = condition;
    dds_readcond *cond;
    if (condition >= 0) {
        if (mask != NULL) {
            if ((dds_entity_kind(condition) == DDS_KIND_COND_READ ) ||
                (dds_entity_kind(condition) == DDS_KIND_COND_QUERY) ){
                ret = dds_entity_lock(condition, DDS_KIND_DONTCARE, (dds_entity**)&cond);
                if (ret == DDS_RETCODE_OK) {
                    *mask = (cond->m_sample_states | cond->m_view_states | cond->m_instance_states);
                    dds_entity_unlock((dds_entity*)cond);
                } else {
                    ret = DDS_ERRNO(ret, DDS_MOD_COND, DDS_ERR_M1);
                }
            } else {
                ret = dds_valid_hdl(condition, DDS_KIND_DONTCARE);
                if (ret == DDS_RETCODE_OK) {
                    ret = DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION, DDS_MOD_COND, DDS_ERR_M1);
                } else {
                    ret = DDS_ERRNO(ret, DDS_MOD_COND, DDS_ERR_M1);
                }
            }
        } else {
            ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_COND, DDS_ERR_M1);
        }
    }
    return ret;
}
