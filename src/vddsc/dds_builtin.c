#include <assert.h>
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_config.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_init.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_domain.h"
#include "kernel/dds_participant.h"
#include "kernel/dds_err.h"
#include "kernel/dds_report.h"
#include "kernel/dds_builtin.h"

static dds_entity_t g_builtin_participant = 0;
static dds_entity_t g_builtin_publisher = 0;

static dds_entity_t g_builtin_participantInfo_wr = 0;
static dds_entity_t g_builtin_participantCMInfo_wr = 0;

#if 0
#define BUILTIN_INFO printf
#else
#define BUILTIN_INFO
#endif

static dds_return_t
dds__builtin_participant_delete(
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
dds__builtin_participant_create(
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
    pp->m_entity.m_deriver.delete = dds__builtin_participant_delete;

fail:
    return participant;
}

void
dds__builtin_participant(
        DDS_ParticipantBuiltinTopicData *data,
        nn_wctime_t timestamp)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    BUILTIN_INFO("---dds__builtin_participant(%x.%x.%x)\n", data->key[0], data->key[1], data->key[2]);
    BUILTIN_INFO("---userdata: %s\n", data->user_data.value._buffer);
    ret = dds_write_ts(g_builtin_participantInfo_wr, data, timestamp.v);
    DDS_REPORT_FLUSH(ret != DDS_RETCODE_OK);
}

void
dds__builtin_cmparticipant(
        DDS_CMParticipantBuiltinTopicData *data,
        nn_wctime_t timestamp)
{
    dds_return_t ret;
    DDS_REPORT_STACK();
    BUILTIN_INFO("---dds__builtin_cmparticipant(%x.%x.%x)\n", data->key[0], data->key[1], data->key[2]);
    BUILTIN_INFO("---product: %s\n", data->product.value);
    ret = dds_write_ts(g_builtin_participantCMInfo_wr, data, timestamp.v);
    DDS_REPORT_FLUSH(ret != DDS_RETCODE_OK);
}

void
dds__builtin_init(
        void)
{
    dds_qos_t *qos;
    dds_entity_t top;

    // TODO: use os 'single init' feature
    if (g_builtin_participant <= 0) {
        nn_plist_t plist;

        // TODO: set builtin qos
        qos = dds_qos_create();

        g_builtin_participant = dds__builtin_participant_create();
        g_builtin_publisher = dds_create_publisher(g_builtin_participant, qos, NULL);

        top = dds_create_topic (g_builtin_participant, &DDS_ParticipantBuiltinTopicData_desc, "DCPSParticipant", qos, NULL);
        g_builtin_participantInfo_wr = dds_create_writer (g_builtin_publisher, top, qos, NULL);
        dds_delete(top);

        top = dds_create_topic (g_builtin_participant, &DDS_CMParticipantBuiltinTopicData_desc, "CMParticipant", qos, NULL);
        g_builtin_participantCMInfo_wr = dds_create_writer (g_builtin_publisher, top, qos, NULL);
        dds_delete(top);

        dds_qos_delete(qos);
    }
}

void
dds__builtin_fini(
        void)
{
    dds_delete(g_builtin_participant);
}
