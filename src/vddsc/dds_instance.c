#include <assert.h>
#include <string.h>
#include "dds.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_write.h"
#include "kernel/dds_writer.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "ddsi/ddsi_ser.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_report.h"


_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_writedispose(
       _In_ dds_entity_t writer,
       _In_ const void *data)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    ret =  dds_writedispose_ts(writer, data, dds_time());
    DDS_REPORT_FLUSH(ret < 0 );
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose(
       _In_ dds_entity_t writer,
       _In_ const void *data)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    ret =  dds_dispose_ts(writer, data, dds_time());
    DDS_REPORT_FLUSH(ret < 0 );
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose_ih(
       _In_ dds_entity_t writer,
       _In_ dds_instance_handle_t handle)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    ret = dds_dispose_ih_ts(writer, handle, dds_time());
    DDS_REPORT_FLUSH(ret < 0 );
    return ret;
}

static struct tkmap_instance*
dds_instance_find(
        _In_ const dds_topic *topic,
        _In_ const void *data,
        _In_ const bool create)
{
    serdata_t sd = serialize_key (gv.serpool, topic->m_stopic, data);
    struct tkmap_instance * inst = dds_tkmap_find (topic, sd, false, create);
    ddsi_serdata_unref (sd);
    return inst;
}

static void
dds_instance_remove(
        _In_     const dds_topic *topic,
        _In_opt_ const void *data,
        _In_     dds_instance_handle_t handle)
{
    struct tkmap_instance * inst;

    if (handle != DDS_HANDLE_NIL) {
        inst = dds_tkmap_find_by_id (gv.m_tkmap, handle);
    } else {
        assert (data);
        inst = dds_instance_find (topic, data, false);
    }

    if (inst) {
        struct thread_state1 * const thr = lookup_thread_state();
        const bool asleep = thr ? !vtime_awake_p(thr->vtime) : false;
        if (asleep) {
            thread_state_awake(thr);
        }
        dds_tkmap_instance_unref (inst);
        if (asleep) {
            thread_state_asleep(thr);
        }
    }
}

static const dds_topic*
dds_instance_info(
        _In_ dds_entity *e)
{
    const dds_topic *topic = NULL;

    assert (e);
    assert ((dds_entity_kind(e->m_hdl) == DDS_KIND_READER) || (dds_entity_kind(e->m_hdl) == DDS_KIND_WRITER));

    if (dds_entity_kind(e->m_hdl) == DDS_KIND_READER) {
        topic = ((dds_reader*)e)->m_topic;
    } else {
        topic = ((dds_writer*)e)->m_topic;
    }
    return topic;
}

static const dds_topic * dds_instance_info_by_hdl (dds_entity_t e)
{
    const dds_topic * topic = NULL;
    dds_retcode_t rc;
    dds_entity *w_or_r;
    rc = dds_entity_lock(e, DDS_KIND_WRITER, &w_or_r);
    if (rc == DDS_RETCODE_ILLEGAL_OPERATION) {
        rc = dds_entity_lock(e, DDS_KIND_READER, &w_or_r);
    }
    if (rc == DDS_RETCODE_OK) {
        topic = dds_instance_info(w_or_r);
        dds_entity_unlock(w_or_r);
    }
    return topic;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_register_instance(
        _In_ dds_entity_t writer,
        _Out_ dds_instance_handle_t *handle,
        _In_ const void *data)
{
    struct tkmap_instance * inst;
    dds_entity *wr;
    dds_return_t ret;
    dds_retcode_t rc;

    DDS_REPORT_STACK();

    if(data == NULL){
        ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Argument data is NULL");
        goto err;
    }
    if(handle == NULL){
        ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Argument data is NULL");
        goto err;
    }
    rc = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
    if (rc != DDS_RETCODE_OK) {
    	ret = DDS_ERRNO(rc, "Error occurred on locking writer");
    	goto err;
    }
        inst = dds_instance_find (((dds_writer*) wr)->m_topic, data, true);
        if(inst != NULL){
            *handle = inst->m_iid;
            ret = DDS_RETCODE_OK;
        } else{
            ret = DDS_ERRNO(DDS_RETCODE_ERROR, "Instance handle has NULL value");
            goto err;
        }
    dds_entity_unlock(wr);
err:
    DDS_REPORT_FLUSH(ret < 0);
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance(
        _In_ dds_entity_t writer,
        _In_opt_ const void *data)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    ret = dds_unregister_instance_ts (writer, data, dds_time());
    DDS_REPORT_FLUSH(ret < 0 );
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance_ih(
       _In_ dds_entity_t writer,
       _In_opt_ dds_instance_handle_t handle)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    ret = dds_unregister_instance_ih_ts(writer, handle, dds_time());
    DDS_REPORT_FLUSH(ret < 0 );
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance_ts(
       _In_ dds_entity_t writer,
       _In_opt_ const void *data,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret = DDS_RETCODE_OK;
    dds_retcode_t rc;
    bool autodispose = true;
    dds_write_action action = DDS_WR_ACTION_UNREGISTER;
    void * sample = (void*) data;
    dds_entity *wr;

    DDS_REPORT_STACK();

    if (data == NULL){
        ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Argument data is NULL");
        goto err;
    }
    if(timestamp < 0){
        ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Argument timestamp has negative value");
        goto err;
    }
    rc = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
    if (rc != DDS_RETCODE_OK) {
        ret =  DDS_ERRNO(rc, "Error occurred on locking writer");
        goto err;
    }

    if (wr->m_qos) {
        dds_qget_writer_data_lifecycle (wr->m_qos, &autodispose);
    }
    if (autodispose) {
        dds_instance_remove (((dds_writer*) wr)->m_topic, data, DDS_HANDLE_NIL);
        action |= DDS_WR_DISPOSE_BIT;
    }
    ret = dds_write_impl ((dds_writer*)wr, sample, timestamp, action);
    dds_entity_unlock(wr);
err:
    DDS_REPORT_FLUSH( ret < 0);
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance_ih_ts(
       _In_ dds_entity_t writer,
       _In_opt_ dds_instance_handle_t handle,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret = DDS_RETCODE_OK;
    dds_retcode_t rc;
    bool autodispose = true;
    dds_write_action action = DDS_WR_ACTION_UNREGISTER;
    dds_entity *wr;

    DDS_REPORT_STACK();

    rc = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
    if (rc != DDS_RETCODE_OK) {
        ret = DDS_ERRNO(rc, "Error occurred on locking writer");
        goto err;
    }

    if (wr->m_qos) {
        dds_qget_writer_data_lifecycle (wr->m_qos, &autodispose);
    }
    if (autodispose) {
        dds_instance_remove (((dds_writer*) wr)->m_topic, NULL, handle);
        action |= DDS_WR_DISPOSE_BIT;
    }

    struct tkmap *map = gv.m_tkmap;
    const dds_topic *topic = dds_instance_info((dds_entity*)wr);
    void *sample = dds_alloc (topic->m_descriptor->m_size);
    if (dds_tkmap_get_key (map, handle, sample)) {
        ret = dds_write_impl ((dds_writer*)wr, sample, timestamp, action);
    } else{
        ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET, "No instance related with the provided handle is found");
    }
    dds_sample_free (sample, topic->m_descriptor, DDS_FREE_ALL);

    dds_entity_unlock(wr);
err:
    DDS_REPORT_FLUSH(ret < 0);
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_writedispose_ts(
       _In_ dds_entity_t writer,
       _In_ const void *data,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    dds_retcode_t rc;
    dds_writer *wr;

    DDS_REPORT_STACK();

    rc = dds_writer_lock(writer, &wr);
    if (rc == DDS_RETCODE_OK) {
        ret = dds_write_impl (wr, data, timestamp, DDS_WR_ACTION_WRITE_DISPOSE);
        if (ret == DDS_RETCODE_OK) {
            dds_instance_remove (wr->m_topic, data, DDS_HANDLE_NIL);
        }
        dds_writer_unlock(wr);
    } else {
        ret = DDS_ERRNO(rc, "Error occurred on locking writer");
    }
    DDS_REPORT_FLUSH(ret < 0);
    return ret;
}

static dds_return_t
dds_dispose_impl(
       _In_ dds_writer *wr,
       _In_ const void *data,
       _In_ dds_instance_handle_t handle,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    assert(wr);
    ret = dds_write_impl(wr, data, timestamp, DDS_WR_ACTION_DISPOSE);
    if (ret == DDS_RETCODE_OK) {
        dds_instance_remove (wr->m_topic, data, handle);
    }
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose_ts(
       _In_ dds_entity_t writer,
       _In_ const void *data,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    dds_retcode_t rc;
    dds_writer *wr;

    DDS_REPORT_STACK();

    rc = dds_writer_lock(writer, &wr);
    if (rc == DDS_RETCODE_OK) {
        ret = dds_dispose_impl(wr, data, DDS_HANDLE_NIL, timestamp);
        dds_writer_unlock(wr);
    } else {
        ret = DDS_ERRNO(rc, "Error occurred on locking writer");
    }
    DDS_REPORT_FLUSH(ret < 0);
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose_ih_ts(
       _In_ dds_entity_t writer,
       _In_ dds_instance_handle_t handle,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    dds_retcode_t rc;
    dds_writer *wr;

    DDS_REPORT_STACK();

    rc = dds_writer_lock(writer, &wr);
    if (rc == DDS_RETCODE_OK) {
        struct tkmap *map = gv.m_tkmap;
        const dds_topic *topic = dds_instance_info((dds_entity*)wr);
        void *sample = dds_alloc (topic->m_descriptor->m_size);
        if (dds_tkmap_get_key (map, handle, sample)) {
            ret = dds_dispose_impl(wr, sample, handle, timestamp);
        } else {
            ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET, "No instance related with the provided handle is found");
        }
        dds_free(sample);
        dds_writer_unlock(wr);
    } else {
        ret = DDS_ERRNO(rc, "Error occurred on locking writer");
    }
    DDS_REPORT_FLUSH(ret < 0);
    return ret;
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
dds_instance_handle_t
dds_instance_lookup(
        dds_entity_t entity,
        const void *data)
{
    dds_instance_handle_t ih = DDS_HANDLE_NIL;
    const dds_topic * topic;
    struct tkmap * map = gv.m_tkmap;
    serdata_t sd;

    assert (data);

    topic = dds_instance_info_by_hdl (entity);
    if (topic) {
        sd = serialize_key (gv.serpool, topic->m_stopic, data);
        ih = dds_tkmap_lookup (map, sd);
        ddsi_serdata_unref (sd);
    }
    return ih;
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
int
dds_instance_get_key(
        dds_entity_t entity,
        dds_instance_handle_t inst,
        void *data)
{
    dds_return_t ret;
    const dds_topic * topic;
    struct tkmap * map = gv.m_tkmap;

    DDS_REPORT_STACK();

    if(data == NULL){
        ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Argument data is NULL");
        goto err;
    }

    topic = dds_instance_info_by_hdl (entity);
    memset (data, 0, topic->m_descriptor->m_size);

    if (dds_tkmap_get_key (map, inst, data)) {
        ret = DDS_RETCODE_OK;
    } else{
        ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET, "No instance related with the provided entity is found");
    }

err:
    DDS_REPORT_FLUSH(ret < 0);
    return ret;
}
