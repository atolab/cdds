#ifndef _DDS_WRITE_H_
#define _DDS_WRITE_H_

#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define DDS_WR_KEY_BIT 0x01
#define DDS_WR_DISPOSE_BIT 0x02
#define DDS_WR_UNREGISTER_BIT 0x04

typedef enum
{
  DDS_WR_ACTION_WRITE = 0,
  DDS_WR_ACTION_WRITE_DISPOSE = DDS_WR_DISPOSE_BIT,
  DDS_WR_ACTION_DISPOSE = DDS_WR_KEY_BIT | DDS_WR_DISPOSE_BIT,
  DDS_WR_ACTION_UNREGISTER = DDS_WR_KEY_BIT | DDS_WR_UNREGISTER_BIT
}
dds_write_action;

int
dds_write_impl(
        _In_ dds_writer *wr,
        _In_ const void *data,
        _In_ dds_time_t tstamp,
        _In_ dds_write_action action);

int
dds_writecdr_impl(
        _In_ dds_writer *wr,
        _In_ const void *cdr,
        _In_ size_t sz,
        _In_ dds_time_t tstamp,
        _In_ dds_write_action action);


#if defined (__cplusplus)
}
#endif
#endif
