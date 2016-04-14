#include <assert.h>
#include "dds.h"
#include "dds_types.h"
#include "q_thread.h"
#include "q_log.h"
#include "os_report.h"

extern struct thread_states thread_states; /* from q_thread.c */
int dds_thread_init (const char* tname)
{
  int ret = DDS_RETCODE_OK;
  struct thread_state1 *ts;
  os_threadId tid = os_threadIdSelf ();
  
  if (thread_exists (tname))
  {
    ret = DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER, DDS_MOD_THREAD, DDS_ERR_M1);
  }
  else
  {
    os_mutexLock (&thread_states.lock);
    ts = init_thread_state (tname);
    if (ts == NULL)
    {
      ret = DDS_ERRNO (DDS_RETCODE_OUT_OF_RESOURCES, DDS_MOD_THREAD, DDS_ERR_M1);
    }
    else
    {
      ts->lb = 0;
      ts->extTid = tid;
      ts->tid = tid;
      nn_log (LC_INFO, "started application thread 0x%llx : %s\n", (unsigned long long) os_threadIdToInteger (tid), tname);
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
