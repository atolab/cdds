#include "dds.h"
#include "os_time.h"

dds_time_t dds_time (void)
{
  os_time time;
  
  /* Get the current time */
  time = os_timeGet ();

  /* convert os_time to dds_time_t */
  dds_time_t dds_time = time.tv_nsec + (time.tv_sec * DDS_NSECS_IN_SEC);

  return dds_time;
}
 
void dds_sleepfor (dds_duration_t n)
{
  os_time interval = { (os_timeSec) (n / DDS_NSECS_IN_SEC), (uint32_t) (n % DDS_NSECS_IN_SEC) };
  os_nanoSleep (interval);
}

void dds_sleepuntil (dds_time_t n)
{
  dds_time_t interval = n - dds_time ();
  if (interval > 0)
  {
    dds_sleepfor (interval);
  }
}
