#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ddsi/q_config.h>
#include <os/os.h>
#include <os/os_report.h>
#include "kernel/dds_init.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "kernel/dds_iid.h"
#include "kernel/dds_domain.h"
#include "kernel/dds_err.h"
#include "kernel/dds_builtin.h"
#include "ddsi/ddsi_ser.h"
#include "ddsi/q_servicelease.h"
#include "ddsi/q_entity.h"
#include "vddsc/vddsc_project.h"
#include "kernel/dds_report.h"

#ifdef _WRS_KERNEL
char *os_environ[] = { NULL };
#endif

#define DOMAIN_ID_MIN 0
#define DOMAIN_ID_MAX 230

struct q_globals gv;

dds_globals dds_global =
{
  DDS_DOMAIN_DEFAULT, OS_ATOMIC_UINT32_INIT (0),
  NULL, NULL, NULL, NULL
};

static char * dds_init_exe = NULL;
static struct cfgst * dds_cfgst = NULL;
bool is_initialized=false;

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

static os_mutex  dds_init_mutex;
static os_once_t dds_init_control = OS_ONCE_T_STATIC_INIT;

void
dds_fini_once(void)
{
  os_mutexDestroy(&dds_init_mutex);
  os_osExit();
}

void
dds_init_once(void)
{
  os_osInit();
  os_procAtExit(dds_fini_once);
  os_mutexInit(&dds_init_mutex);
}

dds_return_t
dds_init(void)
{
  const char * uri;
  char tmp[50];
  int default_domain;

  os_once(&dds_init_control, dds_init_once);

  os_mutexLock(&dds_init_mutex);

  /* TODO: Proper init-once */
  if (os_atomic_inc32_nv (&dds_global.m_init_count) > 1)
  {
    os_mutexUnlock(&dds_init_mutex);
    return DDS_RETCODE_OK;
  }

  gv.tstart = now ();
  gv.exception = false;

  gv.static_logbuf_lock_inited = 0;
  logbuf_init (&gv.static_logbuf);
  os_mutexInit (&gv.static_logbuf_lock);
  gv.static_logbuf_lock_inited = 1;
  os_mutexInit (&dds_global.m_mutex);

  if (ut_handleserver_init() != UT_HANDLE_OK)
  {
      os_mutexUnlock(&dds_init_mutex);
      return DDS_ERRNO(DDS_RETCODE_ERROR, "Failed to initialize internal handle server");
  }

  uri = os_getenv (VDDSC_PROJECT_NAME_NOSPACE_CAPS"_URI");
  dds_cfgst = config_init (uri);
  if (dds_cfgst == NULL)
  {
    os_mutexUnlock(&dds_init_mutex);
    return DDS_ERRNO(DDS_RETCODE_ERROR, "Failed to parse configuration XML file %s", uri);
  }

  os_procName(tmp, sizeof(tmp));
  dds_init_exe = dds_string_dup (tmp);

  dds__builtin_init();

  //check consistency of configuration file and environment variable. And store the default domain ID to dds_global.m_default_domain
  default_domain = dds_domain_default();
  if( default_domain == DDS_DOMAIN_DEFAULT) { //exact value should be a valid domain ID
      os_mutexUnlock(&dds_init_mutex);
      return DDS_ERRNO(DDS_RETCODE_ERROR,
                      "DDS Init Error: Failed to configure domain id");
  }

  os_mutexUnlock(&dds_init_mutex);
  return DDS_RETCODE_OK;
}

//provides default domain id. calculates and stores at a global variable on first call
dds_domainid_t dds_domain_default (void)
{
  if(dds_global.m_default_domain == DDS_DOMAIN_DEFAULT ){
    const char * env_domain = os_getenv ("VORTEX_DOMAIN");
    if (env_domain) //environment variable exists
    {

      char *env_domain_end;
      long long env_domain_value = os_strtoll (env_domain, &env_domain_end, 10);

      if(env_domain == env_domain_end) //VORTEX_DOMAIN is not integer
      {
        DDS_ERROR(DDS_RETCODE_BAD_PARAMETER, "VORTEX_DOMAIN (%s) environment variable is not integer", env_domain);
        dds_global.m_default_domain = DDS_DOMAIN_DEFAULT ;
      }
      else if(env_domain_value == DDS_DOMAIN_DEFAULT){
        //then use the value from configuration
        dds_global.m_default_domain = config.domainId;
      }
      else if (env_domain_value >= DOMAIN_ID_MIN && env_domain_value <= DOMAIN_ID_MAX){ //Valid value for environment variable
        //use environment variable for the domain
        dds_global.m_default_domain = (int)env_domain_value;
        //change the configuration data
        config.domainId = dds_global.m_default_domain;
      }
      else{ //Invalid value for environment variable
        DDS_ERROR(DDS_RETCODE_BAD_PARAMETER, "VORTEX_DOMAIN (%s) environment variable has invalid value", env_domain);
        dds_global.m_default_domain = DDS_DOMAIN_DEFAULT ;
      }

    }
    else{ // no environment variable defined. then use the value from configuration
      dds_global.m_default_domain = config.domainId;
    }
  }

  return  dds_global.m_default_domain;

}

/* Actual initialization called when participant created */

dds_return_t
dds_init_impl(
        _In_ dds_domainid_t domain)
{
  char buff[64];
  uint32_t len;
  dds_return_t ret = DDS_RETCODE_OK;

  if (dds_domain_default() != domain &&
      DDS_DOMAIN_DEFAULT != domain)
  { //if a valid ID exists on configuration and not matching  the given ID

    ret = DDS_ERRNO(DDS_RETCODE_ERROR,
                    "DDS Init failed: Inconsistent domain configuration detected: domain on configuration: %d, domain %d",
                    dds_domain_default(), domain);
    goto fail;
  }

  if (is_initialized)
  { //Did RTPS initialized before?
    return ret;
  }

  if (rtps_config_prep(dds_cfgst) != 0)
  {
    ret = DDS_ERRNO(DDS_RETCODE_ERROR, "RTPS configuration failed.");
    goto fail;
  }
  dds_cfgst = NULL;

  ut_avlInit(&dds_domaintree_def, &dds_global.m_domains);

  /* Start monitoring the liveliness of all threads and renewing the
     service lease if everything seems well. */

  gv.servicelease = nn_servicelease_new(0, 0);
  assert (gv.servicelease);
  nn_servicelease_start_renewing(gv.servicelease);

  rtps_init();
  upgrade_main_thread();

  /* Set additional default participant properties */

#ifdef _WIN32
  gv.default_plist_pp.process_id = (unsigned) GetProcessId (GetCurrentProcess ());
#else
  gv.default_plist_pp.process_id = (unsigned) getpid();
#endif
  gv.default_plist_pp.present |= PP_PRISMTECH_PROCESS_ID;
  if (dds_init_exe)
  {
    gv.default_plist_pp.exec_name = dds_string_dup(dds_init_exe);
  } else
  {
    gv.default_plist_pp.exec_name = dds_string_alloc(32);
    (void) snprintf(gv.default_plist_pp.exec_name, 32, "Vortex:%u", gv.default_plist_pp.process_id);
  }
  len = (uint32_t) (13 + strlen(gv.default_plist_pp.exec_name));
  gv.default_plist_pp.present |= PP_PRISMTECH_EXEC_NAME;
  if (os_gethostname(buff, sizeof(buff)) == os_resultSuccess)
  {
    gv.default_plist_pp.node_name = dds_string_dup(buff);
    gv.default_plist_pp.present |= PP_PRISMTECH_NODE_NAME;
  }
  gv.default_plist_pp.entity_name = dds_alloc(len);
  (void) snprintf(gv.default_plist_pp.entity_name, len, "%s<%u>", dds_init_exe ? dds_init_exe : "",
                  gv.default_plist_pp.process_id);
  gv.default_plist_pp.present |= PP_ENTITY_NAME;


  is_initialized = true;
  return DDS_RETCODE_OK;

fail:

  return ret;
}



extern void dds_fini (void)
{
  os_mutexLock(&dds_init_mutex);
  if (os_atomic_dec32_nv (&dds_global.m_init_count) == 0)
  {
    dds__builtin_fini();

    ut_handleserver_fini();
    if (ddsi_plugin.init_fn)
    {
      rtps_term ();
      nn_servicelease_free (gv.servicelease);
      downgrade_main_thread ();
      thread_states_fini ();
    }

    if (dds_cfgst != NULL) {
      config_print_and_free_cfgst(dds_cfgst);
      dds_cfgst = NULL;
    }

    config_fini ();
    os_mutexDestroy (&gv.static_logbuf_lock);
    os_mutexDestroy (&dds_global.m_mutex);
    dds_string_free (dds_init_exe);
    dds_global.m_default_domain = DDS_DOMAIN_DEFAULT;
    is_initialized=false;
  }
  os_mutexUnlock(&dds_init_mutex);
}
