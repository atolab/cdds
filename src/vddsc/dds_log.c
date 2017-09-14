#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "dds.h"
#include "ddsi/q_log.h"

#define DDS_FMT_MAX 128

void dds_log_info (_In_z_ _Printf_format_string_ const char * fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  nn_vlog (LC_INFO, fmt, args);
  va_end (args);
}

void dds_log_warn (_In_z_ _Printf_format_string_ const char * fmt, ...)
{
  va_list args;
  char fmt2 [DDS_FMT_MAX];

  strcpy (fmt2, "<Warning> ");
  strncat (fmt2, fmt, DDS_FMT_MAX - 11);
  fmt2[DDS_FMT_MAX-1] = 0;
  fmt = fmt2;

  va_start (args, fmt);
  nn_vlog (LC_WARNING, fmt, args);
  va_end (args);
}

void dds_log_error (_In_z_ _Printf_format_string_ const char * fmt, ...)
{
  va_list args;
  char fmt2 [DDS_FMT_MAX];

  strcpy (fmt2, "<Error> ");
  strncat (fmt2, fmt, DDS_FMT_MAX - 9);
  fmt2[DDS_FMT_MAX-1] = 0;
  fmt = fmt2;

  va_start (args, fmt);
  nn_vlog (LC_ERROR, fmt, args);
  va_end (args);
}

void dds_log_fatal (_In_z_ _Printf_format_string_ const char * fmt, ...)
{
  va_list args;
  char fmt2 [DDS_FMT_MAX];

  strcpy (fmt2, "<Fatal> ");
  strncat (fmt2, fmt, DDS_FMT_MAX - 9);
  fmt2[DDS_FMT_MAX-1] = 0;
  fmt = fmt2;

  va_start (args, fmt);
  nn_vlog (LC_FATAL, fmt, args);
  va_end (args);
  DDS_FAIL (fmt);
}
