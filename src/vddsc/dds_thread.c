#include <assert.h>
#include "dds.h"
#include "kernel/dds_types.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_log.h"
#include "os/os_report.h"

extern struct thread_states thread_states; /* from q_thread.c */
int dds_thread_init (const char* tname)
{
  int ret = DDS_RETCODE_OK;
  struct thread_state1 *ts;
  os_threadId tid = os_threadIdSelf ();

  if (thread_exists (tname))
  {
    ret = DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER);
  }
  else
  {
    os_mutexLock (&thread_states.lock);
    ts = init_thread_state (tname);
    if (ts == NULL)
    {
      ret = DDS_ERRNO (DDS_RETCODE_OUT_OF_RESOURCES);
    }
    else
    {
      ts->lb = 0;
      ts->extTid = tid;
      ts->tid = tid;
      nn_log (LC_INFO, "started application thread 0x%"PRIxMAX" : %s\n", os_threadIdToInteger (tid), tname);
    }
    os_mutexUnlock (&thread_states.lock);
  }
  return ret;
}

void dds_thread_fini (void)
{
  struct thread_state1 * ts;
  ts = get_thread_state (os_threadIdSelf ());

  assert (ts->state == THREAD_STATE_ALIVE);
  assert (vtime_asleep_p (ts->vtime));
  reset_thread_state (ts);
  os_reportExit ();
}
