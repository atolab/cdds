#include <assert.h>
#include <string.h>
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_config.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_init.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_domain.h"
#include "kernel/dds_participant.h"
#include "kernel/dds_err.h"
#include "kernel/dds_types.h"
#include "kernel/dds_report.h"
#include "kernel/dds_builtin.h"
#include "kernel/dds_subscriber.h"


#if 0
#define BUILTIN_INFO printf
#else
#define BUILTIN_INFO
#endif

static dds_return_t
dds__delete_builtin_participant(
        dds_entity *e);

static dds_entity_t
dds__create_builtin_participant(
        void);

static _Must_inspect_result_ dds_entity_t
dds__create_builtin_publisher(
    _In_ dds_entity_t participant);

static _Must_inspect_result_ dds_entity_t
dds__create_builtin_writer(
    _In_ dds_entity_t topic);

static _Must_inspect_result_ dds_entity_t
dds__get_builtin_participant(
    void);




static os_mutex g_builtin_mutex;

/* Singletons are used to publish builtin data locally. */
static dds_entity_t g_builtin_local_participant = 0;
static dds_entity_t g_builtin_local_publisher = 0;
static dds_entity_t g_builtin_local_writers[] = {
        0, /* index DDS_BUILTIN_TOPIC_DCPSPARTICIPANT  - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_CMPARTICIPANT    - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_DCPSTYPE         - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_DCPSTOPIC        - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_DCPSPUBLICATION  - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_CMPUBLISHER      - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_CMSUBSCRIBER     - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_CMDATAWRITER     - DDS_KIND_INTERNAL - 1 */
        0, /* index DDS_BUILTIN_TOPIC_CMDATAREADER     - DDS_KIND_INTERNAL - 1 */
};


static dds_return_t
dds__delete_builtin_participant(
        dds_entity *e)
{
    struct thread_state1 * const thr = lookup_thread_state ();
    const bool asleep = !vtime_awake_p (thr->vtime);

    assert(e);
    assert(thr);
    assert(dds_entity_kind(e->m_hdl) == DDS_KIND_PARTICIPANT);

    if (asleep) {
      thread_state_awake(thr);
    }
    dds_domain_free(e->m_domain);
    if (asleep) {
      thread_state_asleep(thr);
    }

    return DDS_RETCODE_OK;
}

/*
 * We don't use the 'normal' create participant.
 *
 * This way, the application is not able to access the local builtin writers.
 * Also, we can indicate that it should be a 'local only' participant, which
 * means that none of the entities under the hierarchy of this participant will
 * be exposed to the outside world. This is what we want, because these builtin
 * writers are only applicable to local user readers.
 */
static dds_entity_t
dds__create_builtin_participant(
        void)
{
    int q_rc;
    nn_plist_t plist;
    struct thread_state1 * thr;
    bool asleep;
    nn_guid_t guid;
    dds_entity_t participant;
    dds_participant *pp;

    nn_plist_init_empty (&plist);

    thr = lookup_thread_state ();
    asleep = !vtime_awake_p (thr->vtime);
    if (asleep) {
        thread_state_awake (thr);
    }
    q_rc = new_participant (&guid, RTPS_PF_NO_BUILTIN_WRITERS | RTPS_PF_NO_BUILTIN_READERS | RTPS_PF_ONLY_LOCAL, &plist);
    if (asleep) {
        thread_state_asleep (thr);
    }

    if (q_rc != 0) {
        participant = DDS_ERRNO(DDS_RETCODE_ERROR, "Internal builtin error");
        goto fail;
    }

    pp = dds_alloc (sizeof (*pp));
    participant = dds_entity_init (&pp->m_entity, NULL, DDS_KIND_PARTICIPANT, NULL, NULL, 0);
    if (participant < 0) {
        goto fail;
    }

    pp->m_entity.m_guid = guid;
    pp->m_entity.m_domain = dds_domain_create (config.domainId);
    pp->m_entity.m_domainid = config.domainId;
    pp->m_entity.m_deriver.delete = dds__delete_builtin_participant;

fail:
    return participant;
}

static _Must_inspect_result_ dds_entity_t
dds__create_builtin_publisher(
    _In_ dds_entity_t participant)
{
    dds_entity_t pub;
    dds_qos_t *qos;
    const char *partition = "__BUILT-IN PARTITION__";

    qos = dds_qos_create();
    dds_qset_partition(qos, 1, &partition);

    pub = dds_create_publisher(participant, qos, NULL);

    dds_qos_delete(qos);

    return pub;
}

static _Must_inspect_result_ dds_entity_t
dds__create_builtin_subscriber(
    _In_ dds_entity *participant)
{
    dds_entity_t sub;
    dds_qos_t *qos;
    const char *partition = "__BUILT-IN PARTITION__";

    qos = dds_qos_create();
    dds_qset_partition(qos, 1, &partition);

    /* Create builtin-subscriber */
    sub = dds__create_subscriber_l(participant, qos, NULL);
    dds_qos_delete(qos);

    return sub;
}

static _Must_inspect_result_ dds_entity_t
dds__create_builtin_writer(
    _In_ dds_entity_t topic)
{
    dds_entity_t wr;
    dds_entity_t pub = dds__get_builtin_publisher();
    if (pub > 0) {
        dds_entity_t top = dds__get_builtin_topic(pub, topic);
        if (top > 0) {
            dds_qos_t *qos;
            // TODO: set builtin qos
            qos = dds_qos_create();
            dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
            dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS(100));
            wr = dds_create_writer(pub, top, qos, NULL);
            dds_qos_delete(qos);
        } else {
            wr = top;
        }
    } else {
        wr = pub;
    }
    return wr;
}


static _Must_inspect_result_ dds_entity_t
dds__get_builtin_participant(
    void)
{
    if (g_builtin_local_participant == 0) {
        g_builtin_local_participant = dds__create_builtin_participant();
    }
    return g_builtin_local_participant;
}


_Must_inspect_result_ dds_entity_t
dds__get_builtin_publisher(
    void)
{
    if (g_builtin_local_publisher == 0) {
        dds_entity_t par = dds__get_builtin_participant();
        if (par > 0) {
            g_builtin_local_publisher = dds__create_builtin_publisher(par);
        }
    }
    return g_builtin_local_publisher;
}

_Check_return_ dds_entity_t
dds__get_builtin_subscriber(
    _In_ dds_entity_t e)
{
    dds_entity_t sub;
    dds_return_t ret;
    dds_entity_t participant;
    dds_participant *p;
    dds_entity *part_entity;

    participant = dds_get_participant(e);
    if (participant <= 0) {
        /* error already in participant error; no need to repeat error */
        ret = participant;
        goto error;
    }
    ret = dds_entity_lock(participant, DDS_KIND_PARTICIPANT, (dds_entity **)&part_entity);
    if (ret != DDS_RETCODE_OK) {
        goto error;
    }
    p = (dds_participant *)part_entity;
    if(p->m_builtin_subscriber <= 0) {
        p->m_builtin_subscriber = dds__create_builtin_subscriber(part_entity);
    }
    sub = p->m_builtin_subscriber;
    dds_entity_unlock(part_entity);

    return sub;

    /* Error handling */
error:
    assert(ret < 0);
    return ret;
}


_Must_inspect_result_ dds_entity_t
dds__get_builtin_topic(
    _In_ dds_entity_t e,
    _In_ dds_entity_t topic)
{
    dds_entity_t participant;
    dds_entity_t ret;

    participant = dds_get_participant(e);
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
            dds_qos_t *qos;
            // TODO: set builtin qos
            qos = dds_qos_create();
            dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
            dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS(100));

            DDS_REPORT_FLUSH(0);
            DDS_REPORT_STACK();
            /* TODO get QoS from subscriber */
            ret = dds_create_topic(participant, desc, name, qos, NULL);
            dds_qos_delete(qos);
        }

    } else {
        /* Failed to get participant of provided entity */
        ret = participant;
    }

err_invalid_topic:
    return ret;
}


static _Must_inspect_result_ dds_entity_t
dds__get_builtin_writer(
    _In_ dds_entity_t topic)
{
    dds_entity_t wr;
    if ((topic >= DDS_BUILTIN_TOPIC_DCPSPARTICIPANT) && (topic <= DDS_BUILTIN_TOPIC_CMDATAREADER)) {
        int index = (int)(topic - DDS_KIND_INTERNAL - 1);
        os_mutexLock(&g_builtin_mutex);
        wr = g_builtin_local_writers[index];
        if (wr == 0) {
            wr = dds__create_builtin_writer(topic);
            if (wr > 0) {
                g_builtin_local_writers[index] = wr;
            }
        }
        os_mutexUnlock(&g_builtin_mutex);
    } else {
        wr = DDS_ERRNO(DDS_RETCODE_ERROR, "Given topic is not a builtin topic.");
    }
    return wr;
}

static dds_return_t
dds__builtin_write(_In_ dds_entity_t topic, void * data, _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    dds_entity_t wr;
    wr = dds__get_builtin_writer(topic);
    if (wr > 0) {
        BUILTIN_INFO("---write by: %x on %ld (%lx)\n", wr, timestamp, timestamp);
        ret = dds_write_ts(wr, data, timestamp);
    } else {
        ret = wr;
    }
    return ret;
}

void
dds__builtin_init(
        void)
{
    os_mutexInit(&g_builtin_mutex);
}

void
dds__builtin_fini(
        void)
{
    dds_delete(g_builtin_local_participant);
    g_builtin_local_participant = 0;
    g_builtin_local_publisher = 0;
    memset(g_builtin_local_writers, 0, sizeof(g_builtin_local_writers));
    os_mutexDestroy(&g_builtin_mutex);
}


void
dds__builtin_participant_cb(
        DDS_ParticipantBuiltinTopicData *data,
        nn_wctime_t timestamp)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    BUILTIN_INFO("---dds__builtin_participant_cb(%x.%x.%x)\n", data->key[0], data->key[1], data->key[2]);
    BUILTIN_INFO("---userdata: %s\n", data->user_data.value._buffer);
    ret = dds__builtin_write(DDS_BUILTIN_TOPIC_DCPSPARTICIPANT, data, timestamp.v);
    DDS_REPORT_FLUSH(ret != DDS_RETCODE_OK);
}

void
dds__builtin_cmparticipant_cb(
        DDS_CMParticipantBuiltinTopicData *data,
        nn_wctime_t timestamp)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    BUILTIN_INFO("---dds__builtin_cmparticipant_cb(%x.%x.%x)\n", data->key[0], data->key[1], data->key[2]);
    BUILTIN_INFO("---product: %s\n", data->product.value);
    ret = dds__builtin_write(DDS_BUILTIN_TOPIC_CMPARTICIPANT, data, timestamp.v);
    DDS_REPORT_FLUSH(ret != DDS_RETCODE_OK);
}
