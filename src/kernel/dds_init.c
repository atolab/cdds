#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "kernel/dds_init.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "kernel/dds_iid.h"
#include "kernel/dds_domain.h"
#include "ddsi/ddsi_ser.h"
#include "os/os.h"
#include "ddsi/q_config.h"
#include "ddsi/q_globals.h"
#include "ddsi/q_servicelease.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"

#ifdef _WRS_KERNEL

#include "os_library.h"
os_dynamicLoad_plugin *os_dynamicLibPlugin = &os_dynamicLibPluginImpl;
char *os_environ[] = { NULL };
struct os_librarySymbols os_staticLibraries[] =
{
  { NULL, (os_entryPoint[]) { {NULL, NULL } } }
};

#endif

struct q_globals gv;

dds_globals dds_global =
{
  DDS_DOMAIN_DEFAULT, OS_ATOMIC_UINT32_INIT (0),
  { OS_ATOMIC_UINT32_INIT (0), OS_ATOMIC_UINT32_INIT (0), OS_ATOMIC_UINT32_INIT (0), OS_ATOMIC_UINT32_INIT (0), OS_ATOMIC_UINT32_INIT (0), OS_ATOMIC_UINT32_INIT (0) },
  NULL, NULL, NULL, NULL
};

static char * dds_init_exe = NULL;
static struct cfgst * dds_cfgst = NULL;

extern void ddsi_impl_init (void);

static int dds_impl_init (void)
{
  os_mutexInit (&gv.attach_lock);
  dds_iid_init ();
  if (dds_global.m_dur_init) (dds_global.m_dur_init) ();
  return 0;
}

static void dds_impl_fini (void)
{
  os_mutexDestroy (&gv.attach_lock);
  if (dds_global.m_dur_fini) (dds_global.m_dur_fini) ();
  dds_iid_fini ();
}

void ddsi_impl_init (void)
{
  /* Register initialization/clean functions */

  ddsi_plugin.init_fn = dds_impl_init;
  ddsi_plugin.fini_fn = dds_impl_fini;

  /* Register read cache functions */

  ddsi_plugin.rhc_free_fn = dds_rhc_free;
  ddsi_plugin.rhc_fini_fn = dds_rhc_fini;
  ddsi_plugin.rhc_store_fn = dds_rhc_store;
  ddsi_plugin.rhc_unregister_wr_fn = dds_rhc_unregister_wr;
  ddsi_plugin.rhc_relinquish_ownership_fn = dds_rhc_relinquish_ownership;
  ddsi_plugin.rhc_set_qos_fn = dds_rhc_set_qos;
  ddsi_plugin.rhc_lookup_fn = dds_tkmap_lookup_instance_ref;
  ddsi_plugin.rhc_unref_fn = dds_tkmap_instance_unref;

  /* Register iid generator */

  ddsi_plugin.iidgen_fn = dds_iid_gen;
}

static void dds_set_report_level (void)
{
  os_reportVerbosity = OS_NONE;
  if (config.enabled_logcats & LC_FATAL)
  {
    os_reportVerbosity = OS_FATAL;
  }
  if (config.enabled_logcats & LC_ERROR)
  {
    os_reportVerbosity = OS_ERROR;
  }
  if (config.enabled_logcats & LC_WARNING)
  {
    os_reportVerbosity = OS_WARNING;
  }
  if (config.enabled_logcats != 0)
  {
    os_reportVerbosity = OS_DEBUG;
  }
}

extern int dds_init (int argc, char ** argv)
{
  const char * uri;

  if (os_atomic_inc32_nv (&dds_global.m_init_count) > 1)
  {
    return DDS_RETCODE_OK;
  }

  uri = os_getenv ("LITE_URI");

  os_osInit ();
  gv.tstart = now ();
  gv.exception = false;

  gv.static_logbuf_lock_inited = 0;
  logbuf_init (&gv.static_logbuf);
  os_mutexInit (&gv.static_logbuf_lock);
  gv.static_logbuf_lock_inited = 1;
  os_mutexInit (&dds_global.m_mutex);

  dds_cfgst = config_init (uri);
  if (dds_cfgst == NULL)
  {
    fprintf (stderr, "Configuration XML file failed to parse\n");
    return DDS_ERRNO (DDS_RETCODE_ERROR, DDS_MOD_KERNEL, DDS_ERR_M1);
  }

  if (! rtps_config_open ())
  {
    fprintf (stderr, "Failed to open log file\n");
    return DDS_ERRNO (DDS_RETCODE_ERROR, DDS_MOD_KERNEL, DDS_ERR_M2);
  }
  dds_set_report_level ();

  if (argc)
  {
    char * tmp = strrchr (argv[0], '/');
    dds_init_exe = dds_string_dup (tmp ? ++tmp : argv[0]);
  }

  return DDS_RETCODE_OK;
}

dds_domainid_t dds_domain_default (void)
{
  return config.domainId;
}

/* Actual initialization called when participant created */

extern int dds_init_impl (dds_domainid_t domain)
{
  char buff[64];
  uint32_t len;

  if (dds_global.m_default_domain != DDS_DOMAIN_DEFAULT)
  {
    if ((dds_global.m_default_domain != domain) && (domain != DDS_DOMAIN_DEFAULT))
    {
      goto fail;
    }
    return DDS_RETCODE_OK;
  }

  /* If domain not set check envirionment variable */

  if (domain == DDS_DOMAIN_DEFAULT)
  {
    char * edom = os_getenv ("LITE_DOMAIN");
    if (edom)
    {
      config.domainId = atoi (edom);
    }
  }
  else
  {
    config.domainId = domain;
  }
  dds_global.m_default_domain = config.domainId;
  if (rtps_config_prep (dds_cfgst) != 0)
  {
    goto fail;
  }
  ut_avlInit (&dds_domaintree_def, &dds_global.m_domains);

  /* Start monitoring the liveliness of all threads and renewing the
     service lease if everything seems well. */

  gv.servicelease = nn_servicelease_new (0, 0);
  assert (gv.servicelease);
  nn_servicelease_start_renewing (gv.servicelease);

  rtps_init ();
  upgrade_main_thread ();

  /* Set additional default participant properties */

#ifdef _WIN32
  gv.default_plist_pp.process_id = (unsigned) GetProcessId (GetCurrentProcess ());
#else
  gv.default_plist_pp.process_id = (unsigned) getpid ();
#endif
  gv.default_plist_pp.present |= PP_PRISMTECH_PROCESS_ID;
  if (dds_init_exe)
  {
    gv.default_plist_pp.exec_name = dds_string_dup (dds_init_exe);
  }
  else
  {
    gv.default_plist_pp.exec_name = dds_string_alloc (32);
    (void) snprintf (gv.default_plist_pp.exec_name, 32, "Lite:%u", gv.default_plist_pp.process_id);
  }
  len = (uint32_t) (13 + strlen (gv.default_plist_pp.exec_name));
  gv.default_plist_pp.present |= PP_PRISMTECH_EXEC_NAME;
  if (os_gethostname (buff, sizeof (buff)) == os_resultSuccess)
  {
    gv.default_plist_pp.node_name = dds_string_dup (buff);
    gv.default_plist_pp.present |= PP_PRISMTECH_NODE_NAME;
  }
  gv.default_plist_pp.entity_name = dds_alloc (len);
  (void) snprintf (gv.default_plist_pp.entity_name, len, "%s<%u>", dds_init_exe ? dds_init_exe : "", gv.default_plist_pp.process_id);
  gv.default_plist_pp.present |= PP_ENTITY_NAME;

  return DDS_RETCODE_OK;

fail:

  return DDS_ERRNO (DDS_RETCODE_ERROR, DDS_MOD_KERNEL, DDS_ERR_M3);
}

extern void dds_fini (void)
{
  if (os_atomic_dec32_nv (&dds_global.m_init_count) == 0)
  {
    if (ddsi_plugin.init_fn)
    {
#ifndef _WIN32
#if VL_BUILD_VALGRIND
      dds_sleepfor (DDS_SECS (2));
#else
      dds_sleepfor (DDS_MSECS (500));
#endif
      rtps_term ();
      nn_servicelease_free (gv.servicelease);
      downgrade_main_thread ();
#if VL_BUILD_VALGRIND
      dds_sleepfor (DDS_MSECS (2));
#endif
      thread_states_fini ();
      config_fini ();
#endif
      os_mutexDestroy (&gv.static_logbuf_lock);
      os_mutexDestroy (&dds_global.m_mutex);
    }
    os_osExit ();
    dds_string_free (dds_init_exe);
    dds_sleepfor (DDS_MSECS (500));
  }
}
