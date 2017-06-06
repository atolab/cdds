#include <assert.h>
#include <string.h>
#include "kernel/dds_topic.h"
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_stream.h"
#include "kernel/dds_init.h"
#include "kernel/dds_domain.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"
#include "ddsi/q_ddsi_discovery.h"
#include "os/os_atomics.h"

#define DDS_TOPIC_STATUS_MASK                                    \
                        DDS_INCONSISTENT_TOPIC_STATUS

const ut_avlTreedef_t dds_topictree_def = UT_AVL_TREEDEF_INITIALIZER_INDKEY
(
  offsetof (struct sertopic, avlnode),
  offsetof (struct sertopic, name_typename),
  (int (*) (const void *, const void *)) strcmp,
  0
);

static dds_return_t dds_topic_status_validate (uint32_t mask)
{
    return (mask & ~(DDS_TOPIC_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_TOPIC, 0) :
                     DDS_RETCODE_OK;
}

/*
  Topic status change callback handler. Supports INCONSISTENT_TOPIC
  status (only defined status on a topic).
*/

static void dds_topic_status_cb (struct dds_topic * topic)
{
    bool call = false;
    void *metrics = NULL;

    os_mutexLock (&topic->m_entity.m_mutex);

    /* Reset the status for possible Listener call.
     * When a listener is not called, the status will be set (again). */
    dds_entity_status_reset((dds_entity_t)topic, DDS_INCONSISTENT_TOPIC_STATUS);

    /* Update status metrics. */
    topic->m_inconsistent_topic_status.total_count++;
    topic->m_inconsistent_topic_status.total_count_change++;

    os_mutexUnlock (&topic->m_entity.m_mutex);

    /* Indicate to the entity hierarchy that we're busy with a callback.
     * This is done from the top to bottom to prevent possible deadlocks.
     * We can't really lock the entities because they have to be possibly
     * accessible from listener callbacks. */
    if (!dds_entity_cb_propagate_begin((dds_entity_t)topic)) {
        /* An entity in the hierarchy is probably being deleted. */
        return;
    }

    /* Is anybody interested within the entity hierarchy through listeners? */
    call = dds_entity_cp_propagate_call((dds_entity_t)topic,
                                        (dds_entity_t)topic,
                                        DDS_INCONSISTENT_TOPIC_STATUS,
                                        (void*)&(topic->m_inconsistent_topic_status),
                                        true);

    /* Let possible waits continue. */
    dds_entity_cb_propagate_end((dds_entity_t)topic);

    if (call) {
        /* Event was eaten by a listener. */
        os_mutexLock (&topic->m_entity.m_mutex);

        /* Reset the change counts of the metrics. */
        topic->m_inconsistent_topic_status.total_count_change = 0;

        os_mutexUnlock (&topic->m_entity.m_mutex);
    } else {
        /* Nobody was interested through a listener. Set the status to maybe force a trigger. */
        dds_entity_status_set((dds_entity_t)topic, DDS_INCONSISTENT_TOPIC_STATUS);
        dds_entity_status_signal((dds_entity_t)topic);
    }
}

sertopic_t dds_topic_lookup (dds_domain * domain, const char * name)
{
  sertopic_t st = NULL;
  ut_avlIter_t iter;

  assert (domain);
  assert (name);

  os_mutexLock (&dds_global.m_mutex);
  st = ut_avlIterFirst (&dds_topictree_def, &domain->m_topics, &iter);
  while (st)
  {
    if (strcmp (st->name, name) == 0)
    {
      break;
    }
    st = ut_avlIterNext (&iter);
  }
  os_mutexUnlock (&dds_global.m_mutex);
  return st;
}

void dds_topic_free (dds_domainid_t domainid, struct sertopic * st)
{
  dds_domain *domain;

  assert (st);

  os_mutexLock (&dds_global.m_mutex);
  domain = (dds_domain*) ut_avlLookup (&dds_domaintree_def, &dds_global.m_domains, &domainid);
  if (domain != NULL)
  {
    ut_avlDelete (&dds_topictree_def, &domain->m_topics, st);
  }
  os_mutexUnlock (&dds_global.m_mutex);
  st->status_cb_entity = NULL;
  sertopic_free (st);
}

static void dds_topic_add (dds_domainid_t id, sertopic_t st)
{
  dds_domain * dom;
  os_mutexLock (&dds_global.m_mutex);
  dom = dds_domain_find_locked (id);
  assert (dom);
  ut_avlInsert (&dds_topictree_def, &dom->m_topics, st);
  os_mutexUnlock (&dds_global.m_mutex);
}

dds_entity_t dds_topic_find (dds_entity_t pp, const char * name)
{
  dds_entity_t tp = NULL;
  sertopic_t st;

  assert (pp->m_kind == DDS_TYPE_PARTICIPANT);

  st = dds_topic_lookup (pp->m_domain, name);
  if (st)
  {
    tp = &st->status_cb_entity->m_entity;
    dds_entity_add_ref (tp);
  }

  return tp;
}

static void dds_topic_delete(dds_entity_t e, bool recurse)
{
    dds_topic_free(e->m_domainid, ((dds_topic*) e)->m_stopic);
}

static dds_return_t dds_topic_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_TOPIC, 0);
    bool consistent = true;
    assert(qos);
    /* Check consistency. */
    consistent &= dds_qos_validate_common(qos);
    consistent &= (qos->present & QP_TOPIC_DATA)         ? validate_octetseq (&qos->topic_data) : true;
    consistent &= (qos->present & QP_DURABILITY_SERVICE) ? validate_durability_service_qospolicy (&qos->durability_service) == 0 : true;
    consistent &= (qos->present & QP_LIFESPAN)           ? validate_duration (&qos->lifespan.duration) == 0 : true;
    consistent &= (qos->present & QP_HISTORY)
               && (qos->present & QP_RESOURCE_LIMITS)    ? validate_history_and_resource_limits (&qos->history, &qos->resource_limits) == 0 : true;

    if (consistent) {
        ret = DDS_RETCODE_OK;
        if (enabled) {
            /* TODO: Improve/check immutable check. */
            if (qos->present != QP_LATENCY_BUDGET) {
                ret = DDS_ERRNO(DDS_RETCODE_IMMUTABLE_POLICY, DDS_MOD_TOPIC, DDS_ERR_M1);
            }
        }
    }
    return ret;
}


static dds_return_t dds_topic_qos_set (dds_entity_t e, const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = dds_topic_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_TOPIC, DDS_ERR_M1));
        }
    }
    return ret;
}

int dds_topic_create
(
  dds_entity_t pp,
  dds_entity_t * topic,
  const dds_topic_descriptor_t * desc,
  const char * name,
  const dds_qos_t * qos,
  const dds_listener_t * listener
)
{
  static uint32_t next_topicid = 0;

  char * key = NULL;
  sertopic_t st;
  const char * typename;
  int ret = DDS_RETCODE_OK;
  dds_topic * top;
  dds_qos_t * new_qos = NULL;
  nn_plist_t plist;
  struct participant * ddsi_pp;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  uint32_t index;

  assert (pp);
  assert (topic);
  assert (desc);
  assert (name);
  assert (pp->m_kind == DDS_TYPE_PARTICIPANT);

  os_mutexLock (&pp->m_mutex);

  /* Validate qos */

  if (qos)
  {
    ret = (int)dds_topic_qos_validate (qos, false);
    if (ret != DDS_RETCODE_OK)
    {
      goto fail;
    }
  }

  /* Check if topic already exists with same name */

  if (dds_topic_lookup (pp->m_domain, name))
  {
    ret = DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER, DDS_MOD_KERNEL, DDS_ERR_M1);
    goto fail;
  }

  typename = desc->m_typename;
  key = (char*) dds_alloc (strlen (name) + strlen (typename) + 2);
  strcpy (key, name);
  strcat (key, "/");
  strcat (key, typename);

  if (qos)
  {
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
  }

  /* Create topic */

  top = dds_alloc (sizeof (*top));
  top->m_descriptor = desc;
  dds_entity_init (&top->m_entity, pp, DDS_TYPE_TOPIC, new_qos, listener, DDS_TOPIC_STATUS_MASK);
  *topic = &top->m_entity;
  top->m_entity.m_deriver.delete = dds_topic_delete;
  top->m_entity.m_deriver.set_qos = dds_topic_qos_set;
  top->m_entity.m_deriver.validate_status = dds_topic_status_validate;

  st = dds_alloc (sizeof (*st));
  st->type = (void*) desc;
  os_atomic_st32 (&st->refcount, 1);
  st->status_cb = dds_topic_status_cb;
  st->status_cb_entity = top;
  st->name_typename = key;
  st->name = dds_alloc (strlen (name) + 1);
  strcpy (st->name, name);
  st->typename = dds_alloc (strlen (typename) + 1);
  strcpy (st->typename, typename);
  st->nkeys = desc->m_nkeys;
  st->keys = desc->m_keys;
  st->id = next_topicid++;

#ifdef VXWORKS_RTP
  st->hash = (st->id * UINT64_C (12844332200329132887UL)) >> 32;
#else
  st->hash = (st->id * UINT64_C (12844332200329132887)) >> 32;
#endif

  /* Check if topic cannot be optimised (memcpy marshal) */

  if ((desc->m_flagset & DDS_TOPIC_NO_OPTIMIZE) == 0)
  {
    st->opt_size = dds_stream_check_optimize (desc);
  }
  top->m_stopic = st;

  /* Add topic to extent */

  dds_topic_add (pp->m_domainid, st);

  nn_plist_init_empty (&plist);
  if (new_qos)
  {
    dds_qos_merge (&plist.qos, new_qos);
  }

  /* Set Topic meta data (for SEDP publication) */

  plist.qos.topic_name = dds_string_dup (st->name);
  plist.qos.type_name = dds_string_dup (st->typename);
  plist.qos.present |= (QP_TOPIC_NAME | QP_TYPE_NAME);
  if (desc->m_meta)
  {
    plist.type_description = dds_string_dup (desc->m_meta);
    plist.present |= PP_PRISMTECH_TYPE_DESCRIPTION;
  }
  if (desc->m_nkeys)
  {
    plist.qos.present |= QP_PRISMTECH_SUBSCRIPTION_KEYS;
    plist.qos.subscription_keys.use_key_list = 1;
    plist.qos.subscription_keys.key_list.n = desc->m_nkeys;
    plist.qos.subscription_keys.key_list.strs = dds_alloc (desc->m_nkeys * sizeof (char*));
    for (index = 0; index < desc->m_nkeys; index++)
    {
      plist.qos.subscription_keys.key_list.strs[index] = dds_string_dup (desc->m_keys[index].m_name);
    }
  }

  /* Publish Topic */

  if (asleep)
  {
    thread_state_awake (thr);
  }
  ddsi_pp = ephash_lookup_participant_guid (&pp->m_guid);
  assert (ddsi_pp);
  sedp_write_topic (ddsi_pp, &plist);
  if (asleep)
  {
    thread_state_asleep (thr);
  }
  nn_plist_fini (&plist);

fail:

  os_mutexUnlock (&pp->m_mutex);
  return ret;
}

static bool dds_topic_chaining_filter (const void *sample, void *ctx)
{
  dds_topic_filter_fn realf = (dds_topic_filter_fn)ctx;
  return realf (sample);
}

static void dds_topic_mod_filter
(
  dds_topic * topic,
  dds_topic_intern_filter_fn * filter,
  void ** ctx,
  bool set
)
{
  assert (topic);
  assert (topic->m_entity.m_kind == DDS_TYPE_TOPIC);

  os_mutexLock (&topic->m_entity.m_mutex);
  if (set)
  {
    topic->m_stopic->filter_fn = *filter;
    topic->m_stopic->filter_ctx = *ctx;

    /* Create sample for read filtering */

    if (topic->m_stopic->filter_sample == NULL)
    {
      topic->m_stopic->filter_sample = dds_alloc (topic->m_descriptor->m_size);
    }
  }
  else
  {
    *filter = topic->m_stopic->filter_fn;
    *ctx = topic->m_stopic->filter_ctx;
  }
  os_mutexUnlock (&topic->m_entity.m_mutex);
}

void dds_topic_set_filter (dds_entity_t topic, dds_topic_filter_fn filter)
{
  dds_topic_intern_filter_fn chaining = dds_topic_chaining_filter;
  void *realf = (void *)filter;
  dds_topic_mod_filter ((dds_topic*) topic, &chaining, &realf, true);
}

dds_topic_filter_fn dds_topic_get_filter (dds_entity_t topic)
{
  dds_topic_intern_filter_fn filter;
  void *ctx;
  dds_topic_mod_filter ((dds_topic*) topic, &filter, &ctx, false);
  return
    (filter == dds_topic_chaining_filter) ? (dds_topic_filter_fn)ctx : NULL;
}

void dds_topic_set_filter_with_ctx
  (dds_entity_t topic, dds_topic_intern_filter_fn filter, void *ctx)
{
  dds_topic_mod_filter ((dds_topic*) topic, &filter, &ctx, true);
}

dds_topic_intern_filter_fn dds_topic_get_filter_with_ctx (dds_entity_t topic)
{
  dds_topic_intern_filter_fn filter;
  void *ctx;
  dds_topic_mod_filter ((dds_topic*) topic, &filter, &ctx, false);
  return (filter == dds_topic_chaining_filter) ? NULL : filter;
}

char * dds_topic_get_name (dds_entity_t topic)
{
  assert (topic);
  assert (topic->m_kind == DDS_TYPE_TOPIC);
  return dds_string_dup (((dds_topic*) topic)->m_stopic->name);
}

char * dds_topic_get_type_name (dds_entity_t topic)
{
  assert (topic);
  assert (topic->m_kind == DDS_TYPE_TOPIC);
  return dds_string_dup (((dds_topic*) topic)->m_stopic->typename);
}

dds_return_t dds_get_inconsistent_topic_status (dds_entity_t entity, dds_inconsistent_topic_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_TOPIC, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_TOPIC) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_TOPIC, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_INCONSISTENT_TOPIC_STATUS) {
            dds_topic *tp = (dds_topic*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = tp->m_inconsistent_topic_status;
            }
            tp->m_inconsistent_topic_status.total_count_change = 0;
            dds_entity_status_reset(entity, DDS_INCONSISTENT_TOPIC_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}
