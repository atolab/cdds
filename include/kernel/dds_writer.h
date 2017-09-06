#ifndef _DDS_WRITER_H_
#define _DDS_WRITER_H_

#include "kernel/dds_entity.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define dds_writer_lock(hdl, obj) dds_entity_lock(hdl, DDS_KIND_WRITER, (dds_entity**)obj)
#define dds_writer_unlock(obj)    dds_entity_unlock((dds_entity*)obj);

#if defined (__cplusplus)
}
#endif
#endif
