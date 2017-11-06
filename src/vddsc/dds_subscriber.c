#include <string.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_err.h"
#include "ddsi/q_entity.h"
#include "kernel/dds_report.h"

#define DDS_SUBSCRIBER_STATUS_MASK                               \
                        DDS_DATA_ON_READERS_STATUS

static dds_return_t
dds_subscriber_instance_hdl(
        dds_entity *e,
        dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    /* TODO: Get/generate proper handle. */
    return DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "Generating subscriber instance handle is not supported");
}

static dds_return_t
dds__subscriber_qos_validate(
        _In_ const dds_qos_t *qos,
        _In_ bool enabled)
{
    dds_return_t ret = DDS_RETCODE_OK;

    assert(qos);

    if((qos->present & QP_GROUP_DATA) && !validate_octetseq(&qos->group_data)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Group data policy is inconsistent and caused an error");
    }
    if((qos->present & QP_PARTITION) && !validate_stringseq(&qos->partition)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Partition policy is inconsistent and caused an error");
    }
    if((qos->present & QP_PRESENTATION) && validate_presentation_qospolicy(&qos->presentation)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Presentation policy is inconsistent and caused an error");
    }
    if((qos->present & QP_PRISMTECH_ENTITY_FACTORY) && !validate_entityfactory_qospolicy(&qos->entity_factory)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Prismtech entity factory policy is inconsistent and caused an error");
    }
    if(ret == DDS_RETCODE_OK && enabled && (qos->present & QP_PRESENTATION)) {
        /* TODO: Improve/check immutable check. */
        ret = DDS_ERRNO(DDS_RETCODE_IMMUTABLE_POLICY, "Presentation QoS policy is immutable");
    }

    return ret;
}

static dds_return_t
dds_subscriber_qos_set(
        OS_UNUSED_PAR(dds_entity *e),
        const dds_qos_t *qos,
        bool enabled)
{
    dds_return_t ret = dds__subscriber_qos_validate(qos, enabled);

    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "VortexDDS does not support changing QoS policies yet");
        }
    }
    return ret;
}

static dds_return_t
dds_subscriber_status_validate(
        uint32_t mask)
{
    return (mask & ~(DDS_SUBSCRIBER_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Invalid status mask") :
                     DDS_RETCODE_OK;
}

/*
  Set boolean on readers that indicates state of DATA_ON_READERS
  status on parent subscriber
*/
static dds_return_t
dds_subscriber_status_propagate(
        dds_entity *sub,
        uint32_t mask,
        bool set)
{
    if (mask & DDS_DATA_ON_READERS_STATUS) {
        dds_entity *iter = sub->m_children;
        while (iter) {
            os_mutexLock (&iter->m_mutex);
            ((dds_reader*) iter)->m_data_on_readers = set;
            os_mutexUnlock (&iter->m_mutex);
            iter = iter->m_next;
        }
    }
    return DDS_RETCODE_OK;
}

_Requires_exclusive_lock_held_(participant)
_Check_return_
static dds_entity_t
dds__create_subscriber_l(
        _Inout_  dds_entity *participant, /* entity-lock must be held */
        _In_opt_ const dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener)
{
    dds_subscriber * sub;
    dds_entity_t subscriber;
    dds_return_t ret;
    dds_qos_t * new_qos;

    /* Validate qos */
    if (qos) {
        if ((ret = dds__subscriber_qos_validate(qos, false)) != DDS_RETCODE_OK) {
            goto err_param;
        }
        new_qos = dds_qos_create();
        /* Only returns failure when one of the qos args is NULL, which
         * is not the case here. */
        (void)dds_qos_copy(new_qos, qos);
    } else {
        new_qos = NULL;
    }

    /* Create subscriber */
    sub = dds_alloc(sizeof(*sub));
    subscriber = dds_entity_init(&sub->m_entity, participant, DDS_KIND_SUBSCRIBER, new_qos, listener, DDS_SUBSCRIBER_STATUS_MASK);
    sub->m_entity.m_deriver.set_qos = dds_subscriber_qos_set;
    sub->m_entity.m_deriver.validate_status = dds_subscriber_status_validate;
    sub->m_entity.m_deriver.propagate_status = dds_subscriber_status_propagate;
    sub->m_entity.m_deriver.get_instance_hdl = dds_subscriber_instance_hdl;

    return subscriber;

    /* Error handling */
err_param:
    return ret;
}

_Pre_satisfies_((participant & DDS_ENTITY_KIND_MASK) == DDS_KIND_PARTICIPANT)
_Must_inspect_result_ dds_entity_t
dds_create_subscriber(
        _In_     dds_entity_t participant,
        _In_opt_ const dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener)
{
    dds_entity * par;
    dds_entity_t hdl;
    dds__retcode_t errnr;

    DDS_REPORT_STACK();

    errnr = dds_entity_lock(participant, DDS_KIND_PARTICIPANT, &par);
    if (errnr != DDS_RETCODE_OK) {
        hdl = DDS_ERRNO(errnr, "Error occurred on locking participant");
        return hdl;
    }

    hdl = dds__create_subscriber_l(par, qos, listener);
    dds_entity_unlock(par);

    DDS_REPORT_FLUSH(hdl <= 0);
    return hdl;
}

_Pre_satisfies_((subscriber & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER)
dds_return_t
dds_notify_readers(
        _In_ dds_entity_t subscriber)
{
    dds_entity *iter;
    dds_entity *sub;
    dds__retcode_t errnr;
    dds_return_t ret;

    DDS_REPORT_STACK();

    errnr = dds_entity_lock(subscriber, DDS_KIND_SUBSCRIBER, &sub);
    if (errnr == DDS_RETCODE_OK) {
        errnr = DDS_RETCODE_UNSUPPORTED;
        ret = DDS_ERRNO(errnr, "Unsupported operation");
        iter = sub->m_children;
        while (iter) {
            os_mutexLock(&iter->m_mutex);
            // TODO: check if reader has data available, call listener
            os_mutexUnlock(&iter->m_mutex);
            iter = iter->m_next;
        }
        dds_entity_unlock(sub);
    } else {
        ret = DDS_ERRNO(errnr, "Error occurred on locking subscriber");
    }

    DDS_REPORT_FLUSH(ret != DDS_RETCODE_OK);
    return ret;
}

dds_return_t
dds_subscriber_begin_coherent(
        _In_ dds_entity_t e)
{
    /* TODO: CHAM-124 Currently unsupported. */
    OS_UNUSED_ARG(e);
    return DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "Using coherency to get a coherent data set is not currently being supported");
}

dds_return_t
dds_subscriber_end_coherent(
        _In_ dds_entity_t e)
{
    /* TODO: CHAM-124 Currently unsupported. */
    OS_UNUSED_ARG(e);
    return DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "Using coherency to get a coherent data set is not currently being supported");
}

_Pre_satisfies_((subscriber & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER)
_Must_inspect_result_ dds_entity_t
dds__get_builtin_topic(
    _In_ dds_entity_t subscriber,
    _In_ dds_entity_t topic)
{
    dds_entity_t participant;
    dds_entity_t ret;

    participant = dds_get_participant(subscriber);
    if (participant > 0) {
        const dds_topic_descriptor_t *desc;
        const char *name;

        if (topic == DDS_BUILTIN_TOPIC_DCPSPARTICIPANT) {
            desc = &DDS_ParticipantBuiltinTopicData_desc;
            name = "DCPSParticipant";
        } else if (topic == DDS_BUILTIN_TOPIC_CMPARTICIPANT) {
            desc = &DDS_CMParticipantBuiltinTopicData_desc;
            name = "CMParticipant";
        } else if (topic == DDS_BUILTIN_TOPIC_DCPSTYPE) {
            desc = &DDS_TypeBuiltinTopicData_desc;
            name = "DCPSType";
        } else if (topic == DDS_BUILTIN_TOPIC_DCPSTOPIC) {
            desc = &DDS_TopicBuiltinTopicData_desc;
            name = "DCPSTopic";
        } else if (topic == DDS_BUILTIN_TOPIC_DCPSPUBLICATION) {
            desc = &DDS_PublicationBuiltinTopicData_desc;
            name = "DCPSPublication";
        } else if (topic == DDS_BUILTIN_TOPIC_CMPUBLISHER) {
            desc = &DDS_CMPublisherBuiltinTopicData_desc;
            name = "CMPublisher";
        } else if (topic == DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION) {
            desc = &DDS_SubscriptionBuiltinTopicData_desc;
            name = "DCPSSubscription";
        } else if (topic == DDS_BUILTIN_TOPIC_CMSUBSCRIBER) {
            desc = &DDS_CMSubscriberBuiltinTopicData_desc;
            name = "CMSubscriber";
        } else if (topic == DDS_BUILTIN_TOPIC_CMDATAWRITER) {
            desc = &DDS_CMDataWriterBuiltinTopicData_desc;
            name = "CMDataWriter";
        } else if (topic == DDS_BUILTIN_TOPIC_CMDATAREADER) {
            desc = &DDS_CMDataReaderBuiltinTopicData_desc;
            name = "CMDataReader";
        } else {
            ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Invalid builtin-topic handle(%d)", topic);
            goto err_invalid_topic;
        }

        ret = dds_find_topic(participant, name);
        if (ret < 0 && dds_err_nr(ret) == DDS_RETCODE_PRECONDITION_NOT_MET) {
            DDS_REPORT_FLUSH(0);
            DDS_REPORT_STACK();
            /* TODO get QoS from subscriber */
            ret = dds_create_topic(participant, desc, name, NULL, NULL);
        }

    } else {
        /* Failed to get participant of provided entity */
        ret = participant;
    }

err_invalid_topic:
    return ret;
}


_Check_return_ dds_entity_t
dds__get_builtin_subscriber(
    _In_ dds_entity_t e)
{
    dds_return_t ret;
    dds_entity_t participant;
    dds_participant *p;
    dds_entity *part_entity;

    if ((participant = dds_get_participant(e)) <= 0) {
        /* error already in participant error; no need to repeat error */
        ret = participant;
        goto error;
    }
    if ((ret = dds_entity_lock(participant, DDS_KIND_PARTICIPANT, (dds_entity **)&part_entity)) != DDS_RETCODE_OK) {
        goto error;
    }
    p = (dds_participant *)part_entity;
    if(p->m_builtin_subscriber <= 0) {
        dds_qos_t *sqos;
        const char *partition = "__BUILT-IN PARTITION__";

        sqos = dds_qos_create();
        dds_qset_partition(sqos, 1, &partition);

        /* Create builtin-subscriber */
        p->m_builtin_subscriber = dds__create_subscriber_l(part_entity, sqos, NULL);
        dds_qos_delete(sqos);
    }
    dds_entity_unlock(part_entity);

    return p->m_builtin_subscriber;

    /* Error handling */
error:
    assert(ret < 0);
    return ret;
}
