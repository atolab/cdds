#include <stdlib.h>
#include "os/os.h"
#include "kernel/dds_types.h"

#define DDS_ERR_CODE_NUM 12
#define DDS_ERR_MOD_NUM 16
#define DDS_ERR_MSG_MAX 128

#define DDS_ERR_NR_INDEX(e) (((-e) & DDS_ERR_NR_MASK) -1)
#define DDS_ERR_MOD_INDEX(e) ((((-e) & DDS_ERR_MOD_MASK) >> 8) -1)

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

/* See DDS_MOD_XXX in dds_types.h */

static const char * dds_err_module_array[DDS_ERR_MOD_NUM] =
{
  "QoS",
  "Kernel",
  "DDSI",
  "Stream",
  "Alloc",
  "WaitSeT",
  "Reader",
  "Writer",
  "Condition",
  "ReadCache",
  "Status",
  "Thread",
  "Instance",
  "Participant",
  "Entity",
  "Topic"
};

const char * dds_err_str (int err)
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

const char * dds_err_mod_str (int err)
{
  unsigned index = DDS_ERR_MOD_INDEX (err);

  if (index >= DDS_ERR_MOD_NUM)
  {
    return "Unknown";
  }

  return dds_err_module_array[index];
}

bool dds_err_check (int err, unsigned flags, const char * where)
{
  if (err < 0)
  {
    if (flags & (DDS_CHECK_REPORT | DDS_CHECK_FAIL))
    {
      char msg[DDS_ERR_MSG_MAX];
      (void) snprintf (msg, DDS_ERR_MSG_MAX, "Error %s:%s:M%u", dds_err_mod_str (err), dds_err_str (err), (unsigned) dds_err_minor (err));
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
      exit (-1);
    }
  }
  return (err >= 0);
}


static void dds_fail_default (const char * msg, const char * where)
{
  fprintf (stderr, "Aborting Failure: %s %s\n", where, msg);
  abort ();
}

static dds_fail_fn dds_fail_func = dds_fail_default;

void dds_fail_set (dds_fail_fn fn)
{
  dds_fail_func = fn;
}

dds_fail_fn dds_fail_get (void)
{
  return dds_fail_func;
}

void dds_fail (const char * msg, const char * where)
{
  if (dds_fail_func)
  {
    (dds_fail_func) (msg, where);
  }
}

#define DDS_HANDLE_IS_VALID(h)  (h!=NULL)

/* TEMPORARY FUNCTION
 * This function checks validity of entities
 * Once handles are used and dds_entity_t is basically the same as dds_return_t this function is not needed anymore */
bool dds_entity_check (_In_opt_ dds_entity_t e, _In_ unsigned flags, _In_z_ const char * where)
{
  if (DDS_HANDLE_IS_VALID(e))
  {
    if (flags & (DDS_CHECK_REPORT | DDS_CHECK_FAIL))
    {
      char *msg = "Err invalid entity";
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
      exit (-1);
    }
  }
  return (e != NULL);
}
