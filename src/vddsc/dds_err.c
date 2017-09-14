#include <stdlib.h>
#include "os/os.h"
#include "kernel/dds_types.h"
#include "kernel/dds_err.h"

#define DDS_ERR_CODE_NUM 12
#define DDS_ERR_MSG_MAX 128

#define DDS_ERR_NR_INDEX(e) (((-e) & DDS_ERR_NR_MASK) -1)

static const char * dds_err_code_array[DDS_ERR_CODE_NUM] =
{
  "Error",
  "Unsupported",
  "Bad Parameter",
  "Precondition Not Met",
  "Out Of Resources",
  "Not Enabled",
  "Immutable Policy",
  "Inconsistent Policy",
  "Already Deleted",
  "Timeout",
  "No Data",
  "Illegal Operation"
};

const char * dds_err_str (dds_return_t err)
{
  unsigned index = DDS_ERR_NR_INDEX (err);
  if (err >= 0)
  {
    return "Success";
  }
  if (index >= DDS_ERR_CODE_NUM)
  {
    return "Unknown";
  }
  return dds_err_code_array[index];
}

bool dds_err_check (dds_return_t err, unsigned flags, const char * where)
{
  if (err < 0)
  {
    if (flags & (DDS_CHECK_REPORT | DDS_CHECK_FAIL))
    {
      char msg[DDS_ERR_MSG_MAX];
      (void) snprintf (msg, DDS_ERR_MSG_MAX, "Error %d:M%d:%s", dds_err_file_id(err), dds_err_line(err), dds_err_str(err));
      if (flags & DDS_CHECK_REPORT)
      {
        printf ("%s: %s\n", where, msg);
      }
      if (flags & DDS_CHECK_FAIL)
      {
        dds_fail (msg, where);
      }
    }
    if (flags & DDS_CHECK_EXIT)
    {
      exit (1);
    }
  }
  return (err >= 0);
}


static void dds_fail_default (_In_z_ const char * msg_str, _In_z_ const char * where_str)
{
  fprintf (stderr, "Aborting Failure: %s %s\n", where_str, msg_str);
  abort ();
}

static os_atomic_voidp_t dds_fail_func = OS_ATOMIC_VOIDP_INIT(&dds_fail_default);

_Ret_maybenull_
dds_fail_fn dds_fail_set (_In_opt_ dds_fail_fn fn)
{
    dds_fail_fn old;
    do {
        old = os_atomic_ldvoidp(&dds_fail_func);
    } while ( !os_atomic_casvoidp(&dds_fail_func, old, fn) );

    return old;
}

_Ret_maybenull_
dds_fail_fn dds_fail_get (void)
{
  return os_atomic_ldvoidp(&dds_fail_func);
}

void dds_fail (_In_z_ const char * msg_str, _In_z_ const char * where_str)
{
    dds_fail_fn fn = os_atomic_ldvoidp(&dds_fail_func);
    if (fn) {
        fn(msg_str, where_str);
    }
}
